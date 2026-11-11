/* SPDX-License-Identifier: GPL-3.0-only
 * DUBWISE (a Felucca fork) */
/* STAB: the dub techno chord. One key plays a whole chord on one voice (a "memory chord": the root is the key,
 * the shape is CHRD), four band-limited oscillators through a 4-pole low-pass with its own fast decay
 * envelope: the short, muffled, resonant stab that the delay and the reverb then smear into the room.
 *   EDIT 1 (CHORD)  CHRD  the chord shape (four notes above the key, semitones: STAB_CHORDS)
 *                   VOIC  the voicing: CLOSE as written, OPEN the second note up an octave, WIDE the second and
 *                         fourth up an octave, LOW the root down an octave (a heavier stab)
 *                   WAVE  SAW, SQR, or HALF (the notes alternate saw / square: hollower, more organ-like)
 *                   DTN   the notes detuned against each other (alternately up and down, cents): width, chorus
 *   EDIT 2 (FILTER) CUT   the low-pass's resting cutoff
 *                   RES   resonance (of the second stage: the first stays damped, the peak stays one peak)
 *                   ENV   how far the stab's envelope opens the filter above CUT
 *                   DEC   how fast that envelope closes (~5 ms .. ~1.3 s)
 * The ADSR (HOME) shapes the amplitude as on every engine; the filter envelope is separate, so a long amp
 * decay with a short filter decay gives the classic "plucked then muffled" chord.
 * Voice state: ph[0..2] and s[4] the four phases; s[0] s[1] and s[5] s[6] the two filter stages (kept over
 * a retrigger: no click); s[2] the filter envelope (Q15); s[3] its block counter (unused: 0). */

static const char *const N_STAB_CHORD[] = {"m7", "m9", "m11", "4THS", "MIN", "5TH", "SUS4", "MAJ7", "MAJ9", "OCT"};
static const int8_t STAB_CHORDS[][4] = {
    {0, 3, 7, 10},               /* m7: the default dub chord */
    {0, 3, 10, 14},              /* m9 (no fifth: the shell voicing) */
    {0, 10, 15, 17},             /* m11 (open: root, 7th, 3rd and 11th above the octave) */
    {0, 5, 10, 15},              /* 4THS: stacked fourths, the quartal minor-11 colour */
    {0, 7, 12, 15},              /* MIN: an open minor triad */
    {0, 7, 12, 19},              /* 5TH: power chord, no third */
    {0, 5, 7, 12},               /* SUS4 */
    {0, 4, 7, 11},               /* MAJ7 */
    {0, 4, 11, 14},              /* MAJ9 (shell) */
    {0, 12, 0, 12},              /* OCT: root and octave, doubled (detuned against each other by DTN) */
};
static const char *const N_STAB_VOIC[] = {"CLOSE", "OPEN", "WIDE", "LOW"};
static const char *const N_STAB_WAVE[] = {"SAW", "SQR", "HALF"};
_Static_assert(NELEM(N_STAB_CHORD) == NELEM(STAB_CHORDS), "a name per chord");

/* the four notes' offsets from the key, 1/16 semitone, for chord c and voicing vo */
static void stab_offsets(int32_t c, int32_t vo, int32_t *o)
{
    uint32_t k;
    c = clamp(c, 0, (int32_t)NELEM(STAB_CHORDS) - 1);
    for (k = 0; k < 4u; k++)
        o[k] = STAB_CHORDS[c][k] * 16;
    if (vo == 1)
        o[1] += 12 * 16;
    else if (vo == 2) {
        o[1] += 12 * 16;
        o[3] += 12 * 16;
    } else if (vo == 3)
        o[0] -= 12 * 16;
}

/* the filter envelope's decay a block (Q15) for DEC d: a time constant of ~5 ms at 0 .. ~1.3 s at 127 */
static int32_t stab_fdec(int32_t d)
{
    uint32_t tau = ((220u << ((uint32_t)d >> 4)) * (16u + ((uint32_t)d & 15u))) >> 4;   /* samples */
    int32_t k = 32768 - (int32_t)((32768u * CTL) / (tau + CTL));
    return clamp(k, 0, 32767);
}

static void stab_note_on(track_t *t, voice_t *v)
{
    (void)t;
    v->ph[1] = v->ph[0] + 0x51EB851Fu;                /* the notes start apart: no phase-locked thump */
    v->ph[2] = v->ph[0] + 0xA3D70A3Eu;
    v->s[4] = (int32_t)(v->ph[0] + 0x2E147AE1u);
    v->s[2] = 32767;                                  /* the filter envelope starts open */
    v->s[3] = 0;
}

static inline int32_t stab_osc(uint32_t wave, uint32_t k, uint32_t ph, uint32_t inc)
{
    if (wave == 1u || (wave == 2u && (k & 1u)))
        return osc_pulse(ph, inc, 0x80000000u) << 1;  /* osc_pulse peaks at half scale: level with the saw */
    return osc_saw(ph, inc);
}

static void stab_render(track_t *t, voice_t *v, int32_t *out, uint32_t n, const vmod_t *m)
{
    const int16_t *p = t->p;
    uint32_t wave = (uint32_t)clamp(p[P_E2], 0, 2), i, k;
    int32_t o[4], det = p[P_E3];
    uint32_t inc[4], ph[4];
    int32_t fe = v->s[2];
    int32_t cut = (p[P_E4] << 8) + m->cutoff + ((v->pitch16 - 60 * 16) << 3)    /* half key tracking */
                  + ((p[P_E6] * fe) >> 7);                                      /* ENV: up to ~+32 steps */
    tsvf_t f1, f2;
    int32_t a1 = v->s[0], a2 = v->s[1], b1 = v->s[5], b2 = v->s[6];

    stab_offsets(p[P_E0], p[P_E1], o);
    for (k = 0; k < 4u; k++) {
        int32_t ct = det ? ((k & 1u) ? det : -det) * (int32_t)(k + 1u) / 4 : 0;   /* -d/4 +d/2 -3d/4 +d */
        inc[k] = cents_inc(clamp(m->pitch16 + o[k], 0, 2047), ct, m->fine);
    }
    ph[0] = v->ph[0];
    ph[1] = v->ph[1];
    ph[2] = v->ph[2];
    ph[3] = (uint32_t)v->s[4];
    tsvf_coef(&f1, cut, 0);
    tsvf_coef(&f2, cut, p[P_E5]);
    for (i = 0; i < n; i++) {
        int32_t s = 0, y;
        for (k = 0; k < 4u; k++) {
            s += stab_osc(wave, k, ph[k], inc[k]);
            ph[k] += inc[k];
        }
        s >>= 2;                                      /* four notes: within +-32k */
        y = tsvf_lp(&f1, s >> 1, &a1, &a2);
        y = tsvf_lp(&f2, y, &b1, &b2);
        y = soft_knee(y, 16000) * 3;                  /* only the resonance peaks saturate; the 4-pole loses level */
        out[i] += voice_amp(y, m, i) * 2;
    }
    v->ph[0] = ph[0];
    v->ph[1] = ph[1];
    v->ph[2] = ph[2];
    v->s[4] = (int32_t)ph[3];
    v->s[0] = a1;
    v->s[1] = a2;
    v->s[5] = b1;
    v->s[6] = b2;
    v->s[2] = mulq15(fe, stab_fdec(p[P_E7]));        /* the filter envelope closes, a block at a time */
}

/* CHRD VOIC WAVE DTN | CUT RES ENV DEC; amp ATK DEC SUS REL; FENV (the host's ADSR -> filter: unused, 0); MONO;
 * sends DIST CHORUS DELAY REVERB */
static const preset_t STAB_PRESETS[] = {
    {"DUB M7", {0, 0, 0, 12, 58, 56, 64, 56}, {0, 64, 0, 52}, 0, 0, FX(0, 0, 80, 40), PAT(14)},
    {"FOG M9", {1, 1, 0, 18, 50, 30, 44, 56}, {2, 72, 20, 70}, 0, 0, FX(0, 30, 70, 70), PAT(14)},
    {"QUARTAL", {3, 0, 0, 10, 60, 55, 40, 40}, {0, 58, 0, 48}, 0, 0, FX(0, 0, 85, 35), PAT(15)},
    {"CHANNEL", {0, 0, 2, 8, 54, 72, 66, 44}, {0, 50, 0, 40}, 0, 0, FX(5, 0, 90, 30), PAT(14)},
    {"DEEP M11", {2, 2, 0, 16, 46, 35, 52, 60}, {4, 80, 30, 80}, 0, 0, FX(0, 20, 70, 80), PAT(16)},
    {"WARM HALF", {1, 1, 2, 22, 62, 20, 30, 64}, {6, 90, 60, 80}, 0, 0, FX(0, 40, 50, 75), PAT(16)},
    {"RESO STAB", {0, 0, 0, 6, 48, 100, 64, 30}, {0, 48, 0, 36}, 0, 0, FX(10, 0, 90, 30), PAT(15)},
    {"PULSE 5TH", {5, 3, 1, 8, 58, 50, 40, 44}, {0, 60, 0, 48}, 0, 0, FX(15, 0, 80, 30), PAT(14)},
    {"SUS GHOST", {6, 1, 0, 24, 44, 25, 36, 70}, {10, 96, 40, 96}, 0, 0, FX(0, 50, 60, 90), PAT(16)},
    {"BRIGHT 9", {8, 0, 0, 14, 72, 45, 50, 50}, {0, 66, 10, 60}, 0, 0, FX(0, 20, 75, 50), PAT(14)},
    {"OCT DRONE", {9, 3, 0, 30, 52, 30, 20, 90}, {60, 100, 110, 100}, 0, 0, FX(0, 60, 40, 90), PAT(17)},
};

static const engine_t ENG_STAB = {
    .name = "STAB",
    .page_title = {"CHORD", "FLT"},
    .edit = {
        {"CHRD", F_ENUM, 0, (int16_t)(NELEM(N_STAB_CHORD) - 1), 0, N_STAB_CHORD, 0},
        {"VOIC", F_ENUM, 0, 3, 0, N_STAB_VOIC, 0},
        {"WAVE", F_ENUM, 0, 2, 0, N_STAB_WAVE, 0},
        {"DTN", F_INT, 0, 127, 12, 0, "ct"},
        {"CUT", F_CUTOFF, 0, 127, 56, 0, 0},
        {"RES", F_PCT, 0, 127, 40, 0, 0},
        {"ENV", F_PCT, 0, 127, 50, 0, 0},
        {"DEC", F_TIME, 0, 127, 46, 0, 0},
    },
    .presets = STAB_PRESETS,
    .npresets = NELEM(STAB_PRESETS),
    .note_on = stab_note_on,
    .render = stab_render,
    .knob = {P_E4, P_E6, P_E7, P_E0},     /* HOME: CUT, ENV, DEC, CHRD: the filter ride and the chord in reach */
    .poly = 4,                            /* four chords of four notes: 16 oscillators at most */
    .keep = 0x63,                         /* s[0] s[1] s[5] s[6]: the filters */
};

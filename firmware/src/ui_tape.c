/* SPDX-License-Identifier: GPL-3.0-only
 * DUBWISE (a Felucca fork) */
/* The DLY page's graph (GR_TAPE): the tape echo as a little cassette resting on a cantilevered concrete slab, in an
 * Ando-like room of smooth board-marked concrete (panel seams, form-tie holes). Berlin and Kingston
 * only hinted at: the label's coordinates (52.5 N, 18.0 N), two tiny three-dot marks in its corners, a linden sprig
 * (Berlin's street tree) on the left and a fern frond (Jamaica's hills) on the right.
 * The reels' hubs have faces: they blink now and then, go wide-eyed when FDBK runs the echo away (111 and up: the
 * loop gain past 1). The reels follow the transport like a deck's: PLAY spins them up, STOP lets them wind down
 * (~half a second each way); running, the tape moves at a speed set by the tempo, each reel turning at the speed its
 * pack gives, the tape winding from one to the other and reversing at its ends; ECHO THROW held spins them faster.
 * Smoke drifts up behind the tape: a little always while playing, a puff with every chord a STAB track starts (a new
 * voice), the room's haze following how loud the stabs are.
 * Fixed colours, made gray on the GREY and MONO palettes (tape_c) so those stay neutral. */

#define TP_CONC RGB(86, 84, 82)
#define TP_CONC_HI RGB(104, 102, 99)
#define TP_SEAM RGB(70, 68, 66)
#define TP_TIE RGB(46, 44, 43)
#define TP_TIE_HI RGB(124, 121, 117)
#define TP_SLAB RGB(124, 122, 118)
#define TP_SLAB_TOP RGB(152, 149, 144)
#define TP_SHADOW RGB(44, 42, 41)
#define TP_SMOKE RGB(214, 210, 204)
#define TP_SHELL RGB(40, 38, 40)
#define TP_EDGE RGB(66, 62, 62)
#define TP_LABEL RGB(234, 228, 212)
#define TP_INK RGB(44, 38, 38)
#define TP_WIN RGB(24, 20, 20)
#define TP_PACK RGB(110, 70, 44)
#define TP_PACK_HI RGB(138, 92, 58)
#define TP_HUB RGB(246, 242, 232)
#define TP_TOOTH RGB(120, 112, 108)
#define TP_CHEEK RGB(250, 150, 160)
#define TP_LEAF RGB(126, 156, 84)
#define TP_LEAF_DK RGB(92, 120, 62)
#define TP_TWIG RGB(96, 82, 62)
#define TP_FERN RGB(64, 132, 82)
#define TP_FERN_DK RGB(46, 100, 62)
#define TP_DE_BLACK RGB(20, 20, 20)
#define TP_DE_RED RGB(200, 24, 24)
#define TP_GOLD RGB(240, 196, 30)
#define TP_JM_GREEN RGB(0, 150, 60)

/* a fixed colour as the palette allows: as it is, or (GREY, MONO) its luma as a gray (R = B, G = 2 R) */
static uint16_t tape_c(uint16_t c)
{
    if (!ux.mono && !ux.bw)
        return c;
    return ux_gray(ux_luma(c) >> 3);
}

/* a pixel blended over what the canvas holds, a (0..100) of c */
static void tape_blend(int32_t x, int32_t y, uint16_t c, int32_t a)
{
    int32_t py = y + cv_oy;
    if ((uint32_t)x >= cv_w || (uint32_t)py >= cv_h || a <= 0)
        return;
    if (a >= 100)
        cv_pset(x, y, c);
    else
        cv_pset(x, y, ux_mix(swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)x]), c, a));
}

/* a filled disc, anti-aliased against what is already on the canvas, a (0..100) opaque (any radius, in every STYLE:
 * cv_rrect squares its corners in LINE). Only the rim is supersampled */
static void tape_disc_a(int32_t cx, int32_t cy, int32_t r, uint16_t c, int32_t a)
{
    int32_t x, y, r8 = r * 8, in2 = (r8 - 12) * (r8 - 12), out2 = (r8 + 12) * (r8 + 12);
    uint16_t cc = tape_c(c);
    for (y = -r; y < r; y++)
        for (x = -r; x < r; x++) {
            int32_t mx = x * 8 + 4, my = y * 8 + 4, d2 = mx * mx + my * my, n;
            if (d2 >= out2)
                continue;
            if (r8 > 12 && d2 <= in2)
                n = 16;
            else {
                int32_t sx, sy;
                n = 0;
                for (sy = 0; sy < 4; sy++)
                    for (sx = 0; sx < 4; sx++) {
                        int32_t dx = x * 8 + sx * 2 + 1, dy = y * 8 + sy * 2 + 1;
                        n += dx * dx + dy * dy <= r8 * r8;
                    }
            }
            if (n)
                tape_blend(cx + x, cy + y, cc, a * n / 16);
        }
}
static void tape_disc(int32_t cx, int32_t cy, int32_t r, uint16_t c) { tape_disc_a(cx, cy, r, c, 100); }

/* ------------------------------------------------------------------ the room --- */
static void tape_room(int32_t haze)
{
    int32_t y, x, i, j;
    cv_rrect(3, 0, 234, 122, -5, tape_c(TP_CONC), ux.surf);
    for (y = 2; y < 60; y++)                                 /* light from above: the top of the wall lighter */
        cv_rect(6, y, 228, 1, tape_c(ux_mix(TP_CONC_HI, TP_CONC, y * 100 / 60)));
    for (x = 63; x < 234; x += 60)                          /* the formwork's panels: seams .. */
        cv_rect(x, 3, 1, 116, tape_c(TP_SEAM));
    for (y = 40; y < 120; y += 40)
        cv_rect(6, y, 228, 1, tape_c(TP_SEAM));
    for (i = 0; i < 4; i++)                                  /* .. and their tie holes, six to a panel */
        for (j = 0; j < 3; j++) {
            int32_t px = 3 + i * 60, py = j * 40, k;
            for (k = 0; k < 6; k++) {
                int32_t hx = px + 15 + (k % 3) * 15, hy = py + 10 + (k / 3) * 20;
                if (hx < 8 || hx > 232 || hy > 117)
                    continue;
                cv_rect(hx, hy, 2, 2, tape_c(TP_TIE));
                cv_rect(hx, hy + 2, 2, 1, tape_c(TP_TIE_HI));
            }
        }
    /* the haze: the whole wall a little lighter while the stabs ring */
    if (haze > 2)
        for (y = 2; y < 92; y += 2)
            for (x = 6; x < 234; x += 2)
                tape_blend(x + (y & 2 ? 1 : 0), y, tape_c(TP_SMOKE), haze / 12);
}

/* the slab the cassette rests on: cantilevered, its lit top edge, its shadow on the wall */
static void tape_slab(void)
{
    int32_t y;
    for (y = 0; y < 6; y++)
        cv_rect(26, 99 + y, 196, 1, tape_c(ux_mix(TP_SHADOW, TP_CONC, y * 100 / 6)));
    cv_rect(20, 92, 204, 7, tape_c(TP_SLAB));
    cv_rect(20, 92, 204, 1, tape_c(TP_SLAB_TOP));
}

/* --------------------------------------------------------------- the flora --- */
/* a linden leaf: a heart, its point at the bottom */
static void tape_linden_leaf(int32_t x, int32_t y, int32_t s)
{
    tape_disc(x - s / 2, y, s, TP_LEAF);
    tape_disc(x + s / 2, y, s, TP_LEAF);
    tape_disc(x, y + s / 2 + 1, s - 1, TP_LEAF);
    cv_line_t(x, y - s + 2, x, y + s + 2, tape_c(TP_LEAF_DK), 1);   /* the midrib */
}
static void tape_linden(int32_t sway)
{
    cv_line_t(14, 121, 18, 98, tape_c(TP_TWIG), 2);
    cv_line_t(18, 98, 26 + sway, 66, tape_c(TP_TWIG), 1);
    cv_line_t(20, 90, 34 + sway, 82, tape_c(TP_TWIG), 1);
    tape_linden_leaf(28 + sway, 62, 4);
    tape_linden_leaf(36 + sway, 79, 4);
    tape_linden_leaf(13 + sway / 2, 80, 3);
    tape_linden_leaf(26 + sway / 2, 100, 3);
    cv_rect(23 + sway, 74, 2, 2, tape_c(TP_GOLD));          /* its little pale blossom */
    cv_rect(25 + sway, 73, 1, 1, tape_c(TP_LABEL));
}
/* a fern frond arching up and over to the left: leaflets either side, shorter towards the tip */
static void tape_fern(int32_t sway)
{
    int32_t k, px = 230, py = 121;
    for (k = 1; k <= 12; k++) {
        int32_t t = k * 256 / 12;                               /* 0..256 along the frond */
        int32_t x = 230 - (t * 24 >> 8) - ((t * t >> 8) * (6 + sway) >> 8);
        int32_t y = 121 - (t * 64 >> 8) + ((t * t >> 8) * 22 >> 8);
        int32_t l = 9 - k * 7 / 12;
        cv_line_t(px, py, x, y, tape_c(TP_FERN_DK), 1);
        cv_line_t(x, y, x - l, y - l / 2 - 1, tape_c(TP_FERN), 2);
        cv_line_t(x, y, x + l / 2 + 1, y - l, tape_c(TP_FERN), 2);
        px = x;
        py = y;
    }
}

/* ---------------------------------------------------------------- the smoke --- */
#define TP_NPUFF 12u
static struct {
    uint32_t frame, ang_l, ang_r, pos, rnd, age[NPART];
    int32_t haze, spd;                                     /* spd: the transport, 0 stopped .. 256 at speed */
    uint8_t dir;
    struct { int16_t x, y; uint8_t r, life, life0; } p[TP_NPUFF];
} tp;
#define TP_LEN 4096u                                        /* the tape's length, in the pos units below */

static uint32_t tape_rand(void)
{
    tp.rnd = tp.rnd * 1664525u + 1013904223u;
    return tp.rnd >> 16;
}
static void tape_puff(int32_t x, int32_t y, uint32_t r, uint32_t life)
{
    uint32_t i, best = 0;
    for (i = 0; i < TP_NPUFF; i++)                          /* a free one, else the closest to fading out */
        if (tp.p[i].life < tp.p[best].life)
            best = i;
    tp.p[best].x = (int16_t)x;
    tp.p[best].y = (int16_t)y;
    tp.p[best].r = (uint8_t)r;
    tp.p[best].life = tp.p[best].life0 = (uint8_t)life;
}

/* the stabs: a new voice on a STAB track (its age past the last seen) is a chord; their loudness the haze */
static int32_t tape_stabs(uint32_t *hits)
{
    uint32_t i, k;
    int32_t lvl = 0;
    *hits = 0;
    for (i = 0; i < NPART; i++) {
        uint32_t newest = tp.age[i];
#if FELUCCA_STAB
        if (trk[i].engine != ENGI_STAB)
            continue;
#endif
        for (k = 0; k < NVOICE; k++) {
            const voice_t *v = &trk[i].v[k];
            if (!v->active)
                continue;
            lvl += v->env_out;
            if (v->age > tp.age[i]) {
                (*hits)++;
                if (v->age > newest)
                    newest = v->age;
            }
        }
        tp.age[i] = newest;
    }
    return lvl > 32767 ? 32767 : lvl;
}

static void tape_smoke(uint32_t df, uint32_t hits)
{
    uint32_t i, s;
    if (hits)                                               /* a chord: puffs from behind the tape, out at its sides */
        for (s = 0; s < 4u; s++) {
            uint32_t r = tape_rand();
            int32_t x = (r & 1u) ? 40 + (int32_t)(r % 22u) : 178 + (int32_t)(r % 22u);
            tape_puff(x, 86, 6u + tape_rand() % 4u, 60u + tape_rand() % 30u);
        }
    if (song.playing && tape_rand() % 24u < df)             /* the room's own drift */
        tape_puff(30 + (int32_t)(tape_rand() % 180u), 92, 4u, 90u);
    for (i = 0; i < TP_NPUFF; i++) {
        int32_t a, age;
        if (!tp.p[i].life)
            continue;
        tp.p[i].life = tp.p[i].life > df ? (uint8_t)(tp.p[i].life - df) : 0;
        age = tp.p[i].life0 - tp.p[i].life;
        tp.p[i].y = (int16_t)(tp.p[i].y - (int32_t)df);
        tp.p[i].x = (int16_t)(tp.p[i].x + (osc_sine((uint32_t)age * 0x04000000u + i * 0x03000000u) * (int32_t)df) / 40000);
        if (age % 6 < (int32_t)df && tp.p[i].r < 22u)
            tp.p[i].r++;
        a = 58 * tp.p[i].life / (tp.p[i].life0 ? tp.p[i].life0 : 1);    /* fading as it rises */
        if (tp.p[i].life && tp.p[i].y > -20)
        {                                                   /* soft: three layers, densest at the centre */
            int32_t r = tp.p[i].r;
            tape_disc_a(tp.p[i].x, tp.p[i].y, r, TP_SMOKE, a / 3);
            tape_disc_a(tp.p[i].x, tp.p[i].y, r * 2 / 3, TP_SMOKE, a / 3);
            tape_disc_a(tp.p[i].x, tp.p[i].y, r / 3 + 1, TP_SMOKE, a / 3);
        }
    }
}

/* ------------------------------------------------------------------ the tape --- */
static void tape_reel(int32_t cx, int32_t cy, int32_t pr, uint32_t ang, int blink, int wow)
{
    uint32_t k;
    tape_disc(cx, cy, pr, TP_PACK);
    tape_disc(cx, cy, pr - 2, TP_PACK_HI);
    tape_disc(cx, cy, pr - 3, TP_PACK);
    tape_disc(cx, cy, 8, TP_HUB);
    for (k = 0; k < 6u; k++) {                              /* the hub's teeth, turning */
        uint32_t a = ang + k * 0x2AAAAAAAu;
        int32_t s = osc_sine(a), c = osc_sine(a + 0x40000000u);
        cv_rect(cx + ((c * 7) >> 15) - 1, cy + ((s * 7) >> 15) - 1, 2, 2, tape_c(TP_TOOTH));
    }
    if (wow) {                                              /* wide eyes, a little "o" */
        cv_rect(cx - 3, cy - 3, 2, 3, tape_c(TP_INK));
        cv_rect(cx + 2, cy - 3, 2, 3, tape_c(TP_INK));
        cv_rect(cx, cy + 2, 2, 2, tape_c(TP_INK));
    } else {
        if (blink) {
            cv_rect(cx - 4, cy - 2, 3, 1, tape_c(TP_INK));
            cv_rect(cx + 2, cy - 2, 3, 1, tape_c(TP_INK));
        } else {
            cv_rect(cx - 3, cy - 3, 2, 2, tape_c(TP_INK));
            cv_rect(cx + 2, cy - 3, 2, 2, tape_c(TP_INK));
        }
        cv_rect(cx - 2, cy + 1, 1, 1, tape_c(TP_INK));     /* the smile */
        cv_rect(cx - 1, cy + 2, 3, 1, tape_c(TP_INK));
        cv_rect(cx + 2, cy + 1, 1, 1, tape_c(TP_INK));
    }
    cv_rect(cx - 6, cy, 2, 1, tape_c(TP_CHEEK));            /* the cheeks */
    cv_rect(cx + 5, cy, 2, 1, tape_c(TP_CHEEK));
}

/* three dots: a quiet nod, black red gold in one corner of the label, green gold black in the other */
static void tape_dots(int32_t x, int32_t y, uint16_t a, uint16_t b, uint16_t c)
{
    cv_rect(x, y, 2, 2, tape_c(a));
    cv_rect(x + 3, y, 2, 2, tape_c(b));
    cv_rect(x + 6, y, 2, 2, tape_c(c));
}

static void tape_cassette(int32_t pl, int32_t pr, int blink, int wow)
{
    const int32_t sx = 45, sy = 8, sw = 150, sh = 84;
    const int32_t lcx = 96, rcx = 144, cyr = 63;
    cv_rrect(sx, sy, sw, sh, -8, tape_c(TP_EDGE), tape_c(TP_CONC));
    cv_rrect(sx + 2, sy + 2, sw - 4, sh - 4, -7, tape_c(TP_SHELL), tape_c(TP_EDGE));
    tape_disc(sx + 6, sy + 6, 2, TP_EDGE);
    tape_disc(sx + sw - 6, sy + 6, 2, TP_EDGE);
    tape_disc(sx + 6, sy + sh - 6, 2, TP_EDGE);
    tape_disc(sx + sw - 6, sy + sh - 6, 2, TP_EDGE);
    cv_rrect(sx + 11, sy + 4, sw - 22, 34, -4, tape_c(TP_LABEL), tape_c(TP_SHELL));
    cv_text_in(sx + 11, sy + 4, sw - 22, &AF_M, "DUBWISE", tape_c(TP_INK), tape_c(TP_LABEL));
    cv_text_in(sx + 11, sy + 21, sw - 22, &AF_S, "52.5N  18.0N", tape_c(TP_INK), tape_c(TP_LABEL));
    tape_dots(sx + 15, sy + 33, TP_DE_BLACK, TP_DE_RED, TP_GOLD);
    tape_dots(sx + sw - 23, sy + 33, TP_JM_GREEN, TP_GOLD, TP_DE_BLACK);
    cv_rrect(70, 46, 100, 34, -10, tape_c(TP_WIN), tape_c(TP_SHELL));
    cv_line_t(lcx, cyr + pl - 1, rcx, cyr + pr - 1, tape_c(TP_PACK), 1);   /* the tape across, under the packs */
    tape_reel(lcx, cyr, pl, tp.ang_l, blink, wow);
    tape_reel(rcx, cyr, pr, tp.ang_r, blink, wow);
    cv_rrect(84, 82, 72, 8, -3, tape_c(TP_EDGE), tape_c(TP_SHELL));
    cv_rect(88, 83, 64, 1, tape_c(TP_PACK));
    cv_rect(115, 85, 10, 3, tape_c(TP_LABEL));               /* the felt pad */
}

static void graph_tape(void)
{
    uint32_t df = ui.frame - tp.frame, hits;
    int32_t pl, pr, lvl, fb = song.g[G_DFDBK], sway;
    int wow = fb >= 111, blink = (ui.frame / 2u) % 47u == 0u;
    tp.frame = ui.frame;
    if (df > 8u)
        df = 1;                                             /* (back on the page after a while: no jump) */
    /* the transport: towards 256 while playing, towards 0 stopped, a deck's spin-up and wind-down */
    if (song.playing)
        tp.spd = tp.spd + 24 * (int32_t)df > 256 ? 256 : tp.spd + 24 * (int32_t)df;
    else
        tp.spd = tp.spd - 16 * (int32_t)df < 0 ? 0 : tp.spd - 16 * (int32_t)df;
    if (tp.spd) {                                           /* the tape runs at the tempo, auto-reverse at its ends */
        uint32_t bpm = (uint32_t)clamp(song.g[G_BPM], 40, 240);
        uint32_t step = (df * bpm * (uint32_t)tp.spd * (pf.ew1 ? 3u : 1u)) / (120u * 256u);
        if (tp.dir) {
            tp.pos = tp.pos > step ? tp.pos - step : 0u;
            if (!tp.pos)
                tp.dir = 0;
        } else {
            tp.pos += step;
            if (tp.pos >= TP_LEN) {
                tp.pos = TP_LEN;
                tp.dir = 1;
            }
        }
    }
    pl = 14 - (int32_t)(tp.pos * 5u / TP_LEN);              /* the packs: 14 .. 9 px, the tape between them */
    pr = 9 + (int32_t)(tp.pos * 5u / TP_LEN);
    if (tp.spd) {                                           /* one speed of tape: the small pack turns faster */
        uint32_t bpm = (uint32_t)clamp(song.g[G_BPM], 40, 240);
        uint64_t base = (uint64_t)df * (pf.ew1 ? 3u : 1u) * 0x38000000u * bpm * (uint32_t)tp.spd / (120u * 256u);
        uint64_t sl = base / (uint32_t)pl, sr = base / (uint32_t)pr;
        if (sl > 0x30000000u) sl = 0x30000000u;              /* (past a sixth of a turn a frame the teeth would alias) */
        if (sr > 0x30000000u) sr = 0x30000000u;
        tp.ang_l += tp.dir ? 0u - (uint32_t)sl : (uint32_t)sl;
        tp.ang_r += tp.dir ? 0u - (uint32_t)sr : (uint32_t)sr;
    }
    lvl = tape_stabs(&hits);
    tp.haze += ((lvl * 100 >> 15) - tp.haze) / 4;          /* 0..100, following the stabs' loudness */
    sway = (osc_sine(ui.frame * 0x00C00000u) * 2) >> 15;    /* the plants in a slow draught */

    tape_room(tp.haze);
    tape_smoke(df, hits);
    tape_slab();
    tape_linden(sway);
    tape_fern(sway);
    tape_cassette(pl, pr, blink, wow);
}

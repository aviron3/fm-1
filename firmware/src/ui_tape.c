/* SPDX-License-Identifier: GPL-3.0-only
 * DUBWISE (a Felucca fork) */
/* The DLY page's graph (GR_TAPE): the tape echo as a little cassette, Berlin meets Kingston. A cream label with the
 * German stripes on the left and the Jamaican flag on the right; in the window two reels whose hubs have faces, the
 * tape winding from the left reel to the right one while the sequencer plays (each reel turning at the speed its pack
 * gives: the tape runs at one speed, a small pack turns fast); at the end of the tape it turns round and runs back.
 * The faces blink now and then; with FDBK at 111 or more (the loop gain past 1: the echo running away) they go wide-eyed; ECHO THROW
 * held spins the reels faster. Echoes rise up the right edge while playing, more of them with more feedback, one per
 * delay TIME. Fixed colours, made gray on the GREY and MONO palettes (tape_c) so those stay neutral. */

#define TP_SHELL RGB(54, 50, 52)
#define TP_EDGE RGB(78, 72, 72)
#define TP_LABEL RGB(246, 236, 208)
#define TP_INK RGB(40, 34, 34)
#define TP_WIN RGB(34, 26, 26)
#define TP_PACK RGB(116, 72, 44)
#define TP_PACK_HI RGB(146, 96, 60)
#define TP_HUB RGB(250, 246, 236)
#define TP_TOOTH RGB(120, 112, 108)
#define TP_CHEEK RGB(255, 150, 160)
#define TP_DE_BLACK RGB(20, 20, 20)
#define TP_DE_RED RGB(221, 0, 0)
#define TP_GOLD RGB(255, 206, 0)
#define TP_JM_GREEN RGB(0, 155, 58)

/* a fixed colour as the palette allows: as it is, or (GREY, MONO) its luma as a gray (R = B, G = 2 R) */
static uint16_t tape_c(uint16_t c)
{
    if (!ux.mono && !ux.bw)
        return c;
    return ux_gray(ux_luma(c) >> 3);
}

/* a filled disc, anti-aliased against what is already on the canvas (any radius, in every STYLE: cv_rrect squares its
 * corners in LINE and keeps its radius small). `under` is unused: the canvas says what lies behind */
static void tape_disc(int32_t cx, int32_t cy, int32_t r, uint16_t c, uint16_t under)
{
    int32_t x, y, r8 = r * 8;
    uint16_t cc = tape_c(c);
    (void)under;
    for (y = -r; y < r; y++)
        for (x = -r; x < r; x++) {
            int32_t sx, sy, n = 0, py = cy + y + cv_oy, px = cx + x;
            for (sy = 0; sy < 4; sy++)
                for (sx = 0; sx < 4; sx++) {
                    int32_t dx = x * 8 + sx * 2 + 1, dy = y * 8 + sy * 2 + 1;   /* 1/8 px from the centre */
                    n += dx * dx + dy * dy <= r8 * r8;
                }
            if (!n || (uint32_t)px >= cv_w || (uint32_t)py >= cv_h)
                continue;
            if (n < 16) {
                uint16_t bg = swap16(cv_px[(uint32_t)py * cv_w + (uint32_t)px]);
                cv_pset(px, py - cv_oy, ux_mix(bg, cc, n * 100 / 16));
            } else
                cv_pset(px, py - cv_oy, cc);
        }
}

/* the German stripes (black, red, gold) and the Jamaican flag (a gold saltire, green top and bottom, black sides) */
static void tape_flags(int32_t dx, int32_t jx, int32_t y, int32_t w, int32_t h)
{
    int32_t i, s = h / 3;
    cv_rect(dx, y, w, s, tape_c(TP_DE_BLACK));
    cv_rect(dx, y + s, w, s, tape_c(TP_DE_RED));
    cv_rect(dx, y + 2 * s, w, h - 2 * s, tape_c(TP_GOLD));
    cv_rect(jx, y, w, h, tape_c(TP_JM_GREEN));
    for (i = 0; i < w / 2; i++) {                          /* the black triangles, hoist and fly */
        int32_t half = h / 2, hh = half - (i * half * 2) / w;
        if (hh > 0) {
            cv_rect(jx + i, y + half - hh, 1, 2 * hh, tape_c(TP_DE_BLACK));
            cv_rect(jx + w - 1 - i, y + half - hh, 1, 2 * hh, tape_c(TP_DE_BLACK));
        }
    }
    cv_line_t(jx, y, jx + w - 1, y + h - 1, tape_c(TP_GOLD), 2);
    cv_line_t(jx, y + h - 1, jx + w - 1, y, tape_c(TP_GOLD), 2);
}

/* a reel: its tape pack (radius pr), the hub turning (angle ang, a phase), a face that stays upright */
static void tape_reel(int32_t cx, int32_t cy, int32_t pr, uint32_t ang, int blink, int wow)
{
    uint32_t k;
    tape_disc(cx, cy, pr, TP_PACK, TP_WIN);
    tape_disc(cx, cy, pr - 2, TP_PACK_HI, TP_PACK);
    tape_disc(cx, cy, pr - 3, TP_PACK, TP_PACK_HI);
    tape_disc(cx, cy, 10, TP_HUB, TP_PACK);
    for (k = 0; k < 6u; k++) {                              /* the hub's teeth, turning */
        uint32_t a = ang + k * 0x2AAAAAAAu;
        int32_t s = osc_sine(a), c = osc_sine(a + 0x40000000u);
        cv_rect(cx + ((c * 8) >> 15) - 1, cy + ((s * 8) >> 15) - 1, 2, 2, tape_c(TP_TOOTH));
    }
    if (wow) {                                              /* wide eyes, a little "o" */
        cv_rect(cx - 4, cy - 3, 2, 3, tape_c(TP_INK));
        cv_rect(cx + 2, cy - 3, 2, 3, tape_c(TP_INK));
        cv_rect(cx - 1, cy + 2, 2, 2, tape_c(TP_INK));
    } else {
        if (blink) {
            cv_rect(cx - 4, cy - 2, 3, 1, tape_c(TP_INK));
            cv_rect(cx + 2, cy - 2, 3, 1, tape_c(TP_INK));
        } else {
            cv_rect(cx - 4, cy - 3, 2, 2, tape_c(TP_INK));
            cv_rect(cx + 2, cy - 3, 2, 2, tape_c(TP_INK));
        }
        cv_rect(cx - 2, cy + 2, 1, 1, tape_c(TP_INK));     /* the smile */
        cv_rect(cx - 1, cy + 3, 3, 1, tape_c(TP_INK));
        cv_rect(cx + 2, cy + 2, 1, 1, tape_c(TP_INK));
    }
    cv_rect(cx - 7, cy, 2, 1, tape_c(TP_CHEEK));            /* the cheeks */
    cv_rect(cx + 5, cy, 2, 1, tape_c(TP_CHEEK));
}

/* a note rising: a head and a stem */
static void tape_note(int32_t x, int32_t y, uint16_t c)
{
    tape_disc(x, y, 2, c, ux.surf);
    cv_rect(x + 1, y - 7, 1, 7, tape_c(c));
    cv_rect(x + 2, y - 7, 2, 1, tape_c(c));
}

static struct { uint32_t frame, ang_l, ang_r, pos; uint8_t dir; } tp;
#define TP_LEN 4096u                                        /* the tape's length, in the pos units below */

static void graph_tape(void)
{
    const int32_t sx = 30, sy = 4, sw = 180, sh = 114;      /* the shell */
    const int32_t lcx = 94, rcx = 146, cyr = 73;             /* the reels */
    uint32_t df = ui.frame - tp.frame, i;
    int32_t pl, pr, fb = song.g[G_DFDBK];
    int wow = fb >= 111, blink = (ui.frame / 2u) % 47u == 0u;
    const char *sub = "BERLIN x KINGSTON";
    tp.frame = ui.frame;
    if (df > 8u)
        df = 1;                                             /* (back on the page after a while: no jump) */
    if (song.playing) {                                     /* the tape runs, auto-reverse at its ends */
        uint32_t step = df * (pf.ew1 ? 3u : 1u);
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
    pl = 18 - (int32_t)(tp.pos * 7u / TP_LEN);              /* the packs: 18 .. 11 px, the tape between them */
    pr = 11 + (int32_t)(tp.pos * 7u / TP_LEN);
    if (song.playing) {                                     /* one speed of tape: the small pack turns faster */
        uint32_t base = df * (pf.ew1 ? 0x0C000000u : 0x04000000u) * 18u;
        int32_t sgn = tp.dir ? -1 : 1;
        tp.ang_l += (uint32_t)(sgn * (int32_t)(base / (uint32_t)pl));
        tp.ang_r += (uint32_t)(sgn * (int32_t)(base / (uint32_t)pr));
    }

    /* the shell, its screws, the label */
    cv_rrect(sx, sy, sw, sh, -9, tape_c(TP_EDGE), ux.surf);
    cv_rrect(sx + 2, sy + 2, sw - 4, sh - 4, -8, tape_c(TP_SHELL), tape_c(TP_EDGE));
    tape_disc(sx + 7, sy + 7, 2, TP_EDGE, TP_SHELL);
    tape_disc(sx + sw - 7, sy + 7, 2, TP_EDGE, TP_SHELL);
    tape_disc(sx + 7, sy + sh - 7, 2, TP_EDGE, TP_SHELL);
    tape_disc(sx + sw - 7, sy + sh - 7, 2, TP_EDGE, TP_SHELL);
    cv_rrect(sx + 12, sy + 6, sw - 24, 40, -5, tape_c(TP_LABEL), tape_c(TP_SHELL));
    tape_flags(sx + 18, sx + sw - 18 - 20, sy + 12, 20, 15);
    cv_text_in(sx + 40, sy + 7, sw - 80, &AF_M, "DUBWISE", tape_c(TP_INK), tape_c(TP_LABEL));
    if (text_w(&AF_S, sub) > sw - 40)
        sub = "BLN x KGN";
    cv_text_in(sx + 20, sy + 27, sw - 40, &AF_S, sub, tape_c(TP_INK), tape_c(TP_LABEL));

    /* the window, the reels with the tape between them */
    cv_rrect(58, 52, 124, 42, -12, tape_c(TP_WIN), tape_c(TP_SHELL));
    cv_line_t(lcx, cyr + pl - 1, rcx, cyr + pr - 1, tape_c(TP_PACK), 1);   /* the tape across, under the packs */
    tape_reel(lcx, cyr, pl, tp.ang_l, blink, wow);
    tape_reel(rcx, cyr, pr, tp.ang_r, blink, wow);

    /* the bottom: the tape past the head, the guide holes, the felt pad */
    cv_rrect(70, 98, 100, 16, -4, tape_c(TP_EDGE), tape_c(TP_SHELL));
    cv_rect(76, 100, 88, 2, tape_c(TP_PACK));
    tape_disc(88, 108, 3, TP_WIN, TP_EDGE);
    tape_disc(152, 108, 3, TP_WIN, TP_EDGE);
    cv_rect(114, 103, 12, 5, tape_c(TP_LABEL));

    /* the echoes rising up the right edge: one per TIME, as many as FDBK keeps, fading to the top */
    if (song.playing && song.g[G_DMIX] > 0 && fb > 0) {
        uint32_t n = 1u + (uint32_t)fb / 30u, per = 24u;
        static const uint16_t NC[3] = {TP_GOLD, TP_JM_GREEN, TP_DE_RED};
        for (i = 0; i < n && i < 5u; i++) {
            int32_t y = 112 - (int32_t)(((ui.frame / 2u) + i * per) % 100u);
            if (y > 14)
                tape_note(220 + (int32_t)((i & 1u) * 6u), y, NC[i % 3u]);
        }
    }
}

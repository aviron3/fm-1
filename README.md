# DUBWISE

**DUBWISE** is a dub techno fork of [Felucca](https://github.com/hugelton/Felucca) for the M-VAVE FM-1. It keeps all of
Felucca and adds:

- **STAB**, a chord engine: one key plays a whole chord (m7, m9, m11, fourths, minor, fifth, sus4, maj7, maj9, octave)
  through a 4-pole low-pass with its own decay envelope. HOME knobs: CUT, ENV, DEC, CHRD (the filter ride).
- **A tape echo** in place of the delay bus: wow and flutter, a stereo offset, a low cut and two low-pass poles in the
  loop (each repeat thinner and darker) and tape saturation, so FDBK past ~100 % runs away into a bounded howl.
  ECHO THROW now pushes the feedback to 0.98.
- **A power-on dub song**: STAB DUB M7, ANALOG DUB SUB, NOISE VINYL (crackle), DRUM DUB KIT, with patterns, a dotted
  1/8 echo and a long dark hall.
- New presets: 11 STAB sounds, ANALOG DUB SUB, NOISE VINYL and TAPE HISS, DRUM DUB KIT; new phrases DUB STAB, OFFCHORD,
  SWELL, DRONE, DUB BASS, DUB BEAT.

Not affiliated with Felucca's author or with M-VAVE. Install at your own risk; keep Felucca's recovery notes at hand.

---

# Felucca

[![License: GPL-3.0-only](https://img.shields.io/badge/license-GPL--3.0--only-blue.svg)](LICENSE)

Felucca is open-source custom synthesizer firmware for the **M-VAVE FM-1**.

The FM-1 is the only supported hardware target. This project does not provide, maintain, or accept requests for ports to other devices or software platforms.

## Install

**Stable release: 1.8.1.** Connect your FM-1 by USB and use the [web installer](https://hugelton.github.io/Felucca/) in Chrome or Edge. The installer offers a way back to the official V15 firmware. Back up your projects and presets before changing firmware.

**Newer FM-1 flash: 1.8.2 beta.** Some newer FM-1 units use UC25HQ80 flash (JEDEC ID `0xB36014`). [1.8.2-beta](https://github.com/hugelton/Felucca/releases/tag/v1.8.2-beta) adds recognition for that chip; testing on the revised hardware is still pending. See [issue #320](https://github.com/hugelton/Felucca/issues/320). It is not the stable release.

## Project resources

- [Web installer](https://hugelton.github.io/Felucca/)
- [Web editor](https://hugelton.github.io/Felucca/webapp/editor/) ([source](https://github.com/hugelton/Felucca-WebApp))
- [Manual and release notes](https://github.com/hugelton/Felucca/wiki/Home)
- [Building from source](BUILDING.md)
- [Issue tracker](https://github.com/hugelton/Felucca/issues)

The firmware uses [Felucca-Engines](https://github.com/hugelton/Felucca-Engines) (MIT) for its built-in sound engines and [felucca-bsp](https://github.com/hugelton/felucca-bsp) (MPL-2.0) for the FM-1 hardware layer. Neither repository implies support for Felucca on other hardware.

## Development and support

Felucca focuses on the FM-1. Third-party modifications and ports are independent projects and are not tested or supported here. Contributions are considered at the maintainer's discretion; no development schedule or ongoing support is promised.

Felucca is developed with the assistance of AI coding agents (code, tests, documentation, translation, maintenance). Design, features, sounds and direction are the maintainer's or the community's; no generative AI is used for music, icons or artwork. See [On AI-Assisted Development and Responsibility](https://github.com/hugelton/Felucca/discussions/166).

## Credits and licence

Felucca is free software under [GPL-3.0-only](LICENSE); the Hügelton Sample Pack is CC0 1.0. The engines and hardware layer retain their own licences. See [CREDITS.md](CREDITS.md) and [LICENSING.md](LICENSING.md).

M-VAVE and FM-1 are trademarks of their respective owners. Felucca is not affiliated with, endorsed by or supported by M-VAVE.

Copyright (C) 2026 Leo Kuroshita (@kurogedelic), Hügelton Instruments

# dec406 v10.2

Open-source COSPAS-SARSAT FGB/1G and SGB/2G receiver, with direct IQ
demodulation, BCH error correction and beacon message decoding.

## Try a real recording

On Linux with GCC, Make, curl, unzip and sha256sum installed:

```bash
make demo-real-sgb
```

The demonstration downloads a published real SGB recording from firmin,
checks the archive SHA-256 and runs the full acquisition, despreading and
decoding chain. Expected output: normal-mode system beacon, country 227
(France), ID `9C77FFFE1A7F00000000000`, position
`43.56049 N, 1.48080 E`, and BCH validation with zero corrected bits.

## Included

- Offline IQ tools for FGB and SGB; hex-frame decoding.
- Unified live FGB/SGB scanner for Airspy, RTL-SDR, PlutoSDR and HackRF.
- Normal and self-test SGB spreading sequences.
- FGB orbitography identification and French UTC calibration time decoding.
- 94 codec regression tests, with the documented C11 build restored.
- Citation metadata for the software.

## Validation

- Clean source build: `dec406_iq`, `dec406_hex`, `dec406_fgb_iq`, `dec406_scan`.
- Codec suite: 94 passed, 0 failed.
- Synthetic SGB regression and published real SGB replay: BCH zero errors.
- Real orbitography frame: BCH-1 OK; raw national-use bits and UTC decoded.

Independent field report and MEOSAR reception:
https://radiotelescopelavillette.wordpress.com/decodage-balises-sarsat/

Real IQ recordings:
https://github.com/moricef/dec406/releases/tag/real-sgb-samples-20261026

This release provides source archives. Build instructions and SDR dependency
requirements are in the README. No precompiled binaries are supplied.

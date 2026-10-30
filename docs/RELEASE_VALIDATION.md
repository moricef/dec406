# dec406 v10.2 release validation

## 2026-10-30 — codec test compilation

The documented C11 build of `tests/test_sgb_codec.c` fails because
`baudot_encode_char()` calls `toupper()` without its declaration from
`<ctype.h>`. The diagnostic explicitly identifies the missing header.
The proposed minimal correction is to include `<ctype.h>`; the user approved
this correction before implementation. No decoder behavior changes.

Validation: rebuild and run the codec suite, replay the mandatory synthetic
SGB fixture and the published real IQ demonstration before tagging the release.

Result: the documented C11 compilation now succeeds; 94 tests pass, 0 fail.

## Clean source validation

Source was extracted with `git archive bb5badb` into an isolated directory.
The four targets `dec406_iq`, `dec406_hex`, `dec406_fgb_iq` and `dec406_scan`
built successfully. The codec suite again passed all 94 tests.

The freshly built IQ decoder validated the synthetic half-sine fixture with
`nerr=0`, ID `9C749C474569361F6220000`, position 43.20001 N, 5.39999 E.
The published firmin normal-mode recording also validated with `nerr=0`,
ID `9C77FFFE1A7F00000000000`, position 43.56049 N, 1.48080 E.

The real FGB frame `FFFE2FCE3000000000000DBD0E4028514490` produced BCH-1 OK,
ID `9C6000000000001`, raw bits `0028514490`, and day 285 14:49:00 UTC.
The full download/checksum/decode demonstration had already passed on the
same decoder sources before this header-only test correction.

The release adds only documentation and citation metadata after this source
validation; decoder sources are unchanged.

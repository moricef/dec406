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

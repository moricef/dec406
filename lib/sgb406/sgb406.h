/**
 * @file sgb406.h
 * @brief COSPAS-SARSAT second generation (SGB, C/S T.018) burst decoder.
 *
 * Library entry point around the dec406 DSSS/OQPSK chain, for embedding in
 * applications (Android NDK/JNI, desktop). Licence: MIT.
 *
 * Not reentrant: call from a single decoding thread.
 */
#ifndef SGB406_H
#define SGB406_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define SGB406_OK          0   /* frame synced, BCH valid (errors corrected) */
#define SGB406_BCH_FAIL    1   /* frame synced, BCH uncorrectable */
#define SGB406_NO_SYNC     2   /* no SGB preamble found */
#define SGB406_BAD_PARAM  -1

#define SGB406_TEXT_MAX 8192

typedef struct {
    int   status;           /* SGB406_* */
    int   bch_errors;       /* bits corrected by BCH(250,202), -1 if failed */
    int   self_test;        /* 1 if the self-test spreading code was used */
    float z_score;          /* despread sync score of the retained window */
    float t_sync;           /* window start, seconds from buffer start */
    char  hex[64];          /* 63 hex chars, T.018 2-bit left pad (corrected if BCH OK) */
    char  text[SGB406_TEXT_MAX]; /* decoded fields, same text as dec406_iq */
} sgb406_result;

/**
 * @brief Decode one SGB burst from an IQ buffer.
 *
 * @param iq          Interleaved I,Q float samples (I0,Q0,I1,Q1,...),
 *                    any scale. Should cover the whole 1 s burst, ideally
 *                    about 1.5 s around it.
 * @param n_samples   Number of complex samples (iq holds 2*n_samples floats).
 * @param fs          Sample rate in Hz (validated: 2457600; 2048000 and
 *                    other rates use a fractional samples-per-chip).
 * @param f_offset_hz Burst centre frequency minus tuner frequency, in Hz.
 *                    The chain then searches +/-16 kHz around it.
 * @param r           Result, always filled.
 *
 * @return r->status.
 */
int sgb406_decode(const float *iq, size_t n_samples, float fs,
                  float f_offset_hz, sgb406_result *r);

const char *sgb406_version(void);

#ifdef __cplusplus
}
#endif

#endif /* SGB406_H */

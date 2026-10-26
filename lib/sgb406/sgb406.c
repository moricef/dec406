/**
 * @file sgb406.c
 * @brief Library wrapper: sliding-window SGB decode of one IQ buffer,
 *        same search as main_iq.c, result returned in a struct.
 */
#include "sgb406.h"

#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dsss_demod.h"
#include "dec406.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define BURST_BLK 1024
#define NBITS (DSSS_PAYLOAD_BITS + DSSS_PARITY_BITS)

int bch_decode_250_202(const uint8_t *msg, uint8_t *out, uint8_t *cw_out);
int bch_decode_250_202_nerr(const uint8_t *msg, uint8_t *out, int *n_errors);

static char  *g_text;
static size_t g_text_len;

int sgb406_printf(const char *fmt, ...)
{
    if (!g_text) return 0;
    size_t room = SGB406_TEXT_MAX - g_text_len;
    if (room <= 1) return 0;
    va_list ap;
    va_start(ap, fmt);
    int n = vsnprintf(g_text + g_text_len, room, fmt, ap);
    va_end(ap);
    if (n > 0) g_text_len += ((size_t)n < room) ? (size_t)n : room - 1;
    return n;
}

const char *sgb406_version(void) { return "sgb406 1.0"; }

static int cmp_float(const void *a, const void *b)
{
    float fa = *(const float *)a, fb = *(const float *)b;
    return (fa > fb) - (fa < fb);
}

/* Same as main_iq.c find_burst_start(), without the stderr trace. */
static size_t find_burst_start(const float complex *buf, size_t n, float fs)
{
    int dec = (int)(fs / 100000.0f + 0.5f);
    if (dec < 2) dec = 2;
    size_t nd = n / (size_t)dec;
    size_t nblk = nd / BURST_BLK;
    if (nblk < 8) return 0;

    float *pwr = malloc(nblk * sizeof(float));
    float *sorted = malloc(nblk * sizeof(float));
    if (!pwr || !sorted) { free(pwr); free(sorted); return 0; }

    for (size_t b = 0; b < nblk; b++) {
        float s = 0.0f;
        for (size_t j = 0; j < BURST_BLK; j++) {
            size_t off = (b * BURST_BLK + j) * (size_t)dec;
            float re = 0.0f, im = 0.0f;
            for (int d = 0; d < dec; d++) {
                re += crealf(buf[off + (size_t)d]);
                im += cimagf(buf[off + (size_t)d]);
            }
            re /= (float)dec; im /= (float)dec;
            s += re * re + im * im;
        }
        pwr[b] = s / (float)BURST_BLK;
    }
    memcpy(sorted, pwr, nblk * sizeof(float));
    qsort(sorted, nblk, sizeof(float), cmp_float);

    float noise = sorted[nblk / 10];
    float sig   = sorted[nblk * 9 / 10];
    size_t result = 0;
    if (noise > 0 && sig / noise > 3.0f) {
        float thr = sqrtf(noise * sig);
        for (size_t b = 0; b < nblk; b++) {
            if (pwr[b] > thr) {
                result = (b > 1 ? b - 1 : 0) * BURST_BLK * (size_t)dec;
                break;
            }
        }
    }
    free(pwr); free(sorted);
    return result;
}

static void bits_to_hex(const uint8_t *bits, int self_test, char *hex)
{
    for (int k = 0; k < 63; k++) {
        int nib = 0;
        for (int b = 0; b < 4; b++) {
            int j = 4 * k + b - 2;
            int bit = (j >= 0 && j < NBITS) ? (bits[j] & 1) : 0;
            nib |= bit << (3 - b);
        }
        if (k == 0 && self_test) nib |= 0x8;
        hex[k] = "0123456789ABCDEF"[nib];
    }
    hex[63] = '\0';
}

int sgb406_decode(const float *iq, size_t n_samples, float fs,
                  float f_offset_hz, sgb406_result *r)
{
    if (!r) return SGB406_BAD_PARAM;
    memset(r, 0, sizeof(*r));
    r->status = SGB406_BAD_PARAM;
    r->bch_errors = -1;
    if (!iq || fs <= 0.0f || n_samples < (size_t)(fs * 0.5f))
        return r->status;

    float sps = fs / 38400.0f;

    /* Shift the burst to baseband once, on a working copy. */
    float complex *buf = malloc(n_samples * sizeof(float complex));
    if (!buf) return r->status;
    double w = -2.0 * M_PI * (double)f_offset_hz / (double)fs;
    for (size_t k = 0; k < n_samples; k++) {
        float complex s = iq[2 * k] + iq[2 * k + 1] * I;
        if (f_offset_hz != 0.0f) {
            double ph = w * (double)k;
            s *= (float)cos(ph) + (float)sin(ph) * I;
        }
        buf[k] = s;
    }

    size_t win = (size_t)(fs * 1.35f);
    if (n_samples < win) win = n_samples;
    size_t step = (size_t)(fs * 0.25f);

    uint8_t out[NBITS], best[NBITS];
    float best_z = 0.0f;
    int found = 0;
    despread_prn_mode_t best_mode = DESPREAD_PRN_NORMAL;

    for (size_t off = 0; off + win <= n_samples; off += step) {
        size_t trim = find_burst_start(buf + off, win, fs);
        float z = 0.0f;
        despread_prn_mode_t mode = DESPREAD_PRN_NORMAL;
        if (dsss_receive_burst_ex(buf + off + trim, win - trim, sps, fs, 0,
                                  out, &z, &mode) == 0) {
            if (!found || z > best_z) {
                best_z = z;
                best_mode = mode;
                memcpy(best, out, NBITS);
                r->t_sync = (float)(off + trim) / fs;
            }
            found++;
        }
    }
    free(buf);

    if (!found) {
        r->status = SGB406_NO_SYNC;
        return r->status;
    }

    r->z_score = best_z;
    r->self_test = (best_mode == DESPREAD_PRN_SELF_TEST);

    int ones = 0;
    for (int i = 0; i < NBITS; i++) ones += best[i] & 1;

    uint8_t info[DSSS_PAYLOAD_BITS], cw[NBITS];
    int nerr = -1;
    int bch = (ones == 0 || ones == NBITS)
            ? -1 : bch_decode_250_202(best, info, cw);
    if (bch == 0) {
        bch_decode_250_202_nerr(best, info, &nerr);
        r->status = SGB406_OK;
        r->bch_errors = nerr;
    } else {
        memcpy(cw, best, NBITS);
        r->status = SGB406_BCH_FAIL;
        r->bch_errors = -1;
    }
    bits_to_hex(cw, r->self_test, r->hex);

    g_text = r->text;
    g_text_len = 0;
    decode_2g_set_frame_mode(r->self_test);
    decode_2g(best);
    g_text = NULL;

    return r->status;
}

/**
 * @file sgb406_test.c
 * @brief Desktop test of libsgb406: cuts an IQ file into overlapping
 *        2.5 s buffers and calls sgb406_decode() on each, as an
 *        application would on every detected burst.
 *
 * Usage: sgb406_test <file> [-s rate] [-f offset_hz] [-u|-i|-I] [-v]
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sgb406.h"

static float *load(const char *fn, int fmt, size_t *n)
{
    FILE *fp = fopen(fn, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long bytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    int bps = fmt == 'u' ? 2 : fmt == 'i' ? 4 : 8;
    *n = (size_t)bytes / (size_t)bps;
    float *iq = malloc(*n * 2 * sizeof(float));
    unsigned char *raw = malloc((size_t)bytes);
    if (!iq || !raw || fread(raw, 1, (size_t)bytes, fp) != (size_t)bytes) {
        free(iq); free(raw); fclose(fp); return NULL;
    }
    fclose(fp);
    for (size_t k = 0; k < *n * 2; k++) {
        switch (fmt) {
        case 'u': iq[k] = ((float)raw[k] - 127.5f) / 127.5f; break;
        case 'i': iq[k] = (float)((short *)raw)[k] / 32768.0f; break;
        case 'I': iq[k] = (float)((int *)raw)[k] / 4194304.0f; break;
        default:  iq[k] = ((float *)raw)[k]; break;
        }
    }
    free(raw);
    return iq;
}

int main(int argc, char **argv)
{
    const char *fn = NULL;
    float fs = 2457600.0f, foff = 0.0f;
    int fmt = 'f', verbose = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-s") && i + 1 < argc) fs = (float)atof(argv[++i]);
        else if (!strcmp(argv[i], "-f") && i + 1 < argc) foff = (float)atof(argv[++i]);
        else if (!strcmp(argv[i], "-u")) fmt = 'u';
        else if (!strcmp(argv[i], "-i")) fmt = 'i';
        else if (!strcmp(argv[i], "-I")) fmt = 'I';
        else if (!strcmp(argv[i], "-v")) verbose = 1;
        else fn = argv[i];
    }
    if (!fn) {
        fprintf(stderr, "Usage: %s <file> [-s rate] [-f offset_hz] [-u|-i|-I] [-v]\n", argv[0]);
        return 1;
    }

    size_t n;
    float *iq = load(fn, fmt, &n);
    if (!iq) { fprintf(stderr, "Cannot read %s\n", fn); return 1; }
    printf("%s: %zu samples, %.1f s at %.0f Hz\n", fn, n, n / fs, fs);

    static sgb406_result r;
    size_t len = (size_t)(fs * 2.5f), step = (size_t)(fs * 1.25f);
    if (len > n) len = n;
    char last_hex[64] = "";
    float last_t = -10.0f;
    int ok = 0, fail = 0;

    for (size_t off = 0; off + len <= n; off += step) {
        int st = sgb406_decode(iq + 2 * off, len, fs, foff, &r);
        if (st == SGB406_NO_SYNC) continue;
        float t = off / fs + r.t_sync;
        /* overlapping buffers see the same burst twice */
        if (t - last_t < 1.5f && !strcmp(r.hex, last_hex)) continue;
        last_t = t;
        strcpy(last_hex, r.hex);
        if (st == SGB406_OK) ok++; else fail++;
        printf("t=%7.2f s  %s  z=%5.1f  %s%s  %s\n", t,
               st == SGB406_OK ? "BCH OK  " : "BCH FAIL",
               r.z_score, r.self_test ? "self-test " : "",
               st == SGB406_OK ? "" : "", r.hex);
        if (st == SGB406_OK) printf("          corrected bits: %d\n", r.bch_errors);
        if (verbose) printf("%s\n", r.text);
    }
    printf("%d BCH OK, %d BCH FAIL\n", ok, fail);
    free(iq);
    return 0;
}

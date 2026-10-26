/* Force-included when building dec406_v2g.c and display_utils.c into the
 * library: their stdout text is captured into sgb406_result.text. */
#ifndef SGB406_PRINTF_H
#define SGB406_PRINTF_H

#include <stdio.h>

int sgb406_printf(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
#define printf sgb406_printf

#endif /* SGB406_PRINTF_H */

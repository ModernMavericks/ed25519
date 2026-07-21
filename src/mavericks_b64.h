/* Minimal standard-alphabet base64 (encode + decode), dependency-free so the sign/keygen tools build
 * on the native 10.9 toolchain. Not constant-time; fine for signatures + keys (not secret-dependent
 * branching that matters for a CLI tool). */
#ifndef MAVERICKS_B64_H
#define MAVERICKS_B64_H
#include <stddef.h>

static const char MAVERICKS_B64E[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

/* Encode n bytes of in into out (NUL-terminated). out must hold >= 4*((n+2)/3)+1 bytes. */
static size_t mavericks_b64_encode(const unsigned char *in, size_t n, char *out) {
    size_t i, o = 0;
    for (i = 0; i + 2 < n; i += 3) {
        out[o++] = MAVERICKS_B64E[in[i] >> 2];
        out[o++] = MAVERICKS_B64E[((in[i] & 3) << 4) | (in[i + 1] >> 4)];
        out[o++] = MAVERICKS_B64E[((in[i + 1] & 15) << 2) | (in[i + 2] >> 6)];
        out[o++] = MAVERICKS_B64E[in[i + 2] & 63];
    }
    if (i < n) {
        out[o++] = MAVERICKS_B64E[in[i] >> 2];
        if (i + 1 < n) {
            out[o++] = MAVERICKS_B64E[((in[i] & 3) << 4) | (in[i + 1] >> 4)];
            out[o++] = MAVERICKS_B64E[(in[i + 1] & 15) << 2];
        } else {
            out[o++] = MAVERICKS_B64E[(in[i] & 3) << 4];
            out[o++] = '=';
        }
        out[o++] = '=';
    }
    out[o] = 0;
    return o;
}

static int mavericks_b64_val(char c) {
    if (c >= 'A' && c <= 'Z') return c - 'A';
    if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    if (c >= '0' && c <= '9') return c - '0' + 52;
    if (c == '+') return 62;
    if (c == '/') return 63;
    return -1;
}

/* Decode NUL-terminated base64 in into out. Skips whitespace, stops at '='. Returns bytes written,
 * or -1 on an invalid character. out must hold >= 3*(strlen(in)/4) bytes. */
static long mavericks_b64_decode(const char *in, unsigned char *out) {
    long o = 0;
    int q[4], k = 0;
    for (; *in; in++) {
        if (*in == '=') break;
        if (*in == '\n' || *in == '\r' || *in == ' ' || *in == '\t') continue;
        int v = mavericks_b64_val(*in);
        if (v < 0) return -1;
        q[k++] = v;
        if (k == 4) {
            out[o++] = (unsigned char)((q[0] << 2) | (q[1] >> 4));
            out[o++] = (unsigned char)((q[1] << 4) | (q[2] >> 2));
            out[o++] = (unsigned char)((q[2] << 6) | q[3]);
            k = 0;
        }
    }
    if (k == 2) {
        out[o++] = (unsigned char)((q[0] << 2) | (q[1] >> 4));
    } else if (k == 3) {
        out[o++] = (unsigned char)((q[0] << 2) | (q[1] >> 4));
        out[o++] = (unsigned char)((q[1] << 4) | (q[2] >> 2));
    }
    return o;
}

#endif /* MAVERICKS_B64_H */

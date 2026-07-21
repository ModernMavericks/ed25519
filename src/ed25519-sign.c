/*
 * ed25519-sign — sign a file with an ed25519 private key (native, 10.9-buildable).
 *
 *     ed25519-sign -s <base64 private key> <file>
 *
 * Prints the base64 ed25519 signature of the file's raw bytes (RFC 8032, deterministic) to stdout.
 *
 * Key bytes: the private key is the 96-byte blob private[64] || public[32] that ed25519-keygen writes
 * (private[64] is orlp/ed25519's EXPANDED key, which ed25519_sign takes directly). After signing we
 * ed25519_verify against the public half, so a bad key or a wrong private/public split fails loudly
 * instead of emitting a signature no verifier would accept.
 *
 * The signature is standard ed25519, so any verifier accepts it (openssl, or Sparkle's SUPublicEDKey
 * check). Native C means no modern toolchain -- it runs anywhere from 10.9 up and in CI. (See the
 * README for assembling a Sparkle appcast enclosure from the signature + file length.)
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ed25519.h"
#include "mavericks_b64.h"

int main(int argc, char **argv) {
    if (argc != 4 || strcmp(argv[1], "-s") != 0) {
        fprintf(stderr, "usage: %s -s <base64 private key> <file>\n", argv[0]);
        return 2;
    }
    const char *keyb64 = argv[2];
    const char *path   = argv[3];

    unsigned char key[128];
    long klen = mavericks_b64_decode(keyb64, key);
    if (klen != 96) {
        fprintf(stderr, "bad private key: decoded %ld bytes, expected 96 (private[64] || public[32])\n", klen);
        return 1;
    }
    const unsigned char *priv = key;        /* [0:64]  expanded private key */
    const unsigned char *pub  = key + 64;   /* [64:96] public key           */

    FILE *f = fopen(path, "rb");
    if (!f) { fprintf(stderr, "cannot open %s\n", path); return 1; }
    if (fseek(f, 0, SEEK_END) != 0) { fclose(f); fprintf(stderr, "seek failed\n"); return 1; }
    long n = ftell(f);
    if (n < 0 || fseek(f, 0, SEEK_SET) != 0) { fclose(f); fprintf(stderr, "tell/seek failed\n"); return 1; }
    unsigned char *buf = malloc((size_t)n ? (size_t)n : 1);
    if (!buf) { fclose(f); fprintf(stderr, "out of memory\n"); return 1; }
    if (fread(buf, 1, (size_t)n, f) != (size_t)n) { fclose(f); free(buf); fprintf(stderr, "read error\n"); return 1; }
    fclose(f);

    unsigned char sig[64];
    ed25519_sign(sig, buf, (size_t)n, pub, priv);

    if (!ed25519_verify(sig, buf, (size_t)n, pub)) {
        free(buf);
        fprintf(stderr, "signature self-check failed -- bad key, or the blob is not private[64]||public[32]\n");
        return 1;
    }
    free(buf);

    char sigb64[128];
    mavericks_b64_encode(sig, 64, sigb64);
    printf("%s\n", sigb64);
    return 0;
}

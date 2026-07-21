/*
 * ed25519-keygen — generate an ed25519 keypair (native, 10.9-buildable).
 *
 * Writes the private key to a 0600 file (default ./ed25519_key, override with -f <path>) as base64,
 * and the public key to <path>.pub (0644). The public key is also printed to stdout. The PRIVATE key
 * is NEVER printed — on stdout it would linger in terminal scrollback and shell history.
 *
 * Key bytes: the private key is the 96-byte blob private[64] || public[32] (orlp/ed25519's expanded
 * key || public), base64; the public key is the raw 32-byte key, base64. The seed comes from
 * /dev/urandom (orlp/ed25519's ed25519_create_seed).
 *
 * Generate once, keep the private key secret — store it as a CI secret and/or in your password
 * manager, then remove the file. (See the README for wiring these into a Sparkle updater.)
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "ed25519.h"
#include "mavericks_b64.h"

/* Write data to path at exactly `mode` (fchmod defeats a permissive umask). O_EXCL|O_NOFOLLOW
 * refuses to overwrite an existing file or follow a symlink -- never clobber a key the user
 * already has, and never let a planted symlink divert the secret elsewhere. Returns 0 on success. */
static int write_file(const char *path, mode_t mode, const char *data) {
    int fd = open(path, O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW, mode);
    if (fd < 0) {
        if (errno == EEXIST)
            fprintf(stderr, "%s: already exists -- remove it or choose another -f path "
                            "(refusing to overwrite a key)\n", path);
        else
            perror(path);
        return -1;
    }
    if (fchmod(fd, mode) != 0) { perror(path); close(fd); return -1; }
    size_t n = strlen(data);
    if (write(fd, data, n) != (ssize_t)n) { perror(path); close(fd); return -1; }
    if (close(fd) != 0) { perror(path); return -1; }
    return 0;
}

int main(int argc, char **argv) {
    const char *keypath = "ed25519_key";
    int c;
    while ((c = getopt(argc, argv, "f:")) != -1) {
        if (c == 'f') keypath = optarg;
        else { fprintf(stderr, "usage: %s [-f <private-key-file>]\n", argv[0]); return 2; }
    }

    unsigned char seed[32], pub[32], priv[64], blob[96];
    if (ed25519_create_seed(seed) != 0) {
        fprintf(stderr, "failed to read a random seed from /dev/urandom\n");
        return 1;
    }
    ed25519_create_keypair(pub, priv, seed);
    memcpy(blob, priv, 64);
    memcpy(blob + 64, pub, 32);

    char pubb64[64], blobb64[192];
    mavericks_b64_encode(pub, 32, pubb64);
    mavericks_b64_encode(blob, 96, blobb64);

    /* validate the derived .pub path BEFORE writing anything, so a too-long -f never leaves a
     * private-key file behind on an error exit. */
    char privline[200], publine[80], pubpath[1024];
    snprintf(privline, sizeof privline, "%s\n", blobb64);
    snprintf(publine, sizeof publine, "%s\n", pubb64);
    if ((size_t)snprintf(pubpath, sizeof pubpath, "%s.pub", keypath) >= sizeof pubpath) {
        fprintf(stderr, "key path too long\n");
        return 1;
    }
    if (write_file(keypath, 0600, privline) != 0) return 1;
    if (write_file(pubpath, 0644, publine) != 0) return 1;

    /* public key + guidance to stdout; NEVER the private key. */
    printf("Public key (base64):\n  %s\n\n", pubb64);
    printf("Private key written to %s (mode 0600), public key to %s.\n", keypath, pubpath);
    printf("Keep the private key secret -- store it as a CI secret and/or in your password manager,\n");
    printf("then remove the file. Never print or commit it.\n");
    return 0;
}

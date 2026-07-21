# ed25519

General-purpose command-line EdDSA tools built for Mac OS X 10.9 Mavericks and higher:

- `ed25519-keygen`
- `ed25519-sign`

## Set up a Sparkle updater

In your project (assuming 1Password and GitHub):

```sh
ed25519-keygen
item_title="$(basename "$PWD") Sparkle private key"
op document create ed25519_key --title "${item_title}"
rm ed25519_key
op document get "${item_title}" | gh secret set SPARKLE_PRIVATE_KEY
mkdir -p updater
mv ed25519_key.pub updater/
git add updater/ed25519_key.pub
```

## Alternative: OpenSSL equivalents

If you have OpenSSL 3.x, the following scripts are equivalent.

`ed25519-keygen`:

```sh
#!/bin/sh
set -eu
key=ed25519_key
while getopts f: o; do case $o in f) key=$OPTARG ;; *) echo "usage: $0 [-f <file>]" >&2; exit 2 ;; esac; done
for f in "$key" "$key.pub"; do [ -e "$f" ] && { echo "$f: exists -- refusing to overwrite" >&2; exit 1; }; done
( umask 077; openssl genpkey -algorithm ed25519 -out "$key" )
pub=$(openssl pkey -in "$key" -pubout -outform DER | tail -c 32 | base64)
printf '%s\n' "$pub" > "$key.pub"
printf 'Public key (base64):\n  %s\nPrivate key -> %s (0600), public key -> %s.pub\n' "$pub" "$key" "$key"
```

`ed25519-sign`:

```sh
#!/bin/sh
set -eu
[ "$#" -eq 3 ] && [ "$1" = -s ] || { echo "usage: $0 -s <private key PEM> <file>" >&2; exit 2; }
tmp=$(mktemp); trap 'rm -f "$tmp"' EXIT
( umask 077; printf '%s\n' "$2" > "$tmp" )
openssl pkeyutl -sign -inkey "$tmp" -rawin -in "$3" | base64
```

## Attribution

`ed25519-keygen` and `ed25519-sign` embed [orlp/ed25519](https://github.com/orlp/ed25519)
by Orson Peters (zlib license). See [THIRD-PARTY-NOTICES.txt](THIRD-PARTY-NOTICES.txt).

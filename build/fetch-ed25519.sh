#!/bin/sh
# Fetch orlp/ed25519 at the pinned commit (build/../UPSTREAM_COMMIT). Not vendored: fetched
# at build time so Renovate (which bumps UPSTREAM_COMMIT) drives it. Pinning by commit SHA
# is the integrity guarantee -- git verifies fetched objects hash to the SHA, so there is no
# separate tarball checksum to keep in sync. Prints the src/ dir (the *.c + ed25519.h).
set -eu
SELF="$(cd "$(dirname "$0")" && pwd)"
ED_ROOT="$(cd "$SELF/.." && pwd)"; export ED_ROOT
. "$SELF/lib.sh"

C="$(upstream_commit)"
DEST="${1:-$ED_ROOT/build/ed25519}"
mkdir -p "$DEST"; DEST="$(cd "$DEST" && pwd)"   # normalize -> contract: prints an ABSOLUTE src dir
CO="$DEST/ed25519-$C"
if [ ! -f "$CO/src/sign.c" ]; then
  rm -rf "$CO"; mkdir -p "$CO"
  git -C "$CO" init -q
  git -C "$CO" remote add origin https://github.com/orlp/ed25519.git
  git -C "$CO" fetch -q --depth 1 origin "$C"
  git -C "$CO" checkout -q "$C"
fi
[ -f "$CO/src/sign.c" ] || { echo "fetch-ed25519: no src/sign.c after fetch" >&2; exit 1; }
printf '%s\n' "$CO/src"

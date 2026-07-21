#!/bin/sh
set -eu
R="$(cd "$(dirname "$0")/.." && pwd)"
export MAVERICKS_SCRIPTS="${MAVERICKS_SCRIPTS:-/Users/schmonz/Documents/shared-trees/mavericks-shared-cmake/scripts}"
STAGE="$(ED_ROOT="$R" sh "$R/build/build-tools.sh")"
pkg="$(VERSION=20190301-mavericks.1 STAGE="$STAGE" ED_ROOT="$R" sh "$R/build/package-pkg.sh")"
[ -f "$pkg" ] || { echo "no pkg at $pkg" >&2; exit 1; }
# Floor + install-location present in the built product.
X="$(mktemp -d)"; trap 'rm -rf "$X"' EXIT
pkgutil --expand "$pkg" "$X/x"
grep -q 'os-version min="10.9.5"' "$X/x/Distribution" || { echo "floor missing" >&2; exit 1; }
# The tools ship inside the pkg payload -- verify, exactly as CI extracts ed25519-sign.
pkgutil --expand-full "$pkg" "$X/full"
find "$X/full" -type f -name ed25519-sign  | grep -q . || { echo "pkg payload missing ed25519-sign"  >&2; exit 1; }
find "$X/full" -type f -name ed25519-keygen | grep -q . || { echo "pkg payload missing ed25519-keygen" >&2; exit 1; }
echo "package-pkg OK"

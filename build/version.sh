#!/bin/sh
# Derive the full version (<date>-mavericks.N), its tag, and whether to release.
#   auto  (default): new upstream date (no tag yet) -> N=1, RELEASE=yes; else current N, RELEASE=no.
#   local          : N = maxExistingN + 1, RELEASE=yes.
# N resets to 1 whenever UPSTREAM_VERSION (the pinned commit date) changes.
set -eu
SELF="$(cd "$(dirname "$0")" && pwd)"
ED_ROOT="${ED_ROOT:-$(cd "$SELF/.." && pwd)}"; export ED_ROOT
. "$SELF/lib.sh"

mode="${1:-auto}"
U="$(upstream_version)"

if [ -n "${MAVERICKS_TAGS+x}" ]; then
  tags="$MAVERICKS_TAGS"
else
  tags="$(git -C "$ED_ROOT" tag --list "${U}-mavericks.*" 2>/dev/null || true)"
fi

maxN=0
for t in $tags; do
  case "$t" in
    "${U}-mavericks."*)
      n="${t##*.}"
      case "$n" in ''|*[!0-9]*) : ;; *) [ "$n" -gt "$maxN" ] && maxN="$n" ;; esac
      ;;
  esac
done

case "$mode" in
  auto)  if [ "$maxN" -eq 0 ]; then N=1; rel=yes; else N="$maxN"; rel=no; fi ;;
  local) N=$((maxN + 1)); rel=yes ;;
  *) echo "version.sh: mode must be 'auto' or 'local' (got '$mode')" >&2; exit 2 ;;
esac

full="${U}-mavericks.${N}"
printf 'FULL=%s\nTAG=%s\nRELEASE=%s\n' "$full" "$full" "$rel"

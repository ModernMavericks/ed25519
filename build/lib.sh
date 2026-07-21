# build/lib.sh -- sourced helpers for the mavericks-ed25519 build scripts. No side effects.

# Absolute path to the INSTALLED mavericks-shared-cmake scripts dir, resolved from the
# CMake user package registry (what find_package consults). Never vendored/hard-coded.
# Override with MAVERICKS_SCRIPTS (tests point it at the shared-cmake working tree).
msc_scripts() {
  if [ -n "${MAVERICKS_SCRIPTS:-}" ]; then printf '%s\n' "$MAVERICKS_SCRIPTS"; return 0; fi
  reg=$(ls "$HOME/.cmake/packages/MavericksSharedCMake/"* 2>/dev/null | head -1)
  if [ -n "$reg" ]; then
    d=$(cat "$reg")
    if [ -d "$d/scripts" ]; then printf '%s\n' "$d/scripts"; return 0; fi
  fi
  echo "msc_scripts: cannot locate installed mavericks-shared-cmake scripts" >&2
  echo "  install it (its README 'Install') or set MAVERICKS_SCRIPTS" >&2
  return 1
}

# Pinned orlp/ed25519 commit SHA (Renovate bumps UPSTREAM_COMMIT).
upstream_commit() { tr -d '[:space:]' < "${ED_ROOT:-.}/UPSTREAM_COMMIT"; }

# Upstream version = pinned commit's date (YYYYMMDD), from the build-derived UPSTREAM_VERSION file.
upstream_version() { tr -d '[:space:]' < "${ED_ROOT:-.}/UPSTREAM_VERSION"; }

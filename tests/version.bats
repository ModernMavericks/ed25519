setup() {
  REPO="$(cd "$BATS_TEST_DIRNAME/.." && pwd)"
  TMP="$(mktemp -d)"
  mkdir -p "$TMP/build"
  cp "$REPO/build/lib.sh" "$REPO/build/version.sh" "$TMP/build/"
  printf '20190301\n' > "$TMP/UPSTREAM_COMMIT"   # unused by version.sh, present for lib.sh
  printf '20190301\n' > "$TMP/UPSTREAM_VERSION"
}
teardown() { rm -rf "$TMP"; }
run_ver() { ( cd "$TMP" && ED_ROOT="$TMP" MAVERICKS_TAGS="$1" sh build/version.sh "$2" ); }

@test "auto, no prior tags -> mavericks.1, release" {
  run run_ver "" auto
  [ "$status" -eq 0 ]
  [[ "$output" == *"FULL=20190301-mavericks.1"* ]]
  [[ "$output" == *"TAG=20190301-mavericks.1"* ]]
  [[ "$output" == *"RELEASE=yes"* ]]
}
@test "auto, current date already released -> no release" {
  run run_ver "20190301-mavericks.1" auto
  [[ "$output" == *"FULL=20190301-mavericks.1"* ]]
  [[ "$output" == *"RELEASE=no"* ]]
}
@test "auto, picks max existing rev" {
  run run_ver "$(printf '20190301-mavericks.1\n20190301-mavericks.2')" auto
  [[ "$output" == *"FULL=20190301-mavericks.2"* ]]
  [[ "$output" == *"RELEASE=no"* ]]
}
@test "auto, upstream date bumped -> reset to mavericks.1, release" {
  ( cd "$TMP" && printf '20240815\n' > UPSTREAM_VERSION )
  run run_ver "$(printf '20190301-mavericks.1\n20190301-mavericks.2')" auto
  [[ "$output" == *"FULL=20240815-mavericks.1"* ]]
  [[ "$output" == *"RELEASE=yes"* ]]
}
@test "local, increments past max rev" {
  run run_ver "$(printf '20190301-mavericks.1\n20190301-mavericks.2')" local
  [[ "$output" == *"FULL=20190301-mavericks.3"* ]]
  [[ "$output" == *"RELEASE=yes"* ]]
}
@test "bad mode fails" {
  run run_ver "" bogus
  [ "$status" -ne 0 ]
}

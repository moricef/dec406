#!/bin/sh
# Build a standalone sgb406 source archive: lib/sgb406/build/sgb406-<ver>.tar.gz
set -e
here=$(cd "$(dirname "$0")" && pwd)
root=$here/../..
ver=$(sed -n 's/.*"sgb406 \([0-9.]*\)".*/\1/p' "$here/sgb406.c")
out=$here/build/sgb406-$ver
rm -rf "$out"
mkdir -p "$out/src" "$out/include"
cp "$here"/sgb406.h "$here"/sgb406.c "$here"/sgb406_printf.h "$here"/sgb406_test.c \
   "$here"/CMakeLists.txt "$here"/README.md "$out/"
cp "$root/LICENSE" "$out/"
for f in dsss_demod rrc_filter symbol_sync costas4 despread freq_acq dec406_v2g display_utils; do
    cp "$root/src/$f.c" "$out/src/"
done
for f in costas4 country_codes dec406 despread diag_log display_utils dsss_demod \
         freq_acq rrc_filter symbol_sync; do
    cp "$root/include/$f.h" "$out/include/"
done
tar czf "$out.tar.gz" -C "$here/build" "sgb406-$ver"
echo "$out.tar.gz"

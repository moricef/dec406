#!/bin/sh
set -eu

archive_url="https://github.com/moricef/dec406/releases/download/real-sgb-samples-20261026/real_sgb_firmin_20261026.zip"
archive_sha256="37e88c9e445e33dd1c525d1fb5c53b94d2d93f2b5a2003d923010ad20be81299"
sample_rel="onair-sgb-real/sgb_firmin_20261026T211313Z_127s.sigmf-data"
expected_hex="9C77FFFE1A7F00000000000"

repo_dir=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
demo_tmp=$(mktemp -d "${TMPDIR:-/tmp}/dec406-real-sgb.XXXXXX")
trap 'rm -rf -- "$demo_tmp"' EXIT HUP INT TERM

archive="$demo_tmp/real_sgb_firmin_20261026.zip"
output="$demo_tmp/decoder-output.txt"

echo "Downloading the published real SGB recording..."
curl --fail --location --retry 3 --output "$archive" "$archive_url"

actual_sha256=$(sha256sum "$archive" | awk '{print $1}')
if [ "$actual_sha256" != "$archive_sha256" ]; then
    echo "SHA-256 mismatch for downloaded archive" >&2
    exit 1
fi

unzip -q "$archive" "$sample_rel" -d "$demo_tmp"
make -C "$repo_dir" build/dec406_iq

echo "Decoding a real over-the-air SGB capture..."
"$repo_dir/build/dec406_iq" "$demo_tmp/$sample_rel" -i -s 2457600 2>&1 | tee "$output"

grep -q "BCH validated:.*nerr=0" "$output"
grep -q "23 Hex ID: $expected_hex" "$output"
grep -q "Country Code: 227 (France)" "$output"
grep -q "Position: 43.56049.*N, 1.48080.*E" "$output"

echo "Real-signal demonstration passed: acquisition, despreading, BCH and decoding are reproducible."


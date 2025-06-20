mkdir -p ./precompiled
find .pio -type f \( -name 'firmware.hex' -o -name 'firmware.elf' \) -exec sh -c 'for f; do d=./precompiled/${f#.pio/build/}; mkdir -p "${d%/*}"; cp -v "$f" "$d"; done' sh {} +
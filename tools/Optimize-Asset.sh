#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 3 ]; then
    echo "usage: $0 <image|audio> <input> <output>" >&2
    exit 2
fi

mode=$1
input=$2
output=$3
ffmpeg_bin=${FFMPEG:-ffmpeg}

mkdir -p "$(dirname "$output")"

case "$mode" in
    image)
        filter="[0:v]scale='min(960,iw)':-2:flags=lanczos,split[a][b];[a]palettegen=max_colors=256:reserve_transparent=0[p];[b][p]paletteuse=dither=sierra2_4a"
        "$ffmpeg_bin" -hide_banner -loglevel error -y -i "$input" \
            -filter_complex "$filter" \
            -frames:v 1 \
            -compression_level 9 \
            "$output"
        ;;
    audio)
        "$ffmpeg_bin" -hide_banner -loglevel error -y -i "$input" \
            -map_metadata -1 \
            -vn \
            -c:a libvorbis \
            -b:a 64k \
            "$output"
        ;;
    *)
        echo "unsupported asset mode: $mode" >&2
        exit 2
        ;;
esac

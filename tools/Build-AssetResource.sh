#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 2 ]; then
    echo "usage: $0 <input> <object>" >&2
    exit 2
fi

input=$1
object=$2
script_dir=$(cd "$(dirname "$0")" && pwd)
generated=$(dirname "$object")/$(basename "$input")
objcopy_bin=${OBJCOPY:-objcopy}

case "$input" in
    *.png|*.PNG) mode=image ;;
    *.ogg|*.OGG) mode=audio ;;
    *)
        echo "unsupported asset extension: $input" >&2
        exit 2
        ;;
esac

"$script_dir/Optimize-Asset.sh" "$mode" "$input" "$generated"

(
    cd "$(dirname "$generated")"
    "$objcopy_bin" \
        -I binary \
        -O elf64-x86-64 \
        -B i386:x86-64 \
        "$(basename "$generated")" \
        "$(basename "$object")"
)

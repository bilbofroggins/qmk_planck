#!/usr/bin/env bash
# Flash this keymap from the QMK tree it is symlinked into.
#
#   ./flash.sh              # compile + flash
#   ./flash.sh -c           # clean build first
#   ./flash.sh --compile    # compile only, no flash
#
# Planck rev7 uses the stm32-dfu bootloader: hold the reset button (or tap the
# RESET key) when qmk says "Waiting for USB serial port".

set -euo pipefail

QMK_HOME="${QMK_HOME:-$HOME/qmk_firmware}"
KEYBOARD="planck/rev7"
HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

if [[ ! -d "$QMK_HOME" ]]; then
    echo "error: QMK tree not found at $QMK_HOME (set QMK_HOME to override)" >&2
    exit 1
fi

if ! command -v qmk >/dev/null 2>&1; then
    echo "error: qmk CLI not found; install with 'brew install qmk/qmk/qmk'" >&2
    exit 1
fi

# This folder is symlinked into the QMK tree under a keymap name that need not
# match the folder name, so find the link that points back here.
KEYMAPS_DIR="$QMK_HOME/keyboards/$KEYBOARD/keymaps"
KEYMAP="${KEYMAP:-}"
if [[ -z "$KEYMAP" ]]; then
    for link in "$KEYMAPS_DIR"/*; do
        if [[ -L "$link" && "$(cd "$link" && pwd -P)" == "$HERE" ]]; then
            KEYMAP="$(basename "$link")"
            break
        fi
    done
fi

if [[ -z "$KEYMAP" ]]; then
    echo "error: no keymap in $KEYMAPS_DIR links to $HERE" >&2
    echo "       link it with: ln -s \"$HERE\" \"$KEYMAPS_DIR/<name>\"" >&2
    echo "       or set KEYMAP=<name> to pick one explicitly" >&2
    exit 1
fi

cmd=(flash)
args=()
for arg in "$@"; do
    case "$arg" in
        --compile) cmd=(compile) ;;
        *) args+=("$arg") ;;
    esac
done

cd "$QMK_HOME"
set -x
exec qmk "${cmd[@]}" -kb "$KEYBOARD" -km "$KEYMAP" ${args[@]+"${args[@]}"}

#!/usr/bin/env bash
# pomodoro64 - build & run
#
# Uso:
#   ./build.sh          compila soltanto
#   ./build.sh run      compila e lancia in VICE
#   ./build.sh clean    pulisce la cartella build
set -euo pipefail
cd "$(dirname "$0")"

case "${1:-build}" in
    build) make ;;
    run)   make run ;;
    clean) make clean ;;
    *)     echo "uso: $0 [build|run|clean]" >&2; exit 1 ;;
esac

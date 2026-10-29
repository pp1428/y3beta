#!/usr/bin/env bash
# Сборка CPU-версий (нужен gcc)
set -e
cd "$(dirname "$0")/../cpu"

echo "=== det_cpu.c ==="
gcc -O2 -o det_cpu det_cpu.c -lm

echo "=== bruteforce_cpu.c ==="
gcc -O2 -o bruteforce_cpu bruteforce_cpu.c -lm

echo
echo "Готово. Запускайте:"
echo "  ./cpu/det_cpu"
echo "  ./cpu/bruteforce_cpu"

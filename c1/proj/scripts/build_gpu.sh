#!/usr/bin/env bash
# Сборка GPU-версий (OpenCL) под Linux.
# Нужны заголовки/библиотека OpenCL, например:
#   Ubuntu/Debian:  sudo apt install ocl-icd-opencl-dev clinfo
#                   + ICD-драйвер вашей видеокарты (nvidia-opencl-icd / mesa-opencl-icd / intel-opencl-icd)
set -e
cd "$(dirname "$0")/../gpu"

echo "=== Проверка доступных OpenCL-устройств (clinfo) ==="
command -v clinfo >/dev/null 2>&1 && clinfo -l || echo "clinfo не найден - пропускаю проверку"

echo "=== det_host.c ==="
gcc -O2 -o det_host det_host.c -lOpenCL

echo "=== bf_host.c ==="
gcc -O2 -o bf_host bf_host.c -lOpenCL

echo
echo "Готово. Запускайте ИЗ ПАПКИ gpu (важно - .cl файлы читаются по относительному пути!):"
echo "  cd gpu"
echo "  ./det_host"
echo "  ./bf_host"

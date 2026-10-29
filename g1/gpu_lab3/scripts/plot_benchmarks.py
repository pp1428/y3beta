#!/usr/bin/env python3
"""Build PNG graphs from lab3 benchmark.csv. Requires matplotlib (and Python 3)."""
import csv
import sys
from collections import defaultdict
from pathlib import Path


def main() -> int:
    csv_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("benchmark.csv")
    if not csv_path.exists():
        print(f"Не найден файл {csv_path}. Сначала выполните: lab3 benchmark --csv {csv_path}")
        return 1

    try:
        import matplotlib.pyplot as plt
    except ImportError:
        print("Для построения графиков установите matplotlib: python -m pip install matplotlib")
        return 2

    rows = []
    with csv_path.open("r", encoding="utf-8-sig", newline="") as file:
        rows.extend(csv.DictReader(file))
    groups = defaultdict(list)
    for row in rows:
        try:
            groups[(row["task"], row["backend"])].append((int(row["input_size"]), float(row["time_ms"])))
        except (KeyError, ValueError):
            continue

    matrix_backends = [key for key in groups if key[0] == "determinant"]
    if matrix_backends:
        fig, ax = plt.subplots()
        for key in sorted(matrix_backends):
            points = sorted(groups[key])
            ax.plot([p[0] for p in points], [p[1] for p in points], marker="o", label=key[1])
        ax.set_title("Вычисление определителя методом Гаусса")
        ax.set_xlabel("Размер матрицы n × n")
        ax.set_ylabel("Время, мс")
        ax.grid(True, alpha=0.3)
        ax.legend()
        fig.tight_layout()
        fig.savefig("matrix_benchmark.png", dpi=160)
        plt.close(fig)
        print("Создан matrix_benchmark.png")

    md5_backends = [key for key in groups if key[0] == "md5"]
    if md5_backends:
        fig, ax = plt.subplots()
        labels, times = [], []
        for key in sorted(md5_backends):
            points = sorted(groups[key])
            if points:
                labels.append(key[1])
                times.append(points[-1][1])
        ax.bar(labels, times)
        ax.set_title("MD5 brute force: сравнение CPU и OpenCL")
        ax.set_ylabel("Время, мс")
        ax.grid(True, axis="y", alpha=0.3)
        fig.tight_layout()
        fig.savefig("md5_benchmark.png", dpi=160)
        plt.close(fig)
        print("Создан md5_benchmark.png")

    if not matrix_backends and not md5_backends:
        print("В CSV нет строк для построения графиков.")
        return 3
    return 0


if __name__ == "__main__":
    raise SystemExit(main())

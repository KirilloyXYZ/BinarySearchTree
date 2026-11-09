import argparse
import csv
from collections import defaultdict
from pathlib import Path
from statistics import median

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def main():
    parser = argparse.ArgumentParser(description="Графики замеров AVL-дерева")
    parser.add_argument("csv", nargs="?", default="results/benchmark.csv", type=Path)
    args = parser.parse_args()
    with args.csv.open(encoding="utf-8", newline="") as source:
        rows = list(csv.DictReader(source))
    if not rows:
        parser.error("В файле нет измерений")

    operations = [("insert_ms", "Построение (n вставок)"),
                  ("search_ms", "Поиск (2n запросов)"),
                  ("remove_ms", "Удаление (n узлов)"),
                  ("traverse_ms", "Обход ЛКП (n узлов)")]
    orders = [("random", "Случайный"), ("ascending", "Возрастающий"), ("descending", "Убывающий")]
    figure, axes = plt.subplots(2, 2, figsize=(12, 8), constrained_layout=True)
    for axis, (column, title) in zip(axes.flat, operations):
        for order, label in orders:
            groups = defaultdict(list)
            for row in rows:
                if row["order"] == order:
                    groups[int(row["nodes"])].append(float(row[column]))
            counts = sorted(groups)
            if counts:
                axis.plot(counts, [median(groups[n]) for n in counts], marker="o", label=label)
        axis.set(title=title, xlabel="Количество узлов", ylabel="Время, мс", xscale="log", yscale="log")
        axis.grid(True, which="both", alpha=0.25)
        axis.legend()
    figure.suptitle("AVL-дерево — медиана повторных измерений")
    for suffix in ("svg", "png"):
        path = args.csv.with_suffix("." + suffix)
        figure.savefig(path, dpi=160)
        print("Сохранено:", path)
    plt.close(figure)


if __name__ == "__main__":
    main()

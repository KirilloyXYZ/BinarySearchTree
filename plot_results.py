"""Build graphs from actual benchmark CSV files. Requires matplotlib."""

import argparse
import csv
from collections import defaultdict
from pathlib import Path
from statistics import median

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


def read_results(paths):
    groups = defaultdict(list)
    for path in paths:
        with path.open(newline="", encoding="utf-8") as source:
            for row in csv.DictReader(source):
                key = (int(row["n"]), row["order"], row["operation"], int(row["calls"]))
                groups[key].append(float(row["time_ms"]))
    return groups


def draw_panel(axis, groups, operations, title, per_call=False):
    series = defaultdict(list)
    for (n, order, operation, calls), times in groups.items():
        if operation in operations:
            value = median(times)
            if per_call:
                value = value * 1000 / calls
            label = operations[operation] + (" · случайно" if order == "random" else " · по порядку")
            series[label].append((n, value))
    for label, points in sorted(series.items()):
        points.sort()
        axis.plot([point[0] for point in points], [point[1] for point in points], marker="o", label=label)
    axis.set_title(title)
    axis.set_xscale("log")
    axis.set_xlabel("Количество элементов N")
    axis.set_ylabel("Время одного вызова, мкс" if per_call else "Время операции, мс")
    axis.grid(True, which="both", alpha=0.25)
    if series:
        axis.legend(fontsize=8)


def save_figure(groups, panels, title, destination):
    rows = (len(panels) + 1) // 2
    figure, axes = plt.subplots(rows, 2, figsize=(14, rows * 4.2), squeeze=False)
    for axis, panel in zip(axes.flat, panels):
        operations, caption, per_call = panel
        draw_panel(axis, groups, operations, caption, per_call)
    for axis in list(axes.flat)[len(panels):]:
        axis.axis("off")
    figure.suptitle(title, fontsize=16)
    figure.tight_layout(rect=(0, 0, 1, 0.96))
    figure.savefig(destination, dpi=160)
    plt.close(figure)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("csv", nargs="*", type=Path)
    parser.add_argument("--output", type=Path, default=Path("results"))
    arguments = parser.parse_args()
    paths = arguments.csv or [path for path in [Path("results/timings.csv"),
        Path("results/large_sorted.csv"), Path("results/large_random.csv")] if path.exists()]
    if not paths:
        parser.error("No CSV files. Run the benchmark first.")
    groups = read_results(paths)
    if not groups:
        parser.error("CSV files contain no measurements.")
    arguments.output.mkdir(parents=True, exist_ok=True)
    with (arguments.output / "summary.csv").open("w", newline="", encoding="utf-8") as destination:
        writer = csv.writer(destination, lineterminator="\n")
        writer.writerow(["n", "order", "operation", "calls", "samples", "median_ms", "per_call_us"])
        for (n, order, operation, calls), times in sorted(groups.items()):
            time = median(times)
            writer.writerow([n, order, operation, calls, len(times), time, time * 1000 / calls])

    save_figure(groups, [
        ({"insert": "Вставка"}, "Построение дерева", False),
        ({"search_present": "Поиск"}, "Успешный поиск", True),
        ({"search_absent": "Поиск"}, "Неуспешный поиск", True),
        ({"remove": "Удаление"}, "Удаление", True),
    ], "AVL-дерево · базовые операции", arguments.output / "tree_basics.png")

    save_figure(groups, [
        ({"map": "Map", "where": "Where"}, "Преобразование и фильтрация", False),
        ({"reduce": "Reduce"}, "Свёртка", False),
        ({"clone": "Копирование", "get_subtree": "Извлечение"}, "Копирование всего дерева", False),
        ({"contains_subtree": "Вхождение"}, "Проверка поддерева размером N", False),
        ({"serialize": "Запись", "deserialize": "Чтение"}, "Сохранение и восстановление", False),
    ], "AVL-дерево · операции с содержимым", arguments.output / "tree_algorithms.png")

    save_figure(groups, [
        ({"set_union": "Объединение", "set_intersection": "Пересечение", "set_difference": "Разность"},
         "Операции над двумя множествами по N элементов", False),
        ({"set_subset": "Включение", "set_equals": "Равенство"}, "Сравнение множеств", False),
        ({"set_map": "Map", "set_where": "Where"}, "Преобразование и фильтрация", False),
        ({"set_reduce": "Reduce"}, "Свёртка", False),
    ], "Множество на AVL-дереве", arguments.output / "set_operations.png")
    print("Saved graphs and summary to", arguments.output)


if __name__ == "__main__":
    main()

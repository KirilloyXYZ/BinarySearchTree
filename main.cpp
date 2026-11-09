#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>

#include "BinarySearchTree.hpp"
#include "DataGeneration.hpp"
#include "Set.hpp"

long long ReadNumber(const char* prompt, long long minimum, long long maximum)
{
    std::string line;
    while (true)
    {
        std::cout << prompt;
        if (!std::getline(std::cin, line)) { throw std::runtime_error("Ввод завершён"); }
        std::istringstream input(line);
        long long value;
        if (input >> value && (input >> std::ws).eof() && value >= minimum && value <= maximum)
        {
            return value;
        }
        std::cout << "Введите целое число от " << minimum << " до " << maximum << ".\n";
    }
}

int ReadInt(const char* prompt)
{
    return static_cast<int>(ReadNumber(prompt, std::numeric_limits<int>::min(), std::numeric_limits<int>::max()));
}

void WriteItem(std::ostream& stream, const int& key, const int& value)
{
    stream << key << " : " << value;
}

void ShowTree(const Tree<int, int>& tree, std::size_t maxDepth = 8, std::size_t maxNodes = 100)
{
    std::cout << "Узлов: " << tree.GetCount() << ", высота: " << tree.GetHeight() << '\n';
    tree.Print(std::cout, WriteItem, maxDepth, maxNodes);
}

int CheckedInt(long long value)
{
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
    {
        throw std::overflow_error("Результат не помещается в int");
    }
    return static_cast<int>(value);
}

int Negate(const int& value) { return CheckedInt(-static_cast<long long>(value)); }
int Square(const int& value) { return CheckedInt(static_cast<long long>(value) * value); }
bool Even(const int& value) { return value % 2 == 0; }
bool Positive(const int& value) { return value > 0; }
int Sum(const int& value, const int& result) { return CheckedInt(static_cast<long long>(value) + result); }

std::unique_ptr<BinarySearchTree<int, int>> GenerateTree()
{
    std::size_t count = static_cast<std::size_t>(ReadNumber("Количество узлов: ", 1, 100000000));
    int minimum = ReadInt("Минимальный ключ: ");
    int maximum = ReadInt("Максимальный ключ: ");
    int order = static_cast<int>(ReadNumber("Порядок: 1 — случайный, 2 — возрастающий, 3 — убывающий: ", 1, 3));
    unsigned int seed = static_cast<unsigned int>(ReadNumber("Seed: ", 0, std::numeric_limits<unsigned int>::max()));
    DynamicArray<int> keys = GenerateKeys(count, minimum, maximum, static_cast<DataOrder>(order - 1), seed);
    std::mt19937 random(seed);
    std::uniform_int_distribution<int> values(minimum, maximum);
    std::unique_ptr<BinarySearchTree<int, int>> tree = std::make_unique<BinarySearchTree<int, int>>();
    for (std::size_t i = 0; i < keys.GetSize(); ++i) { *tree += {keys.Get(i), values(random)}; }
    std::cout << "Добавлено " << tree->GetCount() << " различных ключей.\n";
    return tree;
}

std::unique_ptr<BinarySearchTree<int, int>> ReadSubtree()
{
    std::size_t count = static_cast<std::size_t>(ReadNumber("Количество пар в искомом поддереве: ", 0, 1000000));
    std::unique_ptr<BinarySearchTree<int, int>> tree = std::make_unique<BinarySearchTree<int, int>>();
    for (std::size_t i = 0; i < count; ++i)
    {
        int key = ReadInt("Ключ: ");
        int value = ReadInt("Значение: ");
        if (!tree->Insert(key, value)) { std::cout << "Повторный ключ пропущен.\n"; }
    }
    return tree;
}

void ShowTraversal(const Tree<int, int>& tree)
{
    int order = static_cast<int>(ReadNumber("Обход: 1 — КЛП, 2 — КПЛ, 3 — ЛКП, 4 — ЛПК, 5 — ПЛК, 6 — ПКЛ: ", 1, 6));
    std::unique_ptr<IEnumerator<int, int>> enumerator = tree.GetEnumerator(static_cast<TraversalOrder>(order - 1));
    std::size_t shown = 0;
    while (shown < 100 && enumerator->MoveNext())
    {
        WriteItem(std::cout, enumerator->GetCurrentKey(), enumerator->GetCurrent());
        std::cout << "  ";
        ++shown;
    }
    if (shown < tree.GetCount()) { std::cout << "... (первые 100 узлов)"; }
    if (shown == 0) { std::cout << "Дерево пусто."; }
    std::cout << '\n';
}

void ShowSet(const char* name, const Set<int>& set)
{
    std::cout << name << " (" << set.GetCount() << "): ";
    set.Print(std::cout);
    std::cout << '\n';
}

void RunSetMenu(Set<int>& first, Set<int>& second)
{
    while (true)
    {
        std::cout << "\nМножества\n1 — Добавить элементы  2 — Удалить элемент  3 — Поиск\n"
                     "4 — Очистить          5 — Показать A и B\n"
                     "6 — Объединение       7 — Пересечение      8 — Разность\n"
                     "9 — Включение         10 — Равенство       0 — Назад\n";
        int command = static_cast<int>(ReadNumber("> ", 0, 10));
        if (command == 0) { return; }
        if (command <= 4)
        {
            int choice = static_cast<int>(ReadNumber("Множество: 1 — A, 2 — B: ", 1, 2));
            Set<int>& selected = choice == 1 ? first : second;
            if (command == 1)
            {
                std::size_t count = static_cast<std::size_t>(ReadNumber("Количество вводимых элементов: ", 0, 1000000));
                std::size_t added = 0;
                for (std::size_t i = 0; i < count; ++i) { added += selected.Insert(ReadInt("Элемент: ")); }
                std::cout << "Добавлено " << added << ", повторов " << count - added << ".\n";
            }
            else if (command == 2)
            {
                bool removed = selected.Remove(ReadInt("Элемент: "));
                std::cout << (removed ? "Элемент удалён.\n" : "Элемент не найден.\n");
            }
            else if (command == 3)
            {
                bool found = selected.Contains(ReadInt("Элемент: "));
                std::cout << (found ? "Элемент найден.\n" : "Элемент не найден.\n");
            }
            else { selected.Clear(); }
            ShowSet(choice == 1 ? "A" : "B", selected);
            continue;
        }

        ShowSet("A", first);
        ShowSet("B", second);
        switch (command)
        {
            case 6:
                ShowSet("A ∪ B", *first.Union(second));
                break;
            case 7:
                ShowSet("A ∩ B", *first.Intersection(second));
                break;
            case 8:
            case 9:
            {
                int choice = static_cast<int>(ReadNumber(command == 8 ? "Разность: 1 — A \\ B, 2 — B \\ A: " :
                                                                      "Включение: 1 — A ⊆ B, 2 — B ⊆ A: ", 1, 2));
                const Set<int>& left = choice == 1 ? first : second;
                const Set<int>& right = choice == 1 ? second : first;
                if (command == 8) { ShowSet(choice == 1 ? "A \\ B" : "B \\ A", *left.Difference(right)); }
                else
                {
                    std::cout << (choice == 1 ? "A ⊆ B: " : "B ⊆ A: ") << (left.IsSubsetOf(right) ? "да\n" : "нет\n");
                }
                break;
            }
            case 10:
                std::cout << (first.Equals(second) ? "Множества равны.\n" : "Множества не равны.\n");
                break;
        }
    }
}

int main()
{
    std::unique_ptr<BinarySearchTree<int, int>> tree = std::make_unique<BinarySearchTree<int, int>>();
    Set<int> firstSet;
    Set<int> secondSet;
    while (true)
    {
        try
        {
            std::cout << "\n1 — Вставка       2 — Поиск         3 — Удаление\n"
                         "4 — Очистка       5 — Генерация     6 — Показать дерево\n"
                         "7 — Обход         8 — Map           9 — Where\n"
                         "10 — Reduce       11 — Извлечь поддерево\n"
                         "12 — Проверить вхождение поддерева\n"
                         "13 — Множества    0 — Выход\n";
            int command = static_cast<int>(ReadNumber("> ", 0, 13));
            switch (command)
            {
                case 0:
                    return 0;
                case 1:
                {
                    int key = ReadInt("Ключ: ");
                    int value = ReadInt("Значение: ");
                    std::cout << (tree->Insert(key, value) ? "Узел добавлен.\n" : "Ключ уже существует, значение сохранено.\n");
                    ShowTree(*tree);
                    break;
                }
                case 2:
                {
                    int key = ReadInt("Ключ: ");
                    if (tree->Contains(key)) { std::cout << "Значение: " << (*tree)[key] << '\n'; }
                    else { std::cout << "Ключ не найден.\n"; }
                    break;
                }
                case 3:
                    std::cout << (tree->Remove(ReadInt("Ключ: ")) ? "Узел удалён.\n" : "Ключ не найден.\n");
                    ShowTree(*tree);
                    break;
                case 4:
                    tree->Clear();
                    ShowTree(*tree);
                    break;
                case 5:
                    tree = GenerateTree();
                    ShowTree(*tree);
                    break;
                case 6:
                {
                    std::size_t depth = static_cast<std::size_t>(ReadNumber("Максимальная глубина (корень — 1): ", 1, 64));
                    std::size_t nodes = static_cast<std::size_t>(ReadNumber("Максимум узлов для вывода: ", 1, 1000));
                    ShowTree(*tree, depth, nodes);
                    break;
                }
                case 7:
                    ShowTraversal(*tree);
                    break;
                case 8:
                case 9:
                {
                    int choice = static_cast<int>(ReadNumber(command == 8 ? "1 — изменить знак, 2 — возвести в квадрат: " :
                                                                          "1 — чётные, 2 — положительные: ", 1, 2));
                    std::unique_ptr<Tree<int, int>> result = command == 8 ? tree->Map(choice == 1 ? Negate : Square) :
                                                                          tree->Where(choice == 1 ? Even : Positive);
                    std::cout << "Исходное дерево:\n";
                    ShowTree(*tree);
                    std::cout << "Результат:\n";
                    ShowTree(*result);
                    break;
                }
                case 10:
                {
                    int result = tree->Reduce(Sum, ReadInt("Начальное значение суммы: "));
                    std::cout << "Сумма: " << result << '\n';
                    break;
                }
                case 11:
                {
                    int key = ReadInt("Ключ корня поддерева: ");
                    if (tree->Contains(key)) { ShowTree(*tree->GetSubtree(key)); }
                    else { std::cout << "Ключ не найден.\n"; }
                    break;
                }
                case 12:
                {
                    std::unique_ptr<BinarySearchTree<int, int>> subtree = ReadSubtree();
                    std::cout << "Искомое поддерево:\n";
                    ShowTree(*subtree);
                    std::cout << (tree->ContainsSubtree(*subtree) ? "Поддерево найдено.\n" : "Поддерево не найдено.\n");
                    break;
                }
                case 13:
                    RunSetMenu(firstSet, secondSet);
                    break;
            }
        }
        catch (const std::exception& error)
        {
            if (!std::cin) { return 0; }
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}

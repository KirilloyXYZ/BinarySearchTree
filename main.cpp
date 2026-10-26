#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

#include "BinarySearchTree.hpp"
#include "TreeSet.hpp"
#include "ValueFunctions.hpp"

struct EndOfInput { };

static std::string ReadLine(const std::string& prompt)
{
    std::cout << prompt;
    std::string text;
    if (!std::getline(std::cin, text))
    {
        throw EndOfInput();
    }
    return text;
}

template<typename T>
T ReadValue(const std::string& prompt, T (*read)(const std::string&))
{
    while (true)
    {
        std::string text = ReadLine(prompt);
        try
        {
            return read(text);
        }
        catch (const std::exception& error)
        {
            std::cout << "Ошибка ввода: " << error.what() << '\n';
        }
    }
}

static int ReadInt(const std::string& prompt)
{
    return ReadValue<int>(prompt, ValueFromString<int>);
}

static std::size_t ReadSize(const std::string& prompt)
{
    return ReadValue<std::size_t>(prompt, SizeFromString);
}

template<typename T>
struct DemoFunctions
{
    bool (*compare)(const T&, const T&);
    T (*read)(const std::string&);
    T (*map)(const T&);
    bool (*where)(const T&);
    T (*reduce)(const T&, const T&);
    T startValue;
    T (*generate)(int);
    std::string description;
};

template<typename T>
void PrintEnumeration(const IEnumerable<T>& container, TraversalOrder order)
{
    std::unique_ptr<IEnumerator<T>> enumerator = container.GetEnumerator(order);
    std::size_t shown = 0;
    while (shown < 25 && enumerator->MoveNext())
    {
        if (shown > 0) { std::cout << "; "; }
        std::cout << enumerator->GetCurrent();
        ++shown;
    }
    if (shown == 25 && enumerator->MoveNext()) { std::cout << "; ..."; }
}

template<typename T>
void PrintItems(const std::string& name, const IEnumerable<T>& container, std::size_t count)
{
    std::cout << name << " (элементов: " << count << "): [";
    PrintEnumeration(container, TraversalOrder::LKP);
    std::cout << "]\n";
}

template<typename T>
void PrintTraversals(const IEnumerable<T>& container)
{
    TraversalOrder orders[] = {TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LKP,
        TraversalOrder::LPK, TraversalOrder::PLK, TraversalOrder::PKL};
    const char* names[] = {"КЛП", "КПЛ", "ЛКП", "ЛПК", "ПЛК", "ПКЛ"};
    for (std::size_t i = 0; i < 6; ++i)
    {
        std::cout << names[i] << ": ";
        std::size_t shown = 0;
        container.Traverse(orders[i], [&shown](const T& value)
        {
            if (shown < 25) { std::cout << value << ' '; }
            ++shown;
        });
        if (shown > 25) { std::cout << "..."; }
        std::cout << '\n';
    }
}

static TraversalOrder ReadTraversalOrder()
{
    std::cout << "1. КЛП   2. КПЛ   3. ЛКП   4. ЛПК   5. ПЛК   6. ПКЛ\n";
    int order = ReadInt("Порядок обхода: ");
    if (order < 1 || order > 6)
    {
        throw std::invalid_argument("Порядок обхода должен быть от 1 до 6");
    }
    return static_cast<TraversalOrder>(order - 1);
}

template<typename T, typename Container>
void FillManually(Container& container, const DemoFunctions<T>& functions)
{
    std::size_t count = ReadSize("Количество вводимых элементов: ");
    for (std::size_t i = 0; i < count; ++i)
    {
        T value = ReadValue<T>("Элемент " + std::to_string(i + 1) + ": ", functions.read);
        bool inserted = container.Insert(value);
        if (!inserted) { std::cout << "Этот ключ уже есть; повтор пропущен.\n"; }
    }
}

template<typename T, typename Container>
void FillGenerated(Container& container, const DemoFunctions<T>& functions)
{
    std::size_t count = ReadSize("Количество уникальных элементов: ");
    int minimum = ReadInt("Минимальный ключ: ");
    int maximum = ReadInt("Максимальный ключ: ");
    int seed = ReadInt("Начальное значение генератора, например 2026: ");
    long long range = static_cast<long long>(maximum) - minimum + 1;
    if (minimum > maximum || count > static_cast<unsigned long long>(range) || seed < 0)
    {
        throw std::invalid_argument("Неверные параметры или диапазон слишком мал для уникальных ключей");
    }

    std::mt19937 random(seed);
    std::uniform_int_distribution<long long> positionDistribution(0, range - 1);
    long long position = positionDistribution(random);
    long long step = 1;
    if (range > 1)
    {
        std::uniform_int_distribution<long long> stepDistribution(1, range - 1);
        do { step = stepDistribution(random); }
        while (std::gcd(step, range) != 1);
    }

    // A coprime step visits different keys without storing the whole range.
    for (std::size_t i = 0; i < count; ++i)
    {
        int key = static_cast<int>(minimum + position);
        container.Insert(functions.generate(key));
        position = (position + step) % range;
    }
    std::cout << "Создано уникальных элементов: " << container.GetCount() << '\n';
}

static void PrintTreeMenu()
{
    std::cout << "\nДЕРЕВО\n"
              << "1. Создать вручную   2. Сгенерировать   3. Показать элементы и результат\n"
              << "4. Добавить          5. Найти           6. Удалить\n"
              << "7. Map               8. Where          9. Reduce\n"
              << "10. Извлечь поддерево в результат       11. Проверить вхождение результата\n"
              << "12. Скопировать дерево в результат     13. Очистить дерево\n"
              << "14. Шесть обходов через Traverse       15. Обход через TreeEnumerator\n"
              << "16. Ввести дерево-кандидат в результат\n"
              << "0. Назад\n";
}

template<typename T>
void RunTreeMenu(const DemoFunctions<T>& functions)
{
    std::unique_ptr<Tree<T>> tree = std::make_unique<BinarySearchTree<T>>(functions.compare);
    std::unique_ptr<Tree<T>> result = std::make_unique<BinarySearchTree<T>>(functions.compare);
    std::cout << functions.description << '\n';
    while (true)
    {
        try
        {
            PrintTreeMenu();
            int command = ReadInt("Команда: ");
            switch (command)
            {
                case 0: return;
                case 1:
                case 2:
                {
                    std::unique_ptr<Tree<T>> created = std::make_unique<BinarySearchTree<T>>(functions.compare);
                    if (command == 1) { FillManually(*created, functions); }
                    else { FillGenerated(*created, functions); }
                    tree = std::move(created);
                    PrintItems("Дерево", *tree, tree->GetCount());
                    break;
                }
                case 3:
                    PrintItems("Дерево", *tree, tree->GetCount());
                    std::cout << "Высота: " << tree->GetHeight() << "; порядок и баланс: "
                              << (tree->IsValid() ? "верны" : "нарушены") << '\n';
                    if (tree->GetCount() > 0) { std::cout << "Корень: " << tree->GetRoot() << '\n'; }
                    PrintItems("Последний результат / кандидат", *result, result->GetCount());
                    break;
                case 4:
                {
                    T value = ReadValue<T>("Добавить элемент: ", functions.read);
                    std::cout << (tree->Insert(value) ? "Добавлен.\n" : "Такой ключ уже есть.\n");
                    break;
                }
                case 5:
                case 6:
                {
                    T value = ReadValue<T>("Искомое значение: ", functions.read);
                    if (command == 5) { std::cout << (tree->Contains(value) ? "Найден.\n" : "Не найден.\n"); }
                    else { std::cout << (tree->Remove(value) ? "Удалён.\n" : "Не найден.\n"); }
                    break;
                }
                case 7:
                    result = tree->Map(functions.map);
                    PrintItems("Результат Map", *result, result->GetCount());
                    break;
                case 8:
                    result = tree->Where(functions.where);
                    PrintItems("Результат Where", *result, result->GetCount());
                    break;
                case 9:
                {
                    T reduced = tree->Reduce(functions.reduce, functions.startValue);
                    std::cout << "Reduce: " << reduced << '\n';
                    break;
                }
                case 10:
                {
                    T value = ReadValue<T>("Корень поддерева: ", functions.read);
                    result = tree->GetSubtree(value);
                    PrintItems("Поддерево", *result, result->GetCount());
                    break;
                }
                case 11:
                    std::cout << (tree->ContainsSubtree(*result) ? "Поддерево входит.\n" : "Поддерево не входит.\n");
                    break;
                case 12:
                    result = tree->Clone();
                    PrintItems("Копия", *result, result->GetCount());
                    break;
                case 13:
                    tree->Clear();
                    std::cout << "Дерево очищено.\n";
                    break;
                case 14:
                    PrintTraversals(*tree);
                    break;
                case 15:
                    PrintEnumeration(*tree, ReadTraversalOrder());
                    std::cout << '\n';
                    break;
                case 16:
                {
                    std::unique_ptr<Tree<T>> created = std::make_unique<BinarySearchTree<T>>(functions.compare);
                    FillManually(*created, functions);
                    result = std::move(created);
                    PrintItems("Кандидат", *result, result->GetCount());
                    break;
                }
                default:
                    std::cout << "Неизвестная команда.\n";
            }
        }
        catch (const std::exception& error)
        {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}

static void PrintSetMenu()
{
    std::cout << "\nМНОЖЕСТВА A И B\n"
              << "1. Ввести A   2. Ввести B   3. Сгенерировать A   4. Сгенерировать B\n"
              << "5. Показать A, B и результат\n"
              << "6. Добавить в A   7. Удалить из A   8. Найти в A\n"
              << "9. Map(A)   10. Where(A)   11. Reduce(A)\n"
              << "12. Объединение   13. Пересечение   14. Разность A - B\n"
              << "15. A включено в B?   16. A равно B?   17. Скопировать A\n"
              << "18. Очистить A   19. Очистить B\n"
              << "20. Шесть обходов A через Traverse   21. Обход A через TreeEnumerator   0. Назад\n";
}

template<typename T>
void RunSetMenu(const DemoFunctions<T>& functions)
{
    std::unique_ptr<Set<T>> a = std::make_unique<TreeSet<T>>(functions.compare);
    std::unique_ptr<Set<T>> b = std::make_unique<TreeSet<T>>(functions.compare);
    std::unique_ptr<Set<T>> result = std::make_unique<TreeSet<T>>(functions.compare);
    std::cout << functions.description << '\n';
    while (true)
    {
        try
        {
            PrintSetMenu();
            int command = ReadInt("Команда: ");
            switch (command)
            {
                case 0: return;
                case 1:
                case 2:
                case 3:
                case 4:
                {
                    std::unique_ptr<Set<T>> created = std::make_unique<TreeSet<T>>(functions.compare);
                    if (command <= 2) { FillManually(*created, functions); }
                    else { FillGenerated(*created, functions); }
                    if (command == 1 || command == 3) { a = std::move(created); }
                    else { b = std::move(created); }
                    PrintItems("A", *a, a->GetCount());
                    PrintItems("B", *b, b->GetCount());
                    break;
                }
                case 5:
                    PrintItems("A", *a, a->GetCount());
                    PrintItems("B", *b, b->GetCount());
                    PrintItems("Последний результат", *result, result->GetCount());
                    break;
                case 6:
                {
                    T value = ReadValue<T>("Добавить элемент: ", functions.read);
                    std::cout << (a->Insert(value) ? "Добавлен.\n" : "Такой ключ уже есть.\n");
                    break;
                }
                case 7:
                case 8:
                {
                    T value = ReadValue<T>("Искомое значение: ", functions.read);
                    bool success = command == 7 ? a->Remove(value) : a->Contains(value);
                    std::cout << (success ? "Выполнено: элемент найден.\n" : "Элемент не найден.\n");
                    break;
                }
                case 9: result = a->Map(functions.map); break;
                case 10: result = a->Where(functions.where); break;
                case 11:
                {
                    T reduced = a->Reduce(functions.reduce, functions.startValue);
                    std::cout << "Reduce: " << reduced << '\n';
                    break;
                }
                case 12: result = a->Union(*b); break;
                case 13: result = a->Intersection(*b); break;
                case 14: result = a->Difference(*b); break;
                case 15: std::cout << (a->IsSubsetOf(*b) ? "A включено в B.\n" : "A не включено в B.\n"); break;
                case 16: std::cout << (a->Equals(*b) ? "Множества равны.\n" : "Множества различаются.\n"); break;
                case 17: result = a->Clone(); break;
                case 18: a->Clear(); break;
                case 19: b->Clear(); break;
                case 20: PrintTraversals(*a); break;
                case 21:
                    PrintEnumeration(*a, ReadTraversalOrder());
                    std::cout << '\n';
                    break;
                default: std::cout << "Неизвестная команда.\n";
            }
            if (command == 9 || command == 10 || (command >= 12 && command <= 14) || command == 17)
            {
                PrintItems("Результат", *result, result->GetCount());
            }
        }
        catch (const std::exception& error)
        {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
}

template<typename T>
void RunDemo(const DemoFunctions<T>& functions)
{
    std::cout << "1. Дерево   2. Множество\n";
    int container = ReadInt("Контейнер: ");
    if (container == 1) { RunTreeMenu(functions); }
    else if (container == 2) { RunSetMenu(functions); }
    else { std::cout << "Неизвестный контейнер.\n"; }
}

static int IntMap(const int& value)
{
    if (value == std::numeric_limits<int>::min()) { throw std::overflow_error("Нельзя изменить знак минимального int"); }
    return -value;
}
static bool IntWhere(const int& value) { return value % 2 == 0; }
static int IntReduce(const int& value, const int& result)
{
    long long sum = static_cast<long long>(value) + result;
    if (sum < std::numeric_limits<int>::min() || sum > std::numeric_limits<int>::max()) { throw std::overflow_error("Сумма не помещается в int"); }
    return static_cast<int>(sum);
}
static int IntFromKey(int key) { return key; }
static double DoubleMap(const double& value) { return -value; }
static bool DoubleWhere(const double& value) { return value > 0; }
static double DoubleReduce(const double& value, const double& result)
{
    double sum = value + result;
    if (!std::isfinite(sum)) { throw std::overflow_error("Переполнение суммы double"); }
    return sum;
}
static double DoubleFromKey(int key) { return key + 0.25; }
static std::string StringMap(const std::string& value) { return value + "!"; }
static bool StringWhere(const std::string& value) { return !value.empty(); }
static std::string StringReduce(const std::string& value, const std::string& result)
{
    return result.empty() ? value : result + " | " + value;
}
static std::string StringFromKey(int key) { return "word_" + std::to_string(key); }
int main()
{
    std::cout << "ЛР-1: бинарное дерево поиска и множество\n";
    try
    {
        while (true)
        {
            std::cout << "\nТип элементов: 1. int   2. double   3. string   0. Выход\n";
            int type = ReadInt("Тип: ");
            if (type == 0) { break; }
            if (type == 1)
            {
                DemoFunctions<int> functions = {CompareValues<int>, ValueFromString<int>, IntMap, IntWhere, IntReduce, 0, IntFromKey,
                    "Map: изменить знак; Where: чётные; Reduce: сумма, старт 0."};
                RunDemo(functions);
            }
            else if (type == 2)
            {
                DemoFunctions<double> functions = {CompareValues<double>, DoubleFromString, DoubleMap, DoubleWhere, DoubleReduce, 0.0, DoubleFromKey,
                    "Map: изменить знак; Where: положительные; Reduce: сумма, старт 0."};
                RunDemo(functions);
            }
            else if (type == 3)
            {
                DemoFunctions<std::string> functions = {CompareValues<std::string>, StringFromString, StringMap, StringWhere, StringReduce,
                    "", StringFromKey, "Строки вводятся целой строкой. Map: добавить !; Where: непустые; Reduce: соединить."};
                RunDemo(functions);
            }
            else { std::cout << "Неизвестный тип.\n"; }
        }
    }
    catch (const EndOfInput&)
    {
        std::cout << "\nВвод завершён.\n";
    }
    catch (const std::exception& error)
    {
        std::cerr << "Ошибка: " << error.what() << '\n';
        return 1;
    }
    return 0;
}

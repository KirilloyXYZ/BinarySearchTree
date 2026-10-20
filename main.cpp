#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <string>

#include "BinarySearchTree.hpp"
#include "TreeSet.hpp"
#include "Student.hpp"

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
    std::string (*write)(const T&);
    T (*read)(const std::string&);
    T (*readKey)(const std::string&);
    T (*map)(const T&);
    bool (*where)(const T&);
    T (*reduce)(const T&, const T&);
    T startValue;
    T (*generate)(int);
    std::string description;
};

template<typename T, typename Container>
void PrintItems(const std::string& name, const Container& container, const DemoFunctions<T>& functions)
{
    std::unique_ptr<Sequence<T>> items = container.GetItems(25);
    std::cout << name << " (элементов: " << container.GetCount() << "): [";
    for (std::size_t i = 0; i < items->GetLength(); ++i)
    {
        if (i > 0) { std::cout << "; "; }
        std::cout << std::quoted(functions.write(items->Get(i)));
    }
    if (container.GetCount() > items->GetLength()) { std::cout << "; ..."; }
    std::cout << "]\n";
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
    int minimum = ReadInt("Минимальный ключ (для студентов ID должен быть положительным): ");
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
              << "1. Создать вручную   2. Сгенерировать   3. Показать дерево и результат\n"
              << "4. Добавить          5. Найти           6. Удалить\n"
              << "7. Map               8. Where          9. Reduce\n"
              << "10. Извлечь поддерево в результат       11. Проверить вхождение результата\n"
              << "12. Сохранить дерево в строку          13. Прочитать дерево из строки\n"
              << "14. Прочитать кандидат-поддерево в результат\n"
              << "15. Скопировать дерево в результат     16. Очистить дерево\n"
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
                    PrintItems("Дерево", *tree, functions);
                    break;
                }
                case 3:
                    PrintItems("Дерево", *tree, functions);
                    std::cout << "Высота: " << tree->GetHeight() << "; порядок и баланс: "
                              << (tree->IsValid() ? "верны" : "нарушены") << '\n';
                    std::cout << "Вид сбоку: правая ветка выше, левая ниже корня.\n";
                    tree->Print(std::cout, functions.write);
                    PrintItems("Последний результат / кандидат", *result, functions);
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
                    T value = ReadValue<T>("Искомое значение (для студента только ID): ", functions.readKey);
                    if (command == 5) { std::cout << (tree->Contains(value) ? "Найден.\n" : "Не найден.\n"); }
                    else { std::cout << (tree->Remove(value) ? "Удалён.\n" : "Не найден.\n"); }
                    break;
                }
                case 7:
                    result = tree->Map(functions.map);
                    PrintItems("Результат Map", *result, functions);
                    break;
                case 8:
                    result = tree->Where(functions.where);
                    PrintItems("Результат Where", *result, functions);
                    break;
                case 9:
                {
                    T reduced = tree->Reduce(functions.reduce, functions.startValue);
                    std::cout << "Reduce: " << functions.write(reduced) << '\n';
                    break;
                }
                case 10:
                {
                    T value = ReadValue<T>("Корень поддерева (для студента только ID): ", functions.readKey);
                    result = tree->GetSubtree(value);
                    PrintItems("Поддерево", *result, functions);
                    break;
                }
                case 11:
                    std::cout << (tree->ContainsSubtree(*result) ? "Поддерево входит.\n" : "Поддерево не входит.\n");
                    break;
                case 12:
                    std::cout << tree->ToString(functions.write) << '\n';
                    break;
                case 13:
                    tree->ReadFromString(ReadLine("Строка дерева: "), functions.read);
                    PrintItems("Восстановленное дерево", *tree, functions);
                    break;
                case 14:
                    result->ReadFromString(ReadLine("Строка кандидата: "), functions.read);
                    PrintItems("Кандидат", *result, functions);
                    break;
                case 15:
                    result = tree->Clone();
                    PrintItems("Копия", *result, functions);
                    break;
                case 16:
                    tree->Clear();
                    std::cout << "Дерево очищено.\n";
                    break;
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
              << "18. Очистить A   19. Очистить B   0. Назад\n";
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
                    PrintItems("A", *a, functions);
                    PrintItems("B", *b, functions);
                    break;
                }
                case 5:
                    PrintItems("A", *a, functions);
                    PrintItems("B", *b, functions);
                    PrintItems("Последний результат", *result, functions);
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
                    T value = ReadValue<T>("Искомое значение (для студента только ID): ", functions.readKey);
                    bool success = command == 7 ? a->Remove(value) : a->Contains(value);
                    std::cout << (success ? "Выполнено: элемент найден.\n" : "Элемент не найден.\n");
                    break;
                }
                case 9: result = a->Map(functions.map); break;
                case 10: result = a->Where(functions.where); break;
                case 11:
                {
                    T reduced = a->Reduce(functions.reduce, functions.startValue);
                    std::cout << "Reduce: " << functions.write(reduced) << '\n';
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
                default: std::cout << "Неизвестная команда.\n";
            }
            if (command == 9 || command == 10 || (command >= 12 && command <= 14) || command == 17)
            {
                PrintItems("Результат", *result, functions);
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
static Student StudentFromKey(int key) { return Student(key, "Ivan", "", "Student", "2005-01-01"); }
static Student ReadStudentKey(const std::string& text) { return StudentFromKey(ValueFromString<int>(text)); }
static Student StudentMap(const Student& value)
{
    return Student(value.GetID(), value.GetFirstName(), value.GetMiddleName(), value.GetLastName() + "+", value.GetBirthDate());
}
static bool StudentWhere(const Student& value) { return value.GetID() % 2 == 0; }
static Student StudentReduce(const Student& value, const Student& result)
{
    return value.GetID() <= result.GetID() ? value : result;
}

int main()
{
    std::cout << "ЛР-1: бинарное дерево поиска и множество\n";
    try
    {
        while (true)
        {
            std::cout << "\nТип элементов: 1. int   2. double   3. string   4. Student   0. Выход\n";
            int type = ReadInt("Тип: ");
            if (type == 0) { break; }
            if (type == 1)
            {
                DemoFunctions<int> functions = {CompareValues<int>, ValueToString<int>, ValueFromString<int>,
                    ValueFromString<int>, IntMap, IntWhere, IntReduce, 0, IntFromKey,
                    "Map: изменить знак; Where: чётные; Reduce: сумма, старт 0."};
                RunDemo(functions);
            }
            else if (type == 2)
            {
                DemoFunctions<double> functions = {CompareValues<double>, ValueToString<double>, DoubleFromString,
                    DoubleFromString, DoubleMap, DoubleWhere, DoubleReduce, 0.0, DoubleFromKey,
                    "Map: изменить знак; Where: положительные; Reduce: сумма, старт 0."};
                RunDemo(functions);
            }
            else if (type == 3)
            {
                DemoFunctions<std::string> functions = {CompareValues<std::string>, StringToString,
                    StringFromString, StringFromString, StringMap, StringWhere, StringReduce,
                    "", StringFromKey, "Строки вводятся целой строкой. Map: добавить !; Where: непустые; Reduce: соединить."};
                RunDemo(functions);
            }
            else if (type == 4)
            {
                DemoFunctions<Student> functions = {CompareStudents, StudentToString, StudentFromString,
                    ReadStudentKey, StudentMap, StudentWhere, StudentReduce,
                    Student(std::numeric_limits<int>::max(), "Empty", "", "Result", "2000-01-01"), StudentFromKey,
                    "Ввод: ID \"Имя\" \"Отчество\" \"Фамилия\" \"YYYY-MM-DD\".\n"
                    "Пример: 12 \"Иван\" \"\" \"Петров\" \"2005-01-01\". Ключ — ID.\n"
                    "Map: добавить + к фамилии; Where: чётные ID; Reduce: минимальный ID (пустой результат — максимальный ID типа int)."};
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

#include <iostream>
#include <string>

#include "BinarySearchTree.hpp"

std::string IntToString(const int& value)
{
    return std::to_string(value);
}

int main()
{
    BinarySearchTree<int> tree;
    int values[] = {8, 3, 10, 1, 6, 14, 4, 7, 13};

    for (int value : values)
    {
        tree.Insert(value);
    }

    std::cout << "Дерево после добавления:\n";
    tree.Print(std::cout, IntToString);
    std::cout << "Количество узлов: " << tree.GetCount() << '\n';
    std::cout << "Высота: " << tree.GetHeight() << '\n';
    std::cout << "Корень: " << tree.GetRoot() << '\n';
    std::cout << "Есть ли число 6: " << tree.Contains(6) << '\n';
    std::cout << "Есть ли число 100: " << tree.Contains(100) << '\n';

    tree.Remove(3);
    std::cout << "\nДерево после удаления числа 3:\n";
    tree.Print(std::cout, IntToString);
    std::cout << "Количество узлов: " << tree.GetCount() << '\n';

    tree.Clear();
    std::cout << "\nДерево после очистки:\n";
    tree.Print(std::cout, IntToString);

    return 0;
}

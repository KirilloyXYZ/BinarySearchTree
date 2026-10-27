#include <cstddef>
#include <iostream>
#include <memory>

#include "BinarySearchTree.hpp"

void ShowItems(const Tree<int>& tree)
{
    std::unique_ptr<Sequence<int>> items = tree.GetItems();
    std::cout << '[';
    for (std::size_t i = 0; i < items->GetLength(); ++i)
    {
        if (i > 0) { std::cout << "; "; }
        std::cout << items->Get(i);
    }
    std::cout << "]\n";
}

int Negate(const int& value)
{
    return -value;
}

bool Even(const int& value)
{
    return value % 2 == 0;
}

int Sum(const int& value, const int& result)
{
    return value + result;
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
    ShowItems(tree);
    std::cout << "Количество узлов: " << tree.GetCount() << '\n';
    std::cout << "Высота: " << tree.GetHeight() << '\n';
    std::cout << "Корень: " << tree.GetRoot() << '\n';
    std::cout << "Есть ли число 6: " << tree.Contains(6) << '\n';
    std::cout << "Есть ли число 100: " << tree.Contains(100) << '\n';

    std::unique_ptr<Tree<int>> mapped = tree.Map(Negate);
    std::cout << "\nMap: изменить знак:\n";
    ShowItems(*mapped);

    std::unique_ptr<Tree<int>> filtered = tree.Where(Even);
    std::cout << "\nWhere: оставить чётные:\n";
    ShowItems(*filtered);

    std::cout << "\nReduce: сумма = " << tree.Reduce(Sum, 0) << '\n';

    tree.Remove(3);
    std::cout << "\nДерево после удаления числа 3:\n";
    ShowItems(tree);
    std::cout << "Количество узлов: " << tree.GetCount() << '\n';

    tree.Clear();
    std::cout << "\nДерево после очистки:\n";
    ShowItems(tree);

    return 0;
}

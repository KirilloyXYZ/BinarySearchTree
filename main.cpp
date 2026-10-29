#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

#include "BinarySearchTree.hpp"

template<typename T, typename K>
void ShowItem(const K& key, const T& value)
{
    std::cout << key << " : " << value << "  ";
}

template<typename T, typename K>
void ShowItems(const Tree<T, K>& tree, TraversalOrder order = TraversalOrder::LKP)
{
    tree.Traverse(order, ShowItem<T, K>);
    std::cout << '\n';
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
    BinarySearchTree<int, int> tree;
    int keys[] = {4, 2, 6, 1, 3, 5, 7};

    for (int key : keys)
    {
        tree.Insert(key, key + 10);
    }

    std::cout << "Дерево после добавления (ключ : данные):\n";
    ShowItems(tree);
    std::cout << "Количество узлов: " << tree.GetCount() << '\n';
    std::cout << "Высота: " << tree.GetHeight() << '\n';
    std::cout << "Ключ корня: " << tree.GetRootKey() << '\n';
    std::cout << "Данные корня: " << tree.GetRoot() << '\n';
    std::cout << "Данные по ключу 6: " << tree.Get(6) << '\n';
    std::cout << "Есть ли ключ 100: " << tree.Contains(100) << '\n';

    TraversalOrder orders[] = {TraversalOrder::KLP, TraversalOrder::KPL, TraversalOrder::LKP,
                               TraversalOrder::LPK, TraversalOrder::PLK, TraversalOrder::PKL};
    std::string names[] = {"КЛП", "КПЛ", "ЛКП", "ЛПК", "ПЛК", "ПКЛ"};
    std::cout << "\nШесть обходов:\n";
    for (std::size_t i = 0; i < 6; ++i)
    {
        std::cout << names[i] << ": ";
        ShowItems(tree, orders[i]);
    }

    std::unique_ptr<Tree<int, int>> mapped = tree.Map(Negate);
    std::cout << "\nMap: изменить знак:\n";
    ShowItems(*mapped);

    std::unique_ptr<Tree<int, int>> filtered = tree.Where(Even);
    std::cout << "\nWhere: оставить чётные:\n";
    ShowItems(*filtered);

    std::cout << "\nReduce: сумма = " << tree.Reduce(Sum, 0) << '\n';

    tree.Remove(3);
    std::cout << "\nДерево после удаления ключа 3:\n";
    ShowItems(tree);
    std::cout << "Количество узлов: " << tree.GetCount() << '\n';

    tree.Clear();
    std::cout << "\nДерево после очистки:\n";
    ShowItems(tree);

    BinarySearchTree<std::string, int> people;
    people.Insert(20, "Кирилл");
    people.Insert(10, "Анна");
    std::cout << "\nПример со строками и числовыми ключами:\n";
    ShowItems(people);

    return 0;
}

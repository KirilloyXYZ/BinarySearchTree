#ifndef TRAVERSALORDER_HPP
#define TRAVERSALORDER_HPP

enum class TraversalOrder
{
    KLP, // Root, left, right.
    KPL, // Root, right, left.
    LKP, // Left, root, right.
    LPK, // Left, right, root.
    PLK, // Right, left, root.
    PKL  // Right, root, left.
};

#endif

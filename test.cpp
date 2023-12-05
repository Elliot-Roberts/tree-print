#include"bintree.h"
#include"treeHelper.hpp"
#include<cstdio>
#include<iostream>
using namespace std;

void cube(Node * n) {
    if (n == nullptr) return;
    n->data *= n->data;
    cube(n->left);
    cube(n->right);
}

void change_nulls_to(Node *& n, Node * x) {
    if (n == nullptr) {
        n = x;
        return;
    }
    change_nulls_to(n->left, x);
    change_nulls_to(n->right, x);
}

Node *& random_null(Node *& n) {
    if (n == nullptr) return n;
    if (rand() % 2) {
        return random_null(n->left);
    } else {
        return random_null(n->right);
    }
}

Node * random_node(Node * n) {
    switch (rand() % 3) {
        case 0: 
            {
                if (n->left) {
                    return random_node(n->left);
                } else {
                    return n;
                }
                break;
            }
        case 1: 
            {
                if (n->right) {
                    return random_node(n->right);
                } else {
                    return n;
                }
                break;
            }
        default: 
            {
                return n;
                break;
            }
    }
}

int main() {
    auto root = rand_gen(20, 7);
    //cube(root);
    for (int i = 0; i < 5; ++i) {
        random_null(root) = random_node(root);
    }
    WrappedTree<Node> wt(root);
    cout << wt;
    // auto & n = find(root, 7);
    // if (n != nullptr) remove_node(n);
    // auto [oldt, newt] = wt.compare_to(WrappedTree(root));
    // cout << oldt << newt;
    // destroy(root);
}

#include"bintree.h"
#include"treeHelper.h"
#include<iostream>
using namespace std;

int main() {
    auto root = rand_gen(10, 7);
    WrappedTree<Node> wt(root);
    wt.color_node(root, GREEN);
    cout << wt
         << endl;
    balance(root);
    cout << WrappedTree(root);
    destroy(root);
}

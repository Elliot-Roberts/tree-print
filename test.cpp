#include"bintree.h"
#include"treeHelper.h"
#include<iostream>
using namespace std;

int main() {
    auto root = rand_gen(10, 7);
    WrappedTree<Node> wt(root);
    // wt.color_node(root, GREEN);
    // cout << wt
    //      << endl;
    balance(root);
    insert(root, 200);
    auto [oldt, newt] = wt.compare_to(WrappedTree(root));
    cout << oldt << newt;
    cout << horizontal_concat(oldt.draw(), newt.draw());
    destroy(root);
}

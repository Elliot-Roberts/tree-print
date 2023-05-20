#include"bintree.h"
#include"treeHelper.h"
#include<cstdio>
#include<iostream>
using namespace std;

int main() {
    auto root = rand_gen(10, 7);
    WrappedTree<Node> wt(root);
    // wt.color_node(root, GREEN);
    // cout << wt
    //      << endl;
    // cout << wt;
    // insert(root, 200);
    auto & n = find(root, 7);
    if (n != nullptr) remove_node(n);
    auto [oldt, newt] = wt.compare_to(WrappedTree(root));
    cout << oldt << newt;
    // cout << horizontal_concat(oldt.draw(), newt.draw());
    destroy(root);
    /*
    auto str = string("┐");
    cout << str.size() << endl;
    for (auto c : str) {
        printf("%x\n", c);
    }
    */
}

#include"bintree.h"
#include"treeHelper.h"
#include<iostream>
using namespace std;

int main() {
    auto root = rand_gen(10, 7);
    WrappedTree<Node> wt(root);
    wt.color_node(root, GREEN);
    cout << wt.draw()
         << endl;
    balance(root);
    WrappedTree wt2(root);
    cout << wt2.draw();
    destroy(root);
}

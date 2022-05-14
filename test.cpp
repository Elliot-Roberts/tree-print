#include"bintree.h"
#include"treeHelper.h"
#include<iostream>
using namespace std;

int main() {
    auto root = rand_gen(1, 16);
    WrappedTree<Node> wt(root);
    wt.color_node(root, GREEN);
    cout << wt.draw() << endl;
    destroy(root);
}

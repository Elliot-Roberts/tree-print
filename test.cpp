#include"bintree.h"
#include"treeHelper.h"
#include<iostream>
using namespace std;

int main() {
    auto root = rand_gen(1, 16);
    TreeHelper<Node> th(root);
    cout << th.draw() << endl;
    // cout << th.draw2() << endl;
    // cout << th.draw3() << endl;
    destroy(root);
}

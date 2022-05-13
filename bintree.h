#pragma once
#include<cstdlib>
#include<ctime>

struct Node {
    int data;
    Node* left;
    Node* right;
    Node(int data, Node* left = nullptr, Node* right = nullptr):
        data(data), left(left), right(right) {}
};

Node* insert(Node* &n, int value) {
    if (n == nullptr) {
        n = new Node(value);
        return n;
    }
    if (value < n->data) {
        return insert(n->left, value);
    } else {
        return insert(n->right, value);
    }
}

Node* rand_gen(size_t range = 16, size_t min = 0) {
    srand(time(nullptr));
    size_t size = min + (rand() % range);
    Node* root = nullptr;
    for (size_t i = 0; i < size; ++i) {
        insert(root, rand() % size);
    }
    return root;
}

void destroy(Node* n) {
    if (n == nullptr) return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

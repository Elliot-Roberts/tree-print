#pragma once
#include<cstdlib>
#include<chrono>
#include<utility>

struct Node {
    int data;
    Node* left;
    Node* right;
    Node(int data, Node* left = nullptr, Node* right = nullptr):
        data(data), left(left), right(right) {}
};

Node* & place(Node* &n, int x) {
    if (n == nullptr) return n;
    if (x < n->data) return place(n->left, x);
    else return place(n->right, x);
}

Node* & find(Node* &n, int x) {
    if (n == nullptr) return n;
    if (x == n->data) return n;
    if (x < n->data) return find(n->left, x);
    else return find(n->right, x);
}

Node* insert(Node* &n, int value) {
    return place(n, value) = new Node(value);
}

Node* rand_gen(size_t min = 0, size_t range = 16) {
    srand(std::chrono::high_resolution_clock::now().time_since_epoch().count());
    size_t size = min + (rand() % range);
    Node* root = nullptr;
    for (size_t i = 0; i < size; ++i) {
        insert(root, rand() % size);
    }
    return root;
}

std::size_t size(const Node* n) {
    if (n == nullptr) return 0;
    return size(n->left) + 1 + size(n->right);
}

Node* balanced_from_array(Node** arr, std::size_t len) {
    if (len == 0) return nullptr;
    auto mid = len / 2;
    auto n = arr[mid];
    n->left = balanced_from_array(arr, len-mid-(len&1));
    n->right = balanced_from_array(arr+mid+1, len-mid-1);
    return n;
}

std::size_t to_array(const Node* n, const Node** arr) {
    if (n == nullptr) return 0;
    auto lsize = to_array(n->left, arr);
    arr[lsize] = n;
    auto rsize = to_array(n->right, arr+lsize+1);
    return lsize + 1 + rsize;
}

void balance(Node* &n) {
    auto nsize = size(n);
    Node** arr = new Node* [nsize];
    to_array(n, const_cast<const Node**>(arr));
    n = balanced_from_array(arr, nsize);
    delete arr;
}

// min and max assume n is not null
Node* & min(Node* &n) {
    if (n->left == nullptr) return n;
    return min(n->left);
}
Node* & max(Node* &n) {
    if (n->right == nullptr) return n;
    return max(n->right);
}

void remove_node(Node* &n) {
    Node * temp = n;
    if (n->left == nullptr) {
        n = n->right;
    } else if (n->right == nullptr) {
        n = n->left;
    } else {
        Node* &pred = max(n->left);
        n = pred;
        pred = pred->left;
        n->left = temp->left;
        n->right = temp->right;
    }
    delete temp;
}

void destroy(Node* n) {
    if (n == nullptr) return;
    destroy(n->left);
    destroy(n->right);
    delete n;
}

#include <bits/stdc++.h>
using namespace std;

struct Node {
    int val, priority, size;
    bool reversed = false;
    Node *left = nullptr, *right = nullptr;

    Node(int v)
        : val(v), priority(rand()), size(1) {
    }
};

int getSize(Node *t) {
    return t ? t->size : 0;
}

void update(Node *t) {
    if(t) {
        t->size = 1 + getSize(t->left) + getSize(t->right);
    }
}

// 遅延評価（reverse の伝播）
void push(Node *t) {
    if(t && t->reversed) {
        swap(t->left, t->right);
        if(t->left)
            t->left->reversed ^= true;
        if(t->right)
            t->right->reversed ^= true;
        t->reversed = false;
    }
}

// treap t を k 個目で分割： [0, k) と [k, N)
void split(Node *t, int k, Node *&left, Node *&right) {
    if(!t) {
        left = right = nullptr;
        return;
    }
    push(t);
    if(getSize(t->left) >= k) {
        split(t->left, k, left, t->left);
        right = t;
    } else {
        split(t->right, k - getSize(t->left) - 1, t->right, right);
        left = t;
    }
    update(t);
}

Node *merge(Node *left, Node *right) {
    if(!left || !right)
        return left ? left : right;
    push(left);
    push(right);
    if(left->priority > right->priority) {
        left->right = merge(left->right, right);
        update(left);
        return left;
    } else {
        right->left = merge(left, right->left);
        update(right);
        return right;
    }
}

void reverseSegment(Node *&root, int l, int r) {
    Node *t1, *t2, *t3;
    split(root, r + 1, t2, t3); // [0, r] と [r+1, N)
    split(t2, l, t1, t2);       // [0, l-1], [l, r]
    if(t2)
        t2->reversed ^= true; // 反転フラグをつける
    root = merge(merge(t1, t2), t3);
}

void inorder(Node *t, vector<int> &res) {
    if(!t)
        return;
    push(t);
    inorder(t->left, res);
    res.push_back(t->val);
    inorder(t->right, res);
}

int main() {
    srand(time(0));
    Node *root = nullptr;

    // 巡回路を初期化（例：0〜9）
    for(int i = 0; i < 10000; ++i) {
        root = merge(root, new Node(i));
    }

    // 部分反転（2-opt に相当）
    for(int i = 0; i < 10000; ++i) {
        int l = rand() % 10000;
        int r = rand() % 10000;
        if(l > r)
            swap(l, r);
        reverseSegment(root, l, r);
    }
    vector<int> tour;
    inorder(root, tour);
    for(int v : tour)
        cout << v << " ";
    cout << endl;

    // reverseSegment(root, 1, 5);

    // tour = {};
    // inorder(root, tour);
    // for(int v : tour)
    //     cout << v << " ";
    // cout << endl;

    return 0;
}

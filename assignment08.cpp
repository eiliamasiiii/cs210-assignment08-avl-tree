#include <array>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

struct Node {
    int key;
    int height = 0; // Edge height: leaf = 0, null = -1.
    Node* left = nullptr;
    Node* right = nullptr;
    explicit Node(int value) : key(value) {}
};

void destroy(Node* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}
void collectInorder(const Node* node, std::vector<int>& out) {
    if (!node) return;
    collectInorder(node->left, out);
    out.push_back(node->key);
    collectInorder(node->right, out);
}

class AVLTree {
    Node* root = nullptr;
    // Index order: LL, RR, LR, RL. A double rotation counts as one case.
    std::array<int, 4> cases{};
    static int h(const Node* node) { return node ? node->height : -1; }
    static void update(Node* node) {
        const int left = h(node->left), right = h(node->right);
        node->height = 1 + (left > right ? left : right);
    }
    static int balance(const Node* node) {
        return h(node->left) - h(node->right);
    }
    static Node* rotateRight(Node* top) {
        Node* promoted = top->left;
        top->left = promoted->right;
        promoted->right = top;
        update(top); // Update the demoted node before its new parent.
        update(promoted);
        return promoted;
    }
    static Node* rotateLeft(Node* top) {
        Node* promoted = top->right;
        top->right = promoted->left;
        promoted->left = top;
        update(top);
        update(promoted);
        return promoted;
    }
    void announce(int index, const char* label, int key) {
        ++cases[index];
        std::cout << "Rebalance at " << key << ": " << label << '\n';
    }
    Node* add(Node* node, int key, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(key);
        }
        if (key < node->key) node->left = add(node->left, key, inserted);
        else if (key > node->key) node->right = add(node->right, key, inserted);
        else return node; // Ignore duplicate keys.
        if (!inserted) return node;
        update(node);
        const int b = balance(node);
        if (b > 1) {
            if (key < node->left->key) {
                announce(0, "LL -> right rotation", node->key);
            } else {
                announce(2, "LR -> left then right rotation", node->key);
                node->left = rotateLeft(node->left);
            }
            return rotateRight(node);
        }
        if (b < -1) {
            if (key > node->right->key) {
                announce(1, "RR -> left rotation", node->key);
            } else {
                announce(3, "RL -> right then left rotation", node->key);
                node->right = rotateRight(node->right);
            }
            return rotateLeft(node);
        }
        return node;
    }
    struct Audit { bool valid; int height; };
    static Audit audit(const Node* node, const Node* lower, const Node* upper) {
        if (!node) return {true, -1};
        const auto left = audit(node->left, lower, node);
        const auto right = audit(node->right, node, upper);
        const int actual = 1 + (left.height > right.height
                               ? left.height : right.height);
        const int b = left.height - right.height;
        return {left.valid && right.valid &&
                (!lower || node->key > lower->key) &&
                (!upper || node->key < upper->key) &&
                b >= -1 && b <= 1 && node->height == actual, actual};
    }
    static void collectPreorder(const Node* node, std::vector<int>& out) {
        if (!node) return;
        out.push_back(node->key);
        collectPreorder(node->left, out);
        collectPreorder(node->right, out);
    }
public:
    AVLTree() = default;
    ~AVLTree() { destroy(root); }
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;
    bool insert(int key) {
        bool inserted = false;
        root = add(root, key, inserted);
        return inserted;
    }
    bool search(int key) const {
        const Node* node = root;
        while (node) {
            if (node->key == key) return true;
            node = key < node->key ? node->left : node->right;
        }
        return false;
    }
    int height() const { return h(root); }
    bool valid() const { return audit(root, nullptr, nullptr).valid; }
    std::array<int, 4> caseCounts() const { return cases; }
    std::vector<int> inorder() const {
        std::vector<int> out;
        collectInorder(root, out);
        return out;
    }
    std::vector<int> preorder() const {
        std::vector<int> out;
        collectPreorder(root, out);
        return out;
    }
};

class BinarySearchTree {
    Node* root = nullptr;
    // This ordinary BST computes height recursively and never rotates.
    static int actualHeight(const Node* node) {
        if (!node) return -1;
        const int left = actualHeight(node->left);
        const int right = actualHeight(node->right);
        return 1 + (left > right ? left : right);
    }
public:
    BinarySearchTree() = default;
    ~BinarySearchTree() { destroy(root); }
    BinarySearchTree(const BinarySearchTree&) = delete;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;
    bool insert(int key) {
        Node** link = &root;
        while (*link) {
            if ((*link)->key == key) return false;
            link = key < (*link)->key ? &(*link)->left : &(*link)->right;
        }
        *link = new Node(key);
        return true;
    }
    int height() const { return actualHeight(root); }
    std::vector<int> inorder() const {
        std::vector<int> out;
        collectInorder(root, out);
        return out;
    }
};

int checks = 0;
void check(bool passed, const std::string& label) {
    ++checks;
    std::cout << (passed ? "PASS: " : "FAIL: ") << label << '\n';
    if (!passed) throw std::runtime_error(label);
}
void print(const std::string& label, const std::vector<int>& values) {
    std::cout << label;
    for (int key : values) std::cout << ' ' << key;
    std::cout << '\n';
}
void testCase(const std::string& label, const std::vector<int>& input,
              int caseIndex, const std::vector<int>& expectedPreorder) {
    std::cout << "\n" << label << " test\n";
    print("Insertion order:", input);
    AVLTree tree;
    bool valid = true;
    for (int key : input) valid &= tree.insert(key) && tree.valid();
    std::array<int, 4> expectedCounts{};
    expectedCounts[caseIndex] = 1;
    check(valid && tree.caseCounts() == expectedCounts,
          "exactly the expected case; AVL rules hold after each insert");
    print("Preorder:", tree.preorder());
    print("Inorder:", tree.inorder());
    check(tree.preorder() == expectedPreorder && tree.height() == 1,
          "expected three-node shape and height");
}
int main() {
    testCase("LL", {63, 38, 14}, 0, {38, 14, 63});
    testCase("RR", {17, 42, 69}, 1, {42, 17, 69});
    testCase("LR", {82, 24, 56}, 2, {56, 24, 82});
    testCase("RL", {13, 76, 49}, 3, {49, 13, 76});

    std::cout << "\nSAME-ORDER HEIGHT COMPARISON\n";
    const std::vector<int> input =
        {6,12,18,24,30,36,42,48,54,60,66,72,78,84,90};
    print("Insertion order:", input);
    AVLTree avl;
    BinarySearchTree bst;
    check(avl.height() == -1 && avl.valid() && !avl.search(6) &&
          bst.height() == -1, "empty trees");
    bool valid = true;
    for (int key : input) {
        valid &= avl.insert(key) && bst.insert(key);
        valid &= avl.valid();
    }
    check(valid, "all 15 insertions preserve global BST order, heights, balance");
    print("AVL inorder:", avl.inorder());
    print("BST inorder:", bst.inorder());
    print("AVL preorder:", avl.preorder());
    std::cout << "AVL height: " << avl.height() << " edges\n";
    std::cout << "BST height: " << bst.height() << " edges\n";
    check(avl.inorder() == input && bst.inorder() == input,
          "both trees contain exactly the same sorted keys");
    check(avl.height() == 3 && bst.height() == 14,
          "AVL height 3 versus ordinary BST height 14");
    bool foundAll = true;
    for (int key : input) foundAll &= avl.search(key);
    check(foundAll && !avl.search(45), "all inserted keys found; 45 absent");
    const auto shape = avl.preorder();
    const auto counts = avl.caseCounts();
    check(!avl.insert(48) && !bst.insert(48) && avl.preorder() == shape &&
          avl.caseCounts() == counts && avl.valid(),
          "duplicate rejected without changing AVL shape or rotation counts");
    AVLTree single;
    single.insert(-7);
    check(single.height() == 0 && single.valid() && single.search(-7),
          "single negative key has height zero");
    std::cout << "\nAll " << checks << " checks passed.\n";
}

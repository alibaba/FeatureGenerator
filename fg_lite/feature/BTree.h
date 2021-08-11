#pragma once

#include <vector>
#include <queue>
#include "autil/mem_pool/Pool.h"
#include "autil/mem_pool/pool_allocator.h"

typedef uint32_t  KeyNumType;
typedef uint32_t BlockSizeType;

namespace std {

template<class Fun>
class y_combinator_result {
    Fun fun_;
public:
    template<class T>
    explicit y_combinator_result(T &&fun): fun_(std::forward<T>(fun)) {}

    template<class ...Args>
    decltype(auto) operator()(Args &&...args) {
        return fun_(std::ref(*this), std::forward<Args>(args)...);
    }
};
template<class Fun>
decltype(auto) y_combinator(Fun &&fun) {
    return y_combinator_result<std::decay_t<Fun>>(std::forward<Fun>(fun));
}

} // namespace std

namespace fg_lite {

template<typename T>
using pool_vector = std::vector<T, autil::mem_pool::pool_allocator<T>>;

template<typename KeyType, typename ValueType>
struct BTreeNode {
    std::vector<KeyType> keys;
    std::vector<ValueType> values;
    std::vector<BTreeNode<KeyType, ValueType>*> children;
    bool isLastLeaf;
    BTreeNode(){
        isLastLeaf = false;
    }
    ~BTreeNode() {
        for (auto &child : children) {
            delete child;
        }
    }
};

template<typename KeyType, typename ValueType>
class BTreeVisitor {
public:
    virtual void append(BTreeNode<KeyType, ValueType>* cur) = 0;
    virtual ~BTreeVisitor() = default;
};

template<typename KeyType, typename ValueType>
class BTree {
public:
    BTree(const int nodeNum, const int blockSize)
        : _nodeNum(nodeNum)
        , _blockSize(blockSize)
        , _branchSize(blockSize + 1)
    {
        assert(nodeNum >= 0);
        _remain = (nodeNum % _blockSize)? nodeNum % _blockSize: _blockSize;
        if (nodeNum == 0) _remain = 0;
        _root = new BTreeNode<KeyType, ValueType> ();
        if (nodeNum == 0) {
            _root->isLastLeaf = true;
            return;
        }
        std::queue<BTreeNode<KeyType, ValueType>* > q;
        q.emplace(_root);
        BTreeNode<KeyType, ValueType>* cur = nullptr;
        int childs = _blockSize + 1;
        for(int cnt = _blockSize; cnt < nodeNum; cnt += childs * _blockSize) {
            assert(!q.empty());
            cur = q.front();
            q.pop();
            if (nodeNum - cnt < childs * _blockSize) {
                childs = (nodeNum - cnt + _blockSize - 1) / _blockSize;
            }
            for (int i = 0; i < childs; ++i) {
                cur->children.emplace_back(new BTreeNode<KeyType, ValueType> ());
                q.emplace(cur->children[i]);
            }
        }
        // get the last leaf
        while(!q.empty()) {
            cur = q.front();
            q.pop();
        }
        if(cur) {
            cur->isLastLeaf = true;
        }
    }
    ~BTree() {
        delete _root;
    }
    BTreeNode<KeyType, ValueType>* getRoot() {
        return _root;
    }
    void preOrder(BTreeVisitor<KeyType, ValueType>* visitor) {
        assert(nullptr != _root);
        std::queue<BTreeNode<KeyType, ValueType>* > q;
        q.emplace(_root);
        while (!q.empty()) {
            BTreeNode<KeyType, ValueType>* cur = q.front();
            q.pop();
            visitor->append(cur);
            for (auto& entity : cur->children) {
                q.emplace(entity);
            }
        }

    }
    void middleOrder(BTreeVisitor<KeyType, ValueType> *visitor) {
        assert(nullptr != _root);
        // Todo: convert to non-recursive version
        std::y_combinator(
                [&](auto self, BTreeNode<KeyType, ValueType>* cur)->void{
                    for (auto& entity : cur->children) {
                        self(entity);
                        if (cur->keys.size() < _blockSize) {
                            // one child, one parent
                            visitor->append(cur);
                        }
                    }
                    if (!cur->children.size()) {
                        int limit = (cur->isLastLeaf)? _remain:_blockSize;
                        for (int i = 0; i < limit; ++i) {
                            // if it's the last block, be careful
                            visitor->append(cur);
                        }
                    }
                    if (!cur->isLastLeaf) {
                        while(cur->keys.size() < _blockSize) {
                            visitor->append(cur);
                        }
                    }
                })(_root);
    }
private:
    int _nodeNum;
    int _remain;
public:
    int _blockSize;
    int _branchSize;
    BTreeNode<KeyType, ValueType>* _root;
};

}

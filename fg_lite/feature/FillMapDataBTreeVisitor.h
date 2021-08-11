#pragma once

#include <map>
#include "fg_lite/feature/BTree.h"

namespace fg_lite {

template<typename KeyType, typename ValueType>
class FillMapDataBTreeVisitor : public BTreeVisitor<KeyType, ValueType>  {
public:
    FillMapDataBTreeVisitor(const std::map<KeyType, std::vector<ValueType> > &doc, int blockSize) {
        _iter = doc.begin();
        _iterEnd = doc.end();
    }
private:
    std::pair<KeyType, std::vector<ValueType> > getKV() {
        auto key = (_iter)->first;
        return std::make_pair(key, (_iter++)->second);
    }
    void append(BTreeNode<KeyType, ValueType>* cur) override {
        // directly append cur.keys, make sure it's correct
        if (_iter == _iterEnd) {
            return;
        }
        auto tmp = getKV();
        cur->keys.push_back(tmp.first);
        for (auto val : tmp.second) {
            cur->values.push_back(val);
        }
    }
public:
    typename std::map<KeyType, std::vector<ValueType> >::const_iterator _iter;
    typename std::map<KeyType, std::vector<ValueType> >::const_iterator _iterEnd;
};

} // namespace fg_lite

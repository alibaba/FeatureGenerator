#pragma once

#include "fg_lite/feature/BTree.h"

namespace fg_lite {

struct headInfoStruct {
    KeyNumType keyNum;
    BlockSizeType blockSize;
    uint8_t typeInfo;
    char padding1;
    char padding2;
    char padding3;
    headInfoStruct(KeyNumType _keyNum, BlockSizeType _blockSize, uint8_t _typeInfo):
        keyNum(_keyNum), blockSize(_blockSize),  typeInfo(_typeInfo){
        // for memory padding
        padding1 = '0';
        padding2 = '0';
        padding3 = '0';
    }
};

template<typename KeyType, typename ValueType>
class StoreMapDataBTreeVisitor : public BTreeVisitor<KeyType, ValueType> {
public:
    StoreMapDataBTreeVisitor(KeyNumType keyNum, BlockSizeType blockSize,
                             const uint8_t typeInfo,
                             const size_t& dimension = 1) {
        // headInfoLength, default = 12 (actual = 4 + 4 + 4 = 12)
        size_t headInfoLength = sizeof(headInfoStruct);
        _length = keyNum * (sizeof(KeyType) + sizeof(ValueType) * dimension) + headInfoLength;
        _data = new char [_length];
        fillheadInfo(_data, keyNum, blockSize, typeInfo);
        _fillKeyAddr =  (KeyType*)(_data + headInfoLength);
        _fillValueAddr = (ValueType*) (_data + headInfoLength + keyNum * sizeof(KeyType));
        _keyNum = keyNum;
    }
    ~StoreMapDataBTreeVisitor() {
        delete[] _data;
    }
private:
    void fillheadInfo(char * data, KeyNumType keyNum, BlockSizeType blockSize,
                      const uint8_t& getHeadInfoRet) {
        auto headInfoAddr = (headInfoStruct*) (data);
        *headInfoAddr = headInfoStruct(keyNum, blockSize, getHeadInfoRet);
    }
public:
    // store map data into a string
    void append(BTreeNode<KeyType, ValueType>* cur) override {
        // note that keys.size() * dimension == values.size()
        for (auto key : cur->keys) {
            *(_fillKeyAddr ++) = key;
        }
        for (auto val : cur->values) {
            *(_fillValueAddr ++) = val;
        }
    }
    std::string toString() {
        return std::string(_data, _length);
    }
private:
    std::string _output; //format: key1value1key2value2...
    char * _data;
    size_t _length;
    KeyType* _fillKeyAddr;
    ValueType* _fillValueAddr;
    size_t _keyNum;
};

} // namespace fg_lite

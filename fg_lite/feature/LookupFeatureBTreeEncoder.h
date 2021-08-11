#pragma once

#include <cpuid.h>
#include "fg_lite/feature/FillMapDataBTreeVisitor.h"
#include "fg_lite/feature/StoreMapDataBTreeVisitor.h"
#include "fg_lite/feature/BTree.h"
#include "fg_lite/feature/LookupFeatureDataType.h"

namespace fg_lite {


class LookupFeatureBTreeEncoder
{
public:
    LookupFeatureBTreeEncoder() = default;
    ~LookupFeatureBTreeEncoder() = default;

public:

    template<typename KeyType, typename ValueType>
    static std::string encode(const std::map<KeyType, std::vector<ValueType> > &doc,
                              BlockSizeType blockSize, const size_t& dimension,
                              size_t  minHashType,
                              size_t  appointedValueType)
    {
        BTree<KeyType, ValueType> tree(doc.size(), blockSize);
        FillMapDataBTreeVisitor<KeyType, ValueType>  visitor1(doc, blockSize);
        tree.middleOrder(&visitor1);
        const uint8_t headInfo = getHeadInfo(
                (LookupFeatureV3KeyType)minHashType
                ,(LookupFeatureV3ValueType)appointedValueType);
        StoreMapDataBTreeVisitor <KeyType, ValueType> visitor2(doc.size(), blockSize, headInfo, dimension);
        tree.preOrder(&visitor2);
        return visitor2.toString();
    }

private:
    AUTIL_LOG_DECLARE();
};

}

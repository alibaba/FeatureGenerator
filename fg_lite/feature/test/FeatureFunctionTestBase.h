#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <functional>
#include "autil/MultiValueCreator.h"
#include "autil/MultiValueType.h"
#include "fg_lite/feature/FeatureFunction.h"

#define ASSERT_CAST_AND_RETURN(expectType, pointer)                                                                    \
    ({                                                                                                                 \
        auto p = pointer;                                                                                              \
        ASSERT_TRUE(p) << "pointer is NULL";                                                                           \
        expectType *castResult__ = dynamic_cast<expectType *>(p);                                                      \
        ASSERT_TRUE(castResult__) << "Expect: " #expectType << " Actual: " << typeid(*p).name();                       \
        castResult__;                                                                                                  \
    })

namespace std {
template<typename _Tp, typename _Alloc1, typename _Alloc2>
inline bool
operator==(const std::vector<_Tp, _Alloc1>& __x, const std::vector<_Tp, _Alloc2>& __y)
{ return (__x.size() == __y.size()
          && std::equal(__x.begin(), __x.end(), __y.begin())); }

}

using namespace std;
using namespace testing;
using namespace autil;
using namespace autil::mem_pool;

namespace fg_lite {

class FeatureFunctionTestBase : public ::testing::Test {
public:
    void SetUp() override {
    }
    void TearDown() override {
        for (auto f : _cleanups) {
            f();
        }
    }
protected:
    template<typename T>
    vector<T> *copyVector(const vector<T> &values) {
        auto newValues = new std::vector<T>(values);
        _cleanups.emplace_back([newValues]{ delete newValues; });
        return newValues;
    }
    typedef std::function<FeatureFunction*()> FeatureCreator;
    // input str: v1,v2,v3,....
    template <typename T>
    FeatureInput *genDenseInput(const std::vector<T> &values) {
        return genDenseInput(values, values.size(), 1);
    }
    template <typename T>
    FeatureInput *genDenseInput(const std::vector<T> &values, size_t r, size_t c) {
        if (r * c != values.size()) {
            return nullptr;
        }
        auto data = copyVector(values)->data();
        if (r * c == 0) {
            data = nullptr;
        }
        DenseStorage<T> storage(data, r, c);
        return new FeatureInputTyped<T, DenseStorage<T>>(storage);
    }

    template <typename T, typename ResultType = autil::MultiValueType<T> >
    vector<ResultType> genMultiValues(const vector<vector<T>> &values) {
        vector<ResultType> multiValues;
        for (size_t i = 0; i < values.size(); i++) {
            char* buf = autil::MultiValueCreator::createMultiValueBuffer(values[i], _pool.get());
            ResultType multiValue(buf);
            multiValues.emplace_back(multiValue);
        }
        return multiValues;
    }

    vector<autil::MultiChar> genMultiCharValues(const vector<string> &values) {
        vector<autil::MultiChar> multiValues;
        for (size_t i = 0; i < values.size(); i++) {
            const string &value = values[i];
            char* buf = autil::MultiValueCreator::createMultiValueBuffer(
                    value.data(), value.size(), _pool.get());
            autil::MultiChar multiValue(buf);
            multiValues.emplace_back(multiValue);
        }
        return multiValues;
    }

    vector<MultiString> genMultiStringValues(const vector<vector<string>> &values) {
        vector<MultiString> multiValues;
        for (size_t i = 0; i < values.size(); i++) {
            char* buf = autil::MultiValueCreator::createMultiStringBuffer(values[i], _pool.get());
            MultiString multiString;
            multiString.init(buf);
            multiValues.emplace_back(multiString);
        }
        return multiValues;
    }

    template <typename T>
    FeatureInput *genMultiValueInput(const vector<autil::MultiValueType<T>> &multiValues) {
        MultiValueStorage<T> storage(copyVector(multiValues)->data(), multiValues.size());
        return new FeatureInputTyped<T, MultiValueStorage<T>>(storage);
    }

    template <typename T>
    FeatureInput *genValueOffsetInput(const std::vector<T> &values,
            const std::vector<size_t> &offset)
    {
        ValueOffsetStorage<T> storage(copyVector(values)->data(), values.size(),
                copyVector(offset)->data(), offset.size());
        return new FeatureInputTyped<T, ValueOffsetStorage<T>>(storage);
    }

    void genFeatures(const std::vector<FeatureInput*> &inputs,
                     FeatureCreator creator)
    {
        unique_ptr<FeatureFunction> featureFunction(creator());
        ASSERT_TRUE(featureFunction.get() != nullptr);
        _features.reset(featureFunction->genFeatures(inputs, &_context));
    }

    vector<unique_ptr<FeatureInput>> genStringFeatureInput(const vector<vector<string>> &input){
        vector<unique_ptr<FeatureInput>> stringFeatureInput;
        if(input.size() == 0) return stringFeatureInput;
        bool denseEnable = true;
        size_t r = input.size();
        size_t c = input[0].size();
        for(size_t i = 0; i < input.size(); ++i){
            if(input[i].size() != c){
                denseEnable = false;
                break;
            }
        }
        unique_ptr<FeatureInput> multiCharMultiValue(genMultiValueInput<MultiChar>(genMultiStringValues(input)));
        stringFeatureInput.emplace_back(move(multiCharMultiValue));
        //MultiValueStorage<string> not support
        if(denseEnable == true){
            vector<string> denseInput;
            for(size_t i = 0; i < r; ++i)
                for(size_t j = 0; j < c; ++j)
                    denseInput.emplace_back(input[i][j]);
            unique_ptr<FeatureInput> multiCharDense(genDenseInput<MultiChar>(genMultiCharValues(denseInput), r, c));
            stringFeatureInput.emplace_back(move(multiCharDense));
            unique_ptr<FeatureInput> stringDense(genDenseInput<string>(denseInput, r, c));
            stringFeatureInput.emplace_back(move(stringDense));
        }
        return stringFeatureInput;
    }

    template<typename ResultT = float, typename FeatureT = SingleDenseFeatures>
    void checkSingleDenseFeatures(const std::vector<ResultT> &results) {
        auto typedFeatures = ASSERT_CAST_AND_RETURN(FeatureT, _features.get());
        EXPECT_EQ(typedFeatures->_featureValues.size(), results.size());
        EXPECT_THAT(typedFeatures->_featureValues, ElementsAreArray(results.data(), results.size()));
    }

    template <typename ResultT = float, typename FeatureT = SingleIntegerFeatures>
    void checkSingleIntegerFeatures(const std::vector<ResultT> &results) {
        auto typedFeatures = ASSERT_CAST_AND_RETURN(FeatureT, _features.get());
        EXPECT_EQ(typedFeatures->_featureValues.size(), results.size());
        EXPECT_THAT(typedFeatures->_featureValues,
                    ElementsAreArray(results.data(), results.size()));
    }

    void checkMultiSparseFeatures(const std::vector<string> &names,
                                  const std::vector<size_t> &offsets) {
        vector<ConstString> constNames;
        for (auto name : names) {
            constNames.emplace_back(ConstString(name));
        }
        checkMultiSparseFeatures(constNames, offsets);
    }

    void checkMultiSparseFeatures(const std::vector<ConstString> &names,
                                  std::vector<size_t> offsets) {
        auto typedFeatures = ASSERT_CAST_AND_RETURN(MultiSparseFeatures, _features.get());
        EXPECT_EQ(names.size(), typedFeatures->_featureNames.size());
        EXPECT_THAT(typedFeatures->_featureNames, UnorderedElementsAreArray(names.data(), names.size()));
        EXPECT_THAT(typedFeatures->_offsets, UnorderedElementsAreArray(offsets.data(), offsets.size()));
    }

    void checkSingleSparseFeatures(const std::vector<std::string>& values) {
        std::vector<ConstString> constNames;
        for (auto name : values) {
            constNames.emplace_back(ConstString(name));
        }
        auto typedFeatures = ASSERT_CAST_AND_RETURN(SingleSparseFeatures, _features.get());
        EXPECT_EQ(constNames.size(), typedFeatures->_featureNames.size());
        EXPECT_THAT(typedFeatures->_featureNames, UnorderedElementsAreArray(constNames.data(), constNames.size()));
    }

    void checkMultiSparseWeightingFeatures(const std::vector<string> &names,
            const std::vector<double> &values,
            const std::vector<size_t> &offsets)
    {
        MultiSparseWeightingFeatures *typedFeatures =
            ASSERT_CAST_AND_RETURN(MultiSparseWeightingFeatures, _features.get());
        EXPECT_EQ(names.size(), typedFeatures->_featureNames.size());
        for (size_t i = 0; i < names.size(); i++) {
            EXPECT_EQ(autil::ConstString(names[i]), typedFeatures->_featureNames[i]);
        }
        EXPECT_EQ(typedFeatures->_offsets, offsets);

        EXPECT_EQ(names.size(), typedFeatures->_featureValues.size());
        EXPECT_EQ(values.size(), typedFeatures->_featureValues.size());
        for (size_t i = 0; i < values.size(); i++) {
            EXPECT_DOUBLE_EQ(values[i], typedFeatures->_featureValues[i]);
        }
    }

protected:
    FeatureFunctionContext _context;
    std::unique_ptr<Features> _features;
    std::unique_ptr<Pool> _pool = std::make_unique<autil::mem_pool::Pool>();
    std::vector<std::function<void()>> _cleanups;
};

}

#include "fg_lite/feature/MatchFeatureFunction.h"
#include "fg_lite/feature/FeatureFormatter.h"

using namespace std;
using namespace autil;

namespace fg_lite {

class UserIterator {
public:
    UserIterator()
        : _row(0)
        , _users(nullptr)
    {}
    ~UserIterator() {
        if (!broadcast()) {
            delete []_users;
        }
    }
private:
    template<typename StringType, typename StorageType>
    bool constructUser(FeatureInput *input) {
        auto typedInput = dynamic_cast<FeatureInputTyped<StringType, StorageType>*>(input);
        if (!typedInput) {
            AUTIL_LOG(WARN, "user input type error %s", typeid(*input).name());
            return false;
        }
        size_t row = typedInput->row();
        for (size_t i = 0; i < row; i++) {
            if (typedInput->col(i) < 1) {
                continue;
            }
            const StringType str = typedInput->get(i, 0);
            ConstString userInfo(str.data(), str.size());
            if (!_users[i].parseUserInfo(userInfo) && 1 == row) {
                AUTIL_LOG(DEBUG, "user info[%s] is invalid", userInfo.c_str());
                return false;
            }
        }
        return true;
    }
public:
    bool construct(FeatureInput *input) {
        _row = input->row();
        if (broadcast()) {
            _users = &_user;
        } else {
            _users = new UserMatchInfo[_row];
        }
        if (IT_CSTRING == input->dataType()) { // online predict
            return constructUser<string, DenseStorage<string>>(input);
        } else if (IT_STRING == input->dataType()) { // offline train
            if (IST_SPARSE_MULTI_VALUE == input->storageType()) {
                return constructUser<MultiChar, MultiValueStorage<MultiChar>>(input);
            } else {
                return constructUser<MultiChar, DenseStorage<MultiChar>>(input);
            }
        } else {
            AUTIL_LOG(DEBUG, "match feature not support user input type %d",
                      int(input->dataType()));
            return false;
        }
    }
    bool broadcast() const {
        return _row <= 1;
    }
    const UserMatchInfo &get(size_t id) const {
        if (broadcast()) {
            return _user;
        } else {
            assert(id < _row);
            return _users[id];
        }
    }
private:
    size_t _row;
    UserMatchInfo _user;
    UserMatchInfo *_users;
    AUTIL_LOG_DECLARE();
};

AUTIL_LOG_SETUP(fg_lite, UserIterator);
AUTIL_LOG_SETUP(fg_lite, MatchFeatureFunction);

MatchFeatureFunction::MatchFeatureFunction(
        const MatchFunction *matcher,
        bool wildCardCategory,
        bool wildCardItem)
    : FeatureFunction(matcher->getFeatureName())
    , _matcher(matcher)
    , _wildCardCategory(wildCardCategory)
    , _wildCardItem(wildCardItem)
{
    assert(!(_wildCardCategory && _wildCardItem));
}

Features *MatchFeatureFunction::genFeatures(
        const vector<FeatureInput*> &inputs,
        FeatureFunctionContext *context) const
{
    //user item category
    if (!checkInput(inputs)) {
        AUTIL_LOG(ERROR, "input count invalid, expected[%d], actual[%d]",
                  (int)getInputCount(), (int)inputs.size());
        return nullptr;
    }

    FeatureInput *userInput = inputs[0];
    FeatureInput *itemInput = _wildCardItem ? nullptr : inputs[1];
    FeatureInput *categoryInput = _wildCardCategory ? nullptr :
                                  (_wildCardItem ? inputs[1] : inputs[2]);

    if (itemInput && itemInput->row() == 0) {
        AUTIL_LOG(ERROR, "match feature[%s] itemInput row must > 0",
                  getFeatureName().c_str());
        return nullptr;
    }

    if (categoryInput && categoryInput->row() == 0) {
        AUTIL_LOG(ERROR, "match feature[%s] categoryInput row must > 0",
                  getFeatureName().c_str());
        return nullptr;
    }

    size_t docCount = 1;
    if (itemInput) {
        docCount = itemInput->row();
    }
    if (categoryInput) {
        if (categoryInput->row() == 1 ||
            docCount == 1 ||
            categoryInput->row() == docCount)
        {
            docCount = max(categoryInput->row(), docCount);
        } else {
            AUTIL_LOG(ERROR, "match feature[%s] categoryInput row[%d] != 1 or docCount [%d]",
                      getFeatureName().c_str(), int(categoryInput->row()), int(docCount));
            return nullptr;
        }
    }

    size_t userRow = userInput->row();
    if (userRow > 1 && userRow != docCount) {
        AUTIL_LOG(ERROR, "match feature[%s] row for user not equal doc count or 1",
                  getFeatureName().c_str());
        return nullptr;
    }
    UserIterator userIterator;
    if (!userIterator.construct(userInput)) {
        return nullptr;
    }

    if (_matcher->isSparseFeature()) {
        if(_matcher->needWeighting()) {
            MultiSparseWeightingFeatures *features =
                new MultiSparseWeightingFeatures(docCount);
            genMatchFeatures<MultiSparseWeightingFeatures>(itemInput,
                    categoryInput, userIterator, features);
            return features;
        } else {
            MultiSparseFeatures *features = new MultiSparseFeatures(docCount);
            genMatchFeatures<MultiSparseFeatures>(itemInput,
                    categoryInput, userIterator, features);
            return features;
        }
    } else {
        MultiDenseFeatures *features = new MultiDenseFeatures(
                getFeatureName(), docCount);
        genMatchFeatures<MultiDenseFeatures>(itemInput, categoryInput, userIterator, features);
        return features;
    }
}

class PerDocValueIterator {
public:
    PerDocValueIterator(FeatureInput *input, size_t docId,
                        FeatureFormatter::FeatureBuffer &buffer)
        : _input(input)
        , _docId(docId)
        , _buffer(buffer)
    {
        if (_input && _input->row() == 1) {
            _docId = 0;
        }
    }
public:
    size_t size() const {
        return _input == nullptr ? 1 : _input->col(_docId);
    }
    ConstString get(size_t id) const {
        assert(id < size());
        if (_input == nullptr) {
            return ConstString(MATCH_WILDCARD_STRING.data(), MATCH_WILDCARD_STRING.size());
        } else {
            _buffer.clear();
            _input->toString(_docId, id, _buffer);
            return ConstString(_buffer.data(), _buffer.size());
        }
    }
private:
    FeatureInput *_input;
    size_t _docId;
    FeatureFormatter::FeatureBuffer &_buffer;
};

template <typename FeatureType>
void MatchFeatureFunction::genMatchFeatures(
        FeatureInput *itemInput,
        FeatureInput *categoryInput,
        UserIterator &userIterator,
        FeatureType *features) const
{
    size_t docCount = itemInput != nullptr ? itemInput->row() : categoryInput->row();

    FeatureFormatter::FeatureBuffer itemBuffer(cp_alloc(features->getPool()));
    FeatureFormatter::FeatureBuffer categoryBuffer(cp_alloc(features->getPool()));

    size_t lastCount = 0;

    for (size_t docId = 0; docId < docCount; docId++) {
        features->beginDocument();
        PerDocValueIterator itemValues(itemInput, docId, itemBuffer);
        PerDocValueIterator categoryValues(categoryInput, docId, categoryBuffer);

        const UserMatchInfo &matchInfo = userIterator.get(docId);
        for (size_t i = 0; i < categoryValues.size(); i++) {
            for (size_t j = 0; j < itemValues.size(); j++) {
                _matcher->matchOneFeature(categoryValues.get(i), itemValues.get(j),
                        matchInfo, features);
            }
        }

        if (FVT_MULTI_DENSE == features->getFeatureValueType()) {
            auto castFeatures = dynamic_cast<MultiDenseFeatures*>(features);
            if (castFeatures->valueSize() == lastCount) {
                castFeatures->addFeatureValue(0.0);
            }
            lastCount = castFeatures->valueSize();
        }
    }
}
}

#include "fg_lite/feature/OverLapFeatureFunction.h"

using namespace std;
using namespace autil;

namespace fg_lite {

Features* OverLapFeatureFunction::genNoCombo(FeatureInput *userInput, FeatureInput *itemInput) const {
    auto itemDataType = itemInput->dataType();
    auto itemStorageType = itemInput->storageType();
    auto userDataType = userInput->dataType();
    auto userStorageType = userInput->storageType();
    switch (itemDataType) {
        INPUT_DATA_TYPE_MACRO_HELPER(CASE_SWITCH_USER_DATATYPE2);
        DEFAULT(m1, featureName, itemDataType);
    }
    AUTIL_LOG(ERROR, "feature %s gen failed", getFeatureName().c_str());
    return nullptr;
}

}

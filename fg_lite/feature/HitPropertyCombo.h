#ifndef ISEARCH_FG_LITE_HITPROPERTY_COMBO_H
#define ISEARCH_FG_LITE_HITPROPERTY_COMBO_H

#include "autil/Log.h"
#include "fg_lite/feature/FeatureFunction.h"
#include "fg_lite/feature/Normalizer.h"
#include "fg_lite/feature/Combiner.h"
#include "autil/ConstString.h"
#include <unordered_map>

namespace fg_lite {

class HitPropertyCombo : public FeatureFunction
{
    HitPropertyCombo(const HitPropertyCombo&);
    HitPropertyCombo operator =(const HitPropertyCombo&);
public:
    Features *genFeatures(const std::vector<FeatureInput*> &inputs,
                          FeatureFunctionContext *context) const override;

    size_t getInputCount() const override {
        if (_isOptimized) {
            return 1;
        }
        return 2;
    }

private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_HITPROPERTY_COMBO_H

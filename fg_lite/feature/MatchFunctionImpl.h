#ifndef ISEARCH_FG_LITE_MATCHFUNCTIONIMPL_H
#define ISEARCH_FG_LITE_MATCHFUNCTIONIMPL_H

#include "autil/Log.h"
#include "fg_lite/feature/MatchFunction.h"

namespace fg_lite {

class HitMatch : public MatchFunction
{
public:
    HitMatch(const std::string &featNamePrefix,
             bool needDiscrete,
             bool needWeighting);
    virtual ~HitMatch() = default;
private:
    HitMatch(const HitMatch &);
    HitMatch& operator=(const HitMatch &);
public:
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiDenseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseWeightingFeatures *features) const override;

private:
    static const char HIT_SEPARATOR;
private:
    AUTIL_LOG_DECLARE();
};

class MultiHitMatch : public MatchFunction
{
public:
    MultiHitMatch(const std::string &featNamePrefix, bool needDiscrete,
                  bool showCategory, bool showItem, bool needWeighting);
    virtual ~MultiHitMatch();
private:
    MultiHitMatch(const MultiHitMatch &);
    MultiHitMatch& operator=(const MultiHitMatch &);
public:
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiDenseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseWeightingFeatures *features) const override;
private:
    static const char HIT_SEPARATOR;
private:
    AUTIL_LOG_DECLARE();
};

class CrossMatch : public MatchFunction {
public:
    CrossMatch(const std::string &featNamePrefix);
    virtual ~CrossMatch();
private:
    CrossMatch(const CrossMatch &);
    CrossMatch& operator=(const CrossMatch &);
public:
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiDenseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseWeightingFeatures *features) const override;
private:
    static const char CROSS_SEPARATOR;
private:
    AUTIL_LOG_DECLARE();
};

class CosMatch : public MatchFunction {
public:
    CosMatch(const std::string &featNamePrefix, bool needDiscrete);
    virtual ~CosMatch();
private:
    CosMatch(const CosMatch &);
    CosMatch& operator=(const CosMatch &);
public:
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiDenseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseFeatures *features) const override;
    void matchOneFeature(const autil::ConstString &category,
                         const autil::ConstString &itemValue,
                         const UserMatchInfo &userMatchInfo,
                         MultiSparseWeightingFeatures *features) const override;
private:
    bool parseItemValueForCos(const autil::ConstString &itemValue,
                              std::vector<autil::ConstString> &keys,
                              std::vector<int64_t> &values) const;
    int64_t calculateFeatureValue(const std::vector<int64_t> &values1,
                                  const std::vector<int64_t> &values2,
                                  int32_t matchedCount) const;
private:
    static const std::string SEPARATOR_BETWEEN_KV;
    static const std::string SEPARATOR_KV;
    static const char COS_SEPARATOR;
private:
    AUTIL_LOG_DECLARE();
};

}

#endif //ISEARCH_FG_LITE_MATCHFUNCTIONIMPL_H

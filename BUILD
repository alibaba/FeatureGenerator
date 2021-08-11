load("//:defs.bzl", "gen_cpp_code")

type_list = ['IT_INT32', 'IT_UINT32',
             'IT_INT64', 'IT_UINT64',
             'IT_STRING', 'IT_CSTRING', 'IT_FLOAT', 'IT_DOUBLE',]
type_list_map = ['IT_INT32', 'IT_INT64',
                 'IT_STRING', 'IT_CSTRING', 'IT_FLOAT', 'IT_DOUBLE',]
storage_type = ['DenseStorage', 'MultiValueStorage']

template_header = """
#include "fg_lite/feature/{}"

namespace fg_lite {{
"""

lookup_feature_template = """
LOOKUP_ARRAY_IMPL(typename InputType2Type<{0}>::Type, typename InputType2Type<{1}>::Type, {2}, {3}, typename InputType2Type<{4}>::Type);
"""
template_tail = """
}
"""

gen_cpp_code("lookup_feature_gen_code", [type_list, type_list_map, storage_type, storage_type, type_list_map],
             template_header.format("LookupFeatureFunctionArrayImpl.h"), lookup_feature_template, template_tail)


type_list_full = [
    'IT_INT8', 'IT_UINT8',
    'IT_INT16', 'IT_UINT16',
    'IT_INT32', 'IT_UINT32',
    'IT_INT64', 'IT_UINT64',
    'IT_FLOAT', 'IT_DOUBLE',
    'IT_STRING', 'IT_CSTRING',
]
sparse_type = ['IST_DENSE', 'IST_SPARSE_MULTI_VALUE']

overlap_feature_2_template = """
OVERLAP_IMPL_2({0}, {1}, {2}, {3});
"""
gen_cpp_code("overlap_feature_2_gen_code",
             [type_list_full, sparse_type, type_list_full, sparse_type],
             template_header.format("OverLapFeatureFunctionImpl.h"),
             overlap_feature_2_template,
             template_tail)

cc_library(
    name = "config",
    srcs = ["fg_lite/feature/FeatureConfig.cpp"],
    hdrs = [
        "fg_lite/feature/FeatureConfig.h",
        "fg_lite/feature/LookupFeatureDataType.h",
    ],
    deps = [
        "//autil:json",
        "//autil:log",
        "//autil:string_base",
    ],
    include_prefix = "fg_lite",
    strip_include_prefix = "fg_lite",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "fg_lite_base",
    srcs = [
        "fg_lite/feature/FeatureFunction.cpp",
        "fg_lite/feature/LookupFeatureEncoder.cpp",
        "fg_lite/feature/LookupFeatureSparseEncoder.cpp",
        "fg_lite/feature/Normalizer.cpp",
    ],
    hdrs = [
        "fg_lite/feature/FeatureFormatter.h",
        "fg_lite/feature/Feature.h",
        "fg_lite/feature/FeatureFunction.h",
        "fg_lite/feature/FeatureInput.h",
        "fg_lite/feature/LookupFeatureEncoder.h",
        "fg_lite/feature/LookupFeatureSparseEncoder.h",
        "fg_lite/feature/LookupFeatureBTreeEncoder.h",
        "fg_lite/feature/FillMapDataBTreeVisitor.h",
        "fg_lite/feature/StoreMapDataBTreeVisitor.h",
        "fg_lite/feature/BTree.h",
        "fg_lite/feature/Normalizer.h",
        "fg_lite/feature/Combiner.h",
        "fg_lite/feature/Collector.h",
        "fg_lite/feature/LookupFeatureDataType.h",
    ],
    deps = [
        ":config",
        "//autil:murmur_hash",
        "//autil:log",
        "//autil:type_base",
        "//autil:type_helper",
        "//autil:string_base",
        "//autil:mem_pool_base",
        "//autil:bitmap",
        "//autil:time",
    ],
    include_prefix = "fg_lite",
    strip_include_prefix = "fg_lite",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "lookup_feature_gen",
    srcs =  [
        ":lookup_feature_gen_code",
    ],
    hdrs = glob([
        "fg_lite/feature/*.h",
    ]),
    deps = [
        ":config",
        ":fg_lite_base",
        "//autil:log",
    ],
    copts = ["-g0"],
    include_prefix = "fg_lite",
    strip_include_prefix = "fg_lite",
)

cc_library(
    name = "overlap_feature_2_gen",
    srcs =  [
        ":overlap_feature_2_gen_code",
        "fg_lite/feature/OverLapFeatureEvaluator.cpp",
    ],
    hdrs = glob([
        "fg_lite/feature/*.h",
    ]),
    deps = [
        ":config",
        ":fg_lite_base",
        "//autil:log",
    ],
    copts = ["-g0"],
    include_prefix = "fg_lite",
    strip_include_prefix = "fg_lite",
)

cc_library(
    name = "fg_lite",
    srcs = [
        "fg_lite/feature/ComboFeatureFunction.cpp",
        "fg_lite/feature/FeatureFunctionCreator.cpp",
        "fg_lite/feature/IdFeatureFunction.cpp",
        "fg_lite/feature/KgbMatchSemanticFeatureFunction.cpp",
        "fg_lite/feature/LookupFeatureFunction.cpp",
        "fg_lite/feature/LookupFeatureFunctionArray.cpp",
        "fg_lite/feature/LookupFeatureFunctionBTree.cpp",
        "fg_lite/feature/LookupFeatureFunctionV2.cpp",
        "fg_lite/feature/LookupFeatureFunctionV3.cpp",
        "fg_lite/feature/MatchFeatureFunction.cpp",
        "fg_lite/feature/OverLapFeatureFunction.cpp",
        "fg_lite/feature/OverLapFeatureFunction1.cpp",
        "fg_lite/feature/PreclickUrbWordFeatureFunction.cpp",
        "fg_lite/feature/RawFeatureFunction.cpp",
        "fg_lite/feature/UserMatchInfo.cpp",
        "fg_lite/feature/MatchFunction.cpp",
        "fg_lite/feature/MatchFunctionImpl.cpp",
        "fg_lite/feature/Base64.cpp",
        "fg_lite/feature/LookupFeatureBTreeClient.cpp",
    ],
    hdrs = [
        "fg_lite/feature/ComboFeatureFunction.h",
        "fg_lite/feature/FeatureFunctionCreator.h",
        "fg_lite/feature/IdFeatureFunction.h",
        "fg_lite/feature/KgbMatchSemanticFeatureFunction.h",
        "fg_lite/feature/LookupFeatureFunction.h",
        "fg_lite/feature/LookupFeatureFunctionArray.h",
        "fg_lite/feature/LookupFeatureFunctionArrayImpl.h",
        "fg_lite/feature/LookupFeatureFunctionBTree.h",
        "fg_lite/feature/LookupFeatureFunctionV2.h",
        "fg_lite/feature/LookupFeatureFunctionV3.h",
        "fg_lite/feature/MatchFeatureFunction.h",
        "fg_lite/feature/OverLapFeatureFunction.h",
        "fg_lite/feature/OverLapFeatureFunctionImpl.h",
        "fg_lite/feature/PreclickUrbWordFeatureFunction.h",
        "fg_lite/feature/RawFeatureFunction.h",
        "fg_lite/feature/MatchFunction.h",
        "fg_lite/feature/UserMatchInfo.h",
    ],
    deps = [
        ":config",
        ":overlap_feature_2_gen",
        ":lookup_feature_gen",
        ":fg_lite_base",
        "//autil:string_base",
        "//autil:mem_pool_base",
        "//autil:log",
    ],
    visibility = ["//visibility:public"],
    include_prefix = "fg_lite",
    strip_include_prefix = "fg_lite",
    copts = ["-march=native", "-mavx512f", "-mavx512vl", "-mavx512bw"],
)

cc_library(
    name = "fg_lite_test_helper",
    hdrs = glob([
        "fg_lite/feature/test/*.h",
    ]),
    include_prefix = "fg_lite",
    strip_include_prefix = "fg_lite",
)

cc_test(
    name = "fg_lite_test",
    srcs = glob([
        "fg_lite/feature/test/*.cpp",
    ]),
    data = glob(["testdata/**"]),
    deps = [
        ":fg_lite",
        ":fg_lite_test_helper",
        "@com_google_googletest//:gtest",
    ],
    copts = ["-march=native", "-mavx512f", "-mavx512vl", "-mavx512bw"],
)

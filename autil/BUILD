
cc_library(
    name = "time",
    srcs = ["TimeUtility.cpp"],
    hdrs = ["TimeUtility.h"],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "bitmap",
    srcs = ["bitmap.cpp"],
    hdrs = ["bitmap.h"],
    deps = [
        ":log",
        ":time",
        ":mem_pool_base",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "json_base",
    srcs = [
        "legacy/json.cpp",
        "legacy/astream.cpp",
        "legacy/string_utf8.cpp",
        "legacy/string_conversion.cpp",
        "legacy/json_string_enhanced.cpp",
    ],
    hdrs = [
        "legacy/any.h",
        "legacy/jsonizable_exception.h",
        "legacy/json.h",
        "legacy/string_tools.h",
        "legacy/astream.h",
    ],
    deps = [
        ":string_base",
        ":exception",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "fast_json",
    srcs = [
        "legacy/fast_jsonizable.cpp",
    ],
    hdrs = [
        "legacy/fast_jsonizable.h",
        "legacy/fast_jsonizable_dec.h",
    ],
    deps = [
        "@rapidjson//:rapidjson",
        ":json_base",
        ":type_base",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "json",
    hdrs = [
        "legacy/legacy_jsonizable.h",
        "legacy/legacy_jsonizable_dec.h",
        "legacy/jsonizable.h",
    ],
    deps = [
        "@rapidjson//:rapidjson",
        ":type_base",
        ":type_helper",
        ":json_base",
        ":fast_json",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "log",
    hdrs = ["Log.h"],
    deps = [
        ":time",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "exception",
    srcs = [
        "legacy/exception.cpp",
    ],
    hdrs = [
        "legacy/exception.h",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "common_macros",
    hdrs = ["CommonMacros.h"],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

cc_library(
    name = "lock",
    srcs = [
        "Lock.cpp",
    ],
    hdrs = [
        "Lock.h",
    ],
    deps = [
        ":common_macros",
        ":time",
        ":log",
    ],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "mem_pool_base",
    srcs = [
        "mem_pool/Pool.cpp",
        "mem_pool/RecyclePool.cpp",
        "mem_pool/SimpleAllocatePolicy.cpp",
        "mem_pool/SimpleAllocator.cpp",
    ],
    hdrs = [
        "mem_pool/AllocatePolicy.h",
        "mem_pool/ChunkAllocatorBase.h",
        "mem_pool/MemoryChunk.h",
        "mem_pool/Pool.h",
        "mem_pool/PoolBase.h",
        "mem_pool/RecyclePool.h",
        "mem_pool/SimpleAllocatePolicy.h",
        "mem_pool/SimpleAllocator.h",
        "mem_pool/SubPoolAllocator.h",
        "mem_pool/pool_allocator.h",
    ],
    deps = [
        ":log",
        ":lock",
    ],
    visibility = ["//visibility:public"],
    alwayslink = True,
)

#小目标目前按依赖分类，下次重构ConstStringUtil CountedMulti*并按功能分类
cc_library(
    name = "string_base",
    srcs = [
        "StringUtil.cpp",
        "ShortString.cpp",
        "ConstString.cpp",
        "StringTokenizer.cpp",
        "FormatInt.cpp",
    ],
    hdrs = [
        "StringUtil.h",
        "ShortString.h",
        "ConstString.h",
        "StringTokenizer.h",
        "FormatInt.h",
    ],
    deps = [
        ":log",
    ],
    private_deps = [
        ":lock",
    ],
    alwayslink = True,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "type_base",
    srcs = [
        "LongHashValue.cpp",
        "MultiValueType.cpp",
        "CountedMultiValueType.cpp",
    ],
    hdrs = [
        "LongHashValue.h",
        "MultiValueFormatter.h",
        "MultiValueType.h",
        "CountedMultiValueType.h",
    ],
    deps = [
        ":common_macros",
        ":string_base",
    ],
    alwayslink = True,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "type_helper",
    srcs = [
        "MultiValueCreator.cpp",
    ],
    hdrs = [
        "MultiValueCreator.h",
        "MultiValueWriter.h",
    ],
    deps = [
        ":mem_pool_base",
        ":type_base",
    ],
    private_deps = [
        ":string_base",
        ":exception",
    ],
    alwayslink = True,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "murmur_hash",
    srcs = [
        "MurmurHash.cpp",
    ],
    hdrs = [
        "MurmurHash.h",
    ],
    visibility = ["//visibility:public"],
)

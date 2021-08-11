load('@bazel_tools//tools/build_defs/repo:git.bzl', "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")

def clean_dep(dep):
    return str(Label(dep))

def alifg_workspace():
    git_repository(
        name = "com_google_googletest",
        remote = "git@gitlab.alibaba-inc.com:search_external/gtest.git",
        commit = "1e0cc96ae27e2934c0604be7ba25ac9f27d1f988",
        shallow_since = "1594384953 +0800",
    )

    http_archive(
        name = "rapidjson",
        urls = ["http://search-ad.oss-cn-hangzhou-zmf.aliyuncs.com/aios%2Fthird_party_archives%2Frapidjson-v1.0.2.zip"],
        build_file = clean_dep("//third_party:rapidjson.BUILD"),
        strip_prefix = "rapidjson-1.0.2",
        sha256 = "69e876bd07670189214f44475add2e0afb8374e5798270208488c973a95f501d"
    )

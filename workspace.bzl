load('@bazel_tools//tools/build_defs/repo:git.bzl', "new_git_repository")

def clean_dep(dep):
    return str(Label(dep))

def alifg_workspace():
    new_git_repository(
        name = "gmock_archive",
        remote = "https://github.com/google/googletest.git",
        tag = "release-1.8.0",
        build_file = str(Label("//third_party:gmock.BUILD")),
    )

    new_git_repository(
        name = "rapidjson",
        remote = "https://github.com/Tencent/rapidjson.git",
        tag = "v1.0.2",
        build_file = str(Label("//third_party:rapidjson.BUILD")),
    )

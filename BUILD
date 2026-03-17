load("@rules_cc//cc:defs.bzl", "cc_library", "cc_binary")

cc_library(
    name = "cpu_lib",
    srcs = glob(["src/*.cpp"]),
    hdrs = glob(["include/*.hpp"]),
    includes = ["include"],
    copts = ["-std=c++17"],
)

cc_binary(
    name = "cpu_monitor",
    deps = [":cpu_lib"],
)
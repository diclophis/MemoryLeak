#!/bin/sh

export CCC_ANALYZER_CPLUSPLUS=1
~/Desktop/checker-249/scan-build -v -v -v \
--use-c++=/Users/jon/llvm/Debug+Asserts/bin/clang++ \
-V \
-analyze-headers \
-analyzer-check-dead-stores \
-analyzer-check-idempotent-operations \
-analyzer-check-llvm-conventions \
-analyzer-check-objc-mem \
-analyzer-check-objc-methodsigs \
-analyzer-check-objc-missing-dealloc \
-analyzer-check-objc-unused-ivars \
-analyzer-check-security-syntactic \
xcodebuild -configuration Debug

#--experimental-checks \
#make -j4

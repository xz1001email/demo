#!/bin/bash

echo "para num = $#"
for arg in "$*"
do
    echo "option: $arg"
done

cmake -DLWS_WITH_SSL=OFF -DCMAKE_TOOLCHAIN_FILE=android.toolchain.cmake -DCMAKE_BUILD_TYPE=Debug $arg -DANDROID_NATIVE_API_LEVEL=android-21 -DANDROID_ABI="arm64-v8a" -DANDROID_TOOLCHAIN_NAME=aarch64-linux-android-4.9 ../

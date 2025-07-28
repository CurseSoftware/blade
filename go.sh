#!/usr/bin/env bash

# Utility functions
pexec() { >&2 printf exec; >&2 printf ' %q' "$@"; >&2 printf '\n'; exec "${@:?pexec: missing command}"; }

go-Build-All() {
    cmake -B build .
    cmake --build build
}

go-Build-Debug() {
    cmake -B build_debug -DCMAKE_BUILD_TYPE=Debug .
    cmake --build build_debug
}

go-Compile-Debug() {
    cmake --build build_debug
}

go-Build-Release() {
    cmake -B build_release .
    cmake --build build_release
}

go-Compile-Debug() {
    cmake --build build_release
}

go-Compile-Shaders() {
    glslc resources/builtin/shaders/simple.vert -o simple.vert.spv
    
    glslc resources/builtin/shaders/simple.frag -o simple.frag.spv
}

go-Run-Example() {
    local example_name="$1"
    local build_type="${2:-Debug}"
    local build_dir="build_${build_type,,}"

    echo "Build dir: ${build_Dir}"
    
    if [ -z "$example_name" ]; then
        echo "Usage $0 Run <example_name> [Debug|Release]"
        echo "Available examples:"
        find "${build_dir}" -type f -executable -name "*" | grep -E "(apps|examples)" | sort
        return 1
    fi

    local exe_path=""

    if [ -f "${build_dir}/apps/$example_name/$example_name" ]; then
        exe_path="${build_dir}/apps/$example_name/$example_name"
    elif [ -f "${build_dir}/apps/$example_name/${example_name}_app" ]; then
        exe_path="${build_dir}/apps/$example_name/${example_name}_app"
    elif [ -f "${build_dir}/apps/$example_name/main" ]; then
        exe_path="${build_dir}/apps/$example_name/main"
    else
        # Try to find it anywhere in the build directory
        exe_path=$(find "$build_dir" -type f -executable -name "*$example_name*" | head -1)
    fi
      
    if [ -z "$exe_path" ] || [ ! -f "$exe_path" ]; then
        echo "Error: Could not find executable for example '$example_name'"
        echo "Available executables:"
        find "${build_dir}" -type f -executable | grep -v CMake | sort
        return 1
    fi

    echo "Running: $exe_path"
    "$exe_path"
}

go-Clean() {
    echo "Cleaning all build directories..."
    rm -rf build_debug/*
    rm -rf build_release/*
}

"go-$@"

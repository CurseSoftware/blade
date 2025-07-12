#!/usr/bin/env bash

# Utility functions
pexec() { >&2 printf exec; >&2 printf ' %q' "$@"; >&2 printf '\n'; exec "${@:?pexec: missing command}"; }

go-Build-All() {
    cmake -B build .
    cmake --build build
}

go-Run-Example() {
    local example_name="$1"
    if [ -z "$example_name" ]; then
        echo "Usage $0 Run <example_name>"
        echo "Available examples:"
        find build -type f -executable -name "*" | grep -E "(apps|examples)" | sort
        return 1
    fi

    local exe_path=""

    if [ -f "build/apps/$example_name/$example_name" ]; then
        exe_path="build/apps/$example_name/$example_name"
    elif [ -f "build/apps/$example_name/${example_name}_app" ]; then
        exe_path="build/apps/$example_name/${example_name}_app"
    elif [ -f "build/apps/$example_name/main" ]; then
        exe_path="build/apps/$example_name/main"
    else
        # Try to find it anywhere in the build directory
        exe_path=$(find build -type f -executable -name "*$example_name*" | head -1)
    fi
      
    if [ -z "$exe_path" ] || [ ! -f "$exe_path" ]; then
        echo "Error: Could not find executable for example '$example_name'"
        echo "Available executables:"
        find build -type f -executable | grep -v CMake | sort
        return 1
    fi

    echo "Running: $exe_path"
    "$exe_path"
}

"go-$@"

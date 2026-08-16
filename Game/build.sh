#!/usr/bin/env bash

set -euo pipefail

# macOS build script
#
# Builds the Game dynamic library as C.
#
# Usage:
#   ./build.sh [arm64|x86_64|universal] [Debug|Release]
#
# Aliases:
#   Developer -> Debug
#   Product   -> Release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
solution_dir="$(cd "$script_dir/.." && pwd)"

platform="${1:-$(uname -m)}"
configuration_arg="${2:-Debug}"

case "$platform" in
    arm64|x86_64|universal)
        ;;
    *)
        echo "Invalid platform architecture. Supported platforms: arm64, x86_64, universal" >&2
        exit 1
        ;;
esac

case "$configuration_arg" in
    Debug)
        configuration="Debug"
        build_mode_name="Developer"
        ;;
    Release)
        configuration="Release"
        build_mode_name="Product"
        ;;
    *)
        echo "Invalid configuration. Supported configurations: Debug, Release" >&2
        exit 1
        ;;
esac

source_names=(
    game
    intrinsics
    utility
    memory
    player
    world
    tilemap
    graphics
    audio
    filesystem
    math
)

if xcrun clang -std=c23 -x c -fsyntax-only /dev/null >/dev/null 2>&1; then
    c_standard="c23"
else
    c_standard="c2x"
fi

build_arch() {
    local target_platform="$1"
    local build_configuration_folder="$solution_dir/build/macOS/$target_platform/$configuration"
    local intermediates_configuration_folder="$solution_dir/build/macOS/intermediates/Game/$target_platform/$configuration"

    mkdir -p "$build_configuration_folder"
    mkdir -p "$intermediates_configuration_folder"

    local sources=()
    local objects=()
    local name
    for name in "${source_names[@]}"; do
        local source_file="$script_dir/$name.c"
        local object_file="$intermediates_configuration_folder/$name.o"

        if [[ ! -f "$source_file" ]]; then
            echo "Missing source: $source_file" >&2
            exit 1
        fi

        sources+=("$source_file")
        objects+=("$object_file")
    done

    echo "============="
    echo "Building Game as C for macOS $target_platform $configuration ($build_mode_name build)"
    echo "C standard: $c_standard"
    echo "============="

    local common_compiler_flags=(
        -c
        -x c
        -std="$c_standard"
        -arch "$target_platform"
        -fPIC
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
        -Wno-unused-function
        -Wno-unused-variable
        -Wno-missing-field-initializers
        -Wno-gnu-anonymous-struct
        -Wno-nested-anon-types
        -DGAME_EXPORTS=1
        -DPLATFORM_MACOS=1
        -DCOMPILER_LLVM=1
        -mmacosx-version-min=15.0
    )

    local debug_compiler_flags=(
        -g
        -O0
        -fno-omit-frame-pointer
        -DDEBUG=1
        -D_DEBUG=1
    )

    local release_compiler_flags=(
        -O2
        -DNDEBUG=1
    )

    local common_linker_flags=(
        -dynamiclib
        -arch "$target_platform"
        -Wl,-install_name,@rpath/Game.dylib
        -mmacosx-version-min=15.0
        -o "$build_configuration_folder/Game.dylib"
    )

    local debug_linker_flags=(
        -g
    )

    local release_linker_flags=(
        -Wl,-dead_strip
    )

    local compiler_flags=()
    local linker_flags=()
    if [[ "$configuration" == "Debug" ]]; then
        compiler_flags=("${common_compiler_flags[@]}" "${debug_compiler_flags[@]}")
        linker_flags=("${common_linker_flags[@]}" "${debug_linker_flags[@]}")
    else
        compiler_flags=("${common_compiler_flags[@]}" "${release_compiler_flags[@]}")
        linker_flags=("${common_linker_flags[@]}" "${release_linker_flags[@]}")
    fi

    local index
    for index in "${!sources[@]}"; do
        local source_file="${sources[$index]}"
        local object_file="${objects[$index]}"

        echo "Compiling $(basename "$source_file")"
        xcrun clang "${compiler_flags[@]}" "$source_file" -o "$object_file"
    done

    echo "Linking Game.dylib"
    xcrun clang "${linker_flags[@]}" "${objects[@]}"

    echo "Built $build_configuration_folder/Game.dylib"
}

if [[ "$platform" == "universal" ]]; then
    build_arch arm64
    build_arch x86_64

    universal_build_folder="$solution_dir/build/macOS/universal/$configuration"
    mkdir -p "$universal_build_folder"

    echo "============="
    echo "Creating universal Game.dylib"
    echo "============="
    xcrun lipo -create \
        "$solution_dir/build/macOS/arm64/$configuration/Game.dylib" \
        "$solution_dir/build/macOS/x86_64/$configuration/Game.dylib" \
        -output "$universal_build_folder/Game.dylib"
    xcrun lipo -info "$universal_build_folder/Game.dylib"
    echo "Built $universal_build_folder/Game.dylib"
else
    build_arch "$platform"
fi

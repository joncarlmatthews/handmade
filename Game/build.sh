#!/usr/bin/env bash

set -euo pipefail

# Builds the Game dynamic library as C.
#
# Usage:
#   ./build.sh [arm64|x86_64] [Debug|Release]
#
# Aliases:
#   Developer -> Debug
#   Product   -> Release

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
solution_dir="$(cd "$script_dir/.." && pwd)"

platform="${1:-$(uname -m)}"
configuration_arg="${2:-Debug}"

case "$platform" in
    arm64|x86_64)
        ;;
    *)
        echo "Invalid platform architecture. Supported platforms: arm64, x86_64" >&2
        exit 1
        ;;
esac

case "$configuration_arg" in
    Debug|Developer|developer)
        configuration="Debug"
        build_mode_name="Developer"
        ;;
    Release|Product|product)
        configuration="Release"
        build_mode_name="Product"
        ;;
    *)
        echo "Invalid configuration. Supported configurations: Debug, Release, Developer, Product" >&2
        exit 1
        ;;
esac

build_configuration_folder="$solution_dir/build/macOS/$platform/$configuration"
intermediates_configuration_folder="$solution_dir/build/macOS/intermediates/Game/$platform/$configuration"

mkdir -p "$build_configuration_folder"
mkdir -p "$intermediates_configuration_folder"

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

sources=()
objects=()
for name in "${source_names[@]}"; do
    source_file="$script_dir/$name.c"
    object_file="$intermediates_configuration_folder/$name.o"

    if [[ ! -f "$source_file" ]]; then
        echo "Missing source: $source_file" >&2
        exit 1
    fi

    sources+=("$source_file")
    objects+=("$object_file")
done

if xcrun clang -std=c23 -x c -fsyntax-only /dev/null >/dev/null 2>&1; then
    c_standard="c23"
else
    c_standard="c2x"
fi

echo "============="
echo "Building Game as C for macOS $platform $configuration ($build_mode_name build)"
echo "C standard: $c_standard"
echo "============="

common_compiler_flags=(
    -c
    -x c
    -std="$c_standard"
    -arch "$platform"
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

debug_compiler_flags=(
    -g
    -O0
    -fno-omit-frame-pointer
    -DDEBUG=1
    -D_DEBUG=1
)

release_compiler_flags=(
    -O2
    -DNDEBUG=1
)

common_linker_flags=(
    -dynamiclib
    -arch "$platform"
    -Wl,-install_name,@rpath/Game.dylib
    -mmacosx-version-min=15.0
    -o "$build_configuration_folder/Game.dylib"
)

debug_linker_flags=(
    -g
)

release_linker_flags=(
    -Wl,-dead_strip
)

if [[ "$configuration" == "Debug" ]]; then
    compiler_flags=("${common_compiler_flags[@]}" "${debug_compiler_flags[@]}")
    linker_flags=("${common_linker_flags[@]}" "${debug_linker_flags[@]}")
else
    compiler_flags=("${common_compiler_flags[@]}" "${release_compiler_flags[@]}")
    linker_flags=("${common_linker_flags[@]}" "${release_linker_flags[@]}")
fi

for index in "${!sources[@]}"; do
    source_file="${sources[$index]}"
    object_file="${objects[$index]}"

    echo "Compiling $(basename "$source_file")"
    xcrun clang "${compiler_flags[@]}" "$source_file" -o "$object_file"
done

echo "Linking Game.dylib"
xcrun clang "${linker_flags[@]}" "${objects[@]}"

echo "Built $build_configuration_folder/Game.dylib"

#!/usr/bin/env bash

set -euo pipefail

# Builds the Game dynamic library as C. This is intentionally separate from the
# main CMake/CLion build so it can be used as a C23 refactor smoke test.
#
# Usage:
#   ./build.sh [arm64|x86_64]

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
solution_dir="$(cd "$script_dir/.." && pwd)"

configuration="Debug"
platform="${1:-$(uname -m)}"

case "$platform" in
    arm64|x86_64)
        ;;
    *)
        echo "Invalid platform architecture. Supported platforms: arm64, x86_64" >&2
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
echo "Building Game as C for macOS $platform $configuration"
echo "C standard: $c_standard"
echo "============="

compiler_flags=(
    -c
    -x c
    -std="$c_standard"
    -arch "$platform"
    -g
    -O0
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
    -DDEBUG=1
    -D_DEBUG=1
    -DGAME_EXPORTS=1
    -DPLATFORM_MACOS=1
    -DCOMPILER_LLVM=1
    -mmacosx-version-min=15.0
)

linker_flags=(
    -dynamiclib
    -arch "$platform"
    -g
    -Wl,-install_name,@rpath/Game.dylib
    -mmacosx-version-min=15.0
    -o "$build_configuration_folder/Game.dylib"
)

for index in "${!sources[@]}"; do
    source_file="${sources[$index]}"
    object_file="${objects[$index]}"

    echo "Compiling $(basename "$source_file")"
    xcrun clang "${compiler_flags[@]}" "$source_file" -o "$object_file"
done

echo "Linking Game.dylib"
xcrun clang "${linker_flags[@]}" "${objects[@]}"

echo "Built $build_configuration_folder/Game.dylib"

add_library(Game SHARED
        game.cpp
        intrinsics.cpp
        utility.cpp
        memory.cpp
        player.cpp
        world.cpp
        tilemap.cpp
        graphics.cpp
        audio.cpp
        filesystem.cpp
        math.cpp
)

target_compile_definitions(Game PRIVATE
        PLATFORM_MACOS=1
        COMPILER_LLVM=1
        $<$<CONFIG:Debug>:DEBUG=1>
)

target_compile_options(Game PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
        -Wno-unused-function
        -Wno-unused-variable
        -Wno-missing-field-initializers
)

set_target_properties(Game PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS ON
        PREFIX ""
        OUTPUT_NAME "Game"
        MACOSX_RPATH ON
        INSTALL_NAME_DIR "@rpath"
)

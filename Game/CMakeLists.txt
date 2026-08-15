add_library(Game SHARED
        game.c
        intrinsics.c
        utility.c
        memory.c
        player.c
        world.c
        tilemap.c
        graphics.c
        audio.c
        filesystem.c
        math.c
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
        C_STANDARD 23
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS ON
        PREFIX ""
        OUTPUT_NAME "Game"
        MACOSX_RPATH ON
        INSTALL_NAME_DIR "@rpath"
)

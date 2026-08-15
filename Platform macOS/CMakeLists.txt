set(HANDMADE_MACOS_SOURCES
        main.m
        AppDelegate.mm
        mac_handmade.mm
        AppDelegate.h
        mac_handmade.h
)

add_executable(HandmadeMac MACOSX_BUNDLE
        ${HANDMADE_MACOS_SOURCES}
)

target_include_directories(HandmadeMac PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_compile_definitions(HandmadeMac PRIVATE
        PLATFORM_MACOS=1
        COMPILER_LLVM=1
        $<$<CONFIG:Debug>:DEBUG=1>
)

target_compile_options(HandmadeMac PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter
        -Wno-unused-function
)

target_link_libraries(HandmadeMac PRIVATE
        Game
        "-framework Cocoa"
        "-framework CoreGraphics"
)

set_target_properties(HandmadeMac PROPERTIES
        C_STANDARD 17
        C_STANDARD_REQUIRED ON
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        OBJCXX_STANDARD 17
        OBJCXX_STANDARD_REQUIRED ON
        MACOSX_BUNDLE_BUNDLE_NAME "Handmade Hero"
        MACOSX_BUNDLE_GUI_IDENTIFIER "jonmatthews.Platform-macOS"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
        MACOSX_BUNDLE_BUNDLE_VERSION "1"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
        BUILD_RPATH "@executable_path/../Frameworks"
)

add_custom_command(TARGET HandmadeMac POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory
                "$<TARGET_BUNDLE_DIR:HandmadeMac>/Contents/Frameworks"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_FILE:Game>"
                "$<TARGET_BUNDLE_DIR:HandmadeMac>/Contents/Frameworks/$<TARGET_FILE_NAME:Game>"
        COMMENT "Copying Game dynamic library into HandmadeMac.app"
)

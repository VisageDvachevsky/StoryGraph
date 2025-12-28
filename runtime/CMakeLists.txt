# NovelMind Runtime
# Console-based runtime for playing visual novels

# Create the console runtime executable
add_executable(novelmind_runtime
    src/main.cpp
)

target_link_libraries(novelmind_runtime
    PRIVATE
        engine_core
        novelmind_compiler_options
)

# Set output name
set_target_properties(novelmind_runtime PROPERTIES
    OUTPUT_NAME "novelmind_runtime"
)

# Create the game launcher executable
# This is the main entry point for end-users to play games
add_executable(game_launcher
    src/game_launcher_main.cpp
)

target_link_libraries(game_launcher
    PRIVATE
        engine_core
        novelmind_compiler_options
)

# Set output name and properties
set_target_properties(game_launcher PROPERTIES
    OUTPUT_NAME "game_launcher"
)

# On Windows, we might want to use WIN32 to hide the console
# For now, we keep the console for debugging purposes
# To hide the console on Windows, uncomment the following:
# if(WIN32)
#     set_target_properties(game_launcher PROPERTIES
#         WIN32_EXECUTABLE TRUE
#     )
# endif()

# Installation
install(TARGETS novelmind_runtime game_launcher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

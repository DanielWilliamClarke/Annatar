include(FetchContent)

set(BUILD_SHARED_LIBS OFF)

# Set dependency versions
set(SFML_VERSION 2.6.1)
set(IMGUI_VERSION 1.89)

# Install SFML location
FetchContent_Declare(
    SFML
    URL "https://github.com/SFML/SFML/archive/${SFML_VERSION}.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
# Set SFML options
option(SFML_BUILD_AUDIO "Build audio" OFF)
option(SFML_BUILD_NETWORK "Build network" OFF)
FetchContent_MakeAvailable(sfml)

# Install Imgui location
FetchContent_Declare(
    imgui
    URL "https://github.com/ocornut/imgui/archive/v${IMGUI_VERSION}.zip"
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
# Set Dear ImGui options
FetchContent_MakeAvailable(imgui)

# Install IMGUI_SFML
FetchContent_Declare(
        imgui-sfml
        GIT_REPOSITORY https://github.com/SFML/imgui-sfml.git
        GIT_TAG 2.6.x
)
# Set ImGui-SFML options
set(IMGUI_DIR ${imgui_SOURCE_DIR})
option(IMGUI_SFML_FIND_SFML "Use find_package to find SFML" OFF)
FetchContent_MakeAvailable(imgui-sfml)

# Range V3
FetchContent_Declare(
    Range-v3
    GIT_REPOSITORY "https://github.com/ericniebler/range-v3"
    GIT_TAG "97452bb3eb74a73fc86504421a6a27c92bce6b99"
)
FetchContent_MakeAvailable(Range-v3)

# EnTT (Entity Component System)
FetchContent_Declare(
    EnTT
    GIT_REPOSITORY "https://github.com/skypjack/entt"
    GIT_TAG "v3.13.0"
)
FetchContent_MakeAvailable(EnTT)

# tomlplusplus (TOML config parser - header-only)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY "https://github.com/marzer/tomlplusplus"
    GIT_TAG "v3.4.0"
)
FetchContent_MakeAvailable(tomlplusplus)
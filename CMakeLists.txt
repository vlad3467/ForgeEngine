cmake_minimum_required(VERSION 3.28)

project(ForgeEngine)

set(CMAKE_EXPERIMENTAL_CXX_MODULES OFF)

# Пути
set(SDL3_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3")
set(SDL3_IMAGE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_image")
set(SDL3_TTF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/SDL3_ttf")
set(GLM_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/glm")
set(FREETYPE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/include")
set(FREETYPE_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/external/freetype/lib/libfreetype.a")
set(IMGUI_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")

# FreeType
add_library(freetype_custom STATIC IMPORTED)
set_target_properties(freetype_custom PROPERTIES
    IMPORTED_LOCATION "${FREETYPE_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}"
)

# ImGui библиотека (только основные файлы)
add_library(imgui STATIC
    ${IMGUI_PATH}/imgui.cpp
    ${IMGUI_PATH}/imgui_draw.cpp
    ${IMGUI_PATH}/imgui_tables.cpp
    ${IMGUI_PATH}/imgui_widgets.cpp
)

target_include_directories(imgui PUBLIC ${IMGUI_PATH})

# Основное приложение
add_executable(app 
    code/src/main.cpp
    code/src/shader.cpp
    ${IMGUI_PATH}/imgui_impl_sdl3.cpp
    ${IMGUI_PATH}/imgui_impl_opengl3.cpp
)

# Настройки C++
set_target_properties(app PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Отключаем модули
target_compile_options(app PRIVATE 
    $<$<COMPILE_LANG_AND_ID:CXX,GNU>:-fno-modules>
    $<$<COMPILE_LANG_AND_ID:CXX,Clang>:-fno-modules>
)

# Директории для include
target_include_directories(app PRIVATE
    ${SDL3_PATH}/include
    ${SDL3_IMAGE_PATH}/include
    ${SDL3_TTF_PATH}/include
    ${GLM_PATH}
    ${FREETYPE_INCLUDE_DIR}
    ${IMGUI_PATH}
    ${IMGUI_PATH}/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/code/include
    ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include
)

# Директории для библиотек
target_link_directories(app PRIVATE
    ${SDL3_PATH}/lib
    ${SDL3_IMAGE_PATH}/lib
    ${SDL3_TTF_PATH}/lib
)

# Линковка библиотек
target_link_libraries(app 
    PRIVATE
    SDL3
    SDL3_image
    SDL3_ttf
    freetype_custom
    imgui
    glad
)

# Публичные определения
target_compile_definitions(app PUBLIC 
    RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/"
    SRC_PATH="${CMAKE_CURRENT_SOURCE_DIR}/src/"
)

# Glad
add_subdirectory(external/glad)
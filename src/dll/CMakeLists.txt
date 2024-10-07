cmake_minimum_required(VERSION 3.29 FATAL_ERROR)

project(aliasIsolation)

include(CheckIPOSupported)
check_ipo_supported(RESULT supported OUTPUT error)

add_library(imgui STATIC)

target_include_directories(imgui PUBLIC ../external/imgui)
target_sources(imgui PRIVATE ../external/imgui/imgui.cpp ../external/imgui/imgui_draw.cpp 
../external/imgui/imgui_tables.cpp ../external/imgui/imgui_widgets.cpp 
../external/imgui/backends/imgui_impl_win32.cpp ../external/imgui/backends/imgui_impl_dx11.cpp)

add_library(aliasIsolation MODULE)
set_target_properties(aliasIsolation PROPERTIES SUFFIX ".asi")

if(supported)
    set_property(TARGET aliasIsolation PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
    message(WARNING "IPO / LTO is not supported: ${error}")
endif()

target_link_libraries(aliasIsolation PUBLIC crashHandler imgui minhook TracyClient d3dcompiler.lib d3d11.lib)
target_include_directories(aliasIsolation PRIVATE ../crashHandler 
../external/glm ../external/stb ../external/tracy/public)
target_compile_definitions(aliasIsolation PRIVATE GLM_FORCE_CTOR_INIT GLM_FORCE_AVX)
target_compile_options(aliasIsolation PRIVATE -Wno-microsoft-cast)
target_sources(aliasIsolation PRIVATE dllMain.cpp menu.cpp methodHook.cpp post.cpp 
profiler.cpp rendering.cpp resourceUtil.cpp settings.cpp shaderHooks.cpp shaderRegistry.cpp 
taa.cpp utilities.cpp)
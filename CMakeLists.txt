cmake_minimum_required(VERSION 3.29 FATAL_ERROR)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

project(aliasIsolation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED On)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-use=X:\\aliasIsolation.profdata")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoverage-mapping")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fmodules")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -flto")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=x86-64-v3")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mavx")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /O2 /fp:fast /GR-")

# Needed to fix Wine/Proton usage with latest MSVC ABI, MS made an ABI-breaking change in VS2022 v17.10
# which Wine does not yet support.
add_compile_definitions(_DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR)
#add_compile_definitions(IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

add_subdirectory(src/external/tracy)
add_subdirectory(src/external/minhook)
add_subdirectory(src/crashHandler)
add_subdirectory(src/dll)

# Build HLSL shaders
add_custom_target(shaders)

set(HLSL_SHADER_FILES data/shaders/bloomMerge_ps.hlsl data/shaders/chromaticAberration_ps.hlsl data/shaders/mainPost_vs.hlsl data/shaders/shadowDownsample_ps.hlsl data/shaders/shadowLinearize_ps.hlsl data/shaders/sharpen_ps.hlsl)

set_source_files_properties(data/shaders/bloomMerge_ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(data/shaders/chromaticAberration_ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(data/shaders/mainPost_vs.hlsl PROPERTIES ShaderType "vs")
set_source_files_properties(data/shaders/shadowDownsample_ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(data/shaders/shadowLinearize_ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(data/shaders/sharpen_ps.hlsl PROPERTIES ShaderType "ps")
set_source_files_properties(${HLSL_SHADER_FILES} PROPERTIES ShaderModel "4_0")

foreach(FILE ${HLSL_SHADER_FILES})
  get_filename_component(FILE_WE ${FILE} NAME_WE)
  get_source_file_property(shadertype ${FILE} ShaderType)
  get_source_file_property(shadermodel ${FILE} ShaderModel)
  add_custom_command(TARGET shaders
                     COMMAND fxc.exe /nologo /Emain /T${shadertype}_${shadermodel} /O3 /Ges /Qstrip_reflect /Qstrip_debug /Fo ${CMAKE_BINARY_DIR}/${FILE_WE}.cso ${FILE}
                     MAIN_DEPENDENCY ${FILE}
                     COMMENT "HLSL ${FILE}"
                     WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                     VERBATIM)
endforeach(FILE)
add_dependencies(${PROJECT_NAME} shaders)

cmake_minimum_required(VERSION 3.26)
project(v_engine)

set(CMAKE_CXX_STANDARD 17)

find_package(glfw3 CONFIG REQUIRED)
find_package(Vulkan REQUIRED)
find_package(fmt CONFIG REQUIRED)

add_executable(v_engine
        src/first_app.cpp
        src/first_app.h
        src/lve_window.cpp
        src/lve_window.h
        src/main.cpp
        src/lve_pipeline.cpp
        src/lve_pipeline.h
        src/lve_device.cpp
        src/lve_device.h
        src/lve_swapchain.cpp
        src/lve_swapchain.h)

target_link_libraries(v_engine PRIVATE glfw Vulkan::Vulkan fmt::fmt)


# AUTO COMPILE SHADERS ON BUILD

add_custom_target(shaders)

set(SHADER_FILES shaders/simple_shader.vert shaders/simple_shader.frag)

set_source_files_properties(shaders/*.vert PROPERTIES ShaderType "vs")
set_source_files_properties(shaders/*.frag PROPERTIES ShaderType "fs")

foreach (FILE ${SHADER_FILES})
    get_filename_component(FILE_WE ${FILE} NAME_WE)
    get_source_file_property(shadertype ${FILE} ShaderType)
    add_custom_command(
            TARGET shaders
            COMMAND glslc.exe ${FILE} -o ${FILE}.spv
            MAIN_DEPENDENCY ${FILE}
            COMMENT "GLSL ${FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
    )
    file(COPY ${FILE}.spv DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/shaders/)

endforeach (FILE)

add_dependencies(${PROJECT_NAME} shaders)

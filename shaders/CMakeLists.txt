file(GLOB SHADERS *.vert *.frag)
find_package(Vulkan)

foreach(SHADER IN LISTS SHADERS)
    get_filename_component(FILENAME ${SHADER} NAME)
    set(SPIRV "${PROJECT_BINARY_DIR}/shaders/${FILENAME}.spv")
    message("")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_BINARY_DIR}/shaders/"
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SPIRV}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${FILENAME}")
    list(APPEND SPV_SHADERS ${SPIRV})
endForeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

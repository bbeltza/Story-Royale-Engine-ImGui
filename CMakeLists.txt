if (NOT IMGUI_PATH)
    if (IS_DIRECTORY ${CMAKE_SOURCE_DIR}/imgui)
        set(IMGUI_PATH ${CMAKE_SOURCE_DIR}/imgui)
        set(__MSG "project's source directory $\{CMAKE_SOURCE_DIR\}/imgui")
    elseif(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
        set(IMGUI_PATH ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
        set(__MSG "library's source directory $\{CMAKE_SOURCE_DIR\}/imgui")
    else()
        message(FATAL_ERROR "[SRE_IMGUI] - Could not find ImGui path, you should know that this repository does not include ImGui, so you'll need to clone the implementation entirely. Please, set the variable `IMGUI_PATH` or place ImGui at either $\{CMAKE_SOURCE_DIR%\} or $\{CMAKE_CURRENT_SOURCE_DIR\}")
    endif()

    message(STATUS "Variable `IMGUI_PATH` not found. Using default at ${__MSG}")
else()
    if (NOT IS_DIRECTORY ${IMGUI_PATH})
        if (NOT IS_ABSOLUTE ${IMGUI_PATH})
            message(WARNING "[SRE_IMGUI] - `IMGUI_PATH` is not an absolute path. You might really want to set it as an absolute path to avoid directory conflicts")
        endif()
        message(FATAL_ERROR "[SRE_IMGUI] - Could NOT find path from `IMGUI_PATH` (${IMGUI_PATH}). No such directory.")
    endif()
endif()

project(imgui_sre)
add_library(imgui_sre
    # ImGui sources
      "${IMGUI_PATH}/imgui.cpp"
      "${IMGUI_PATH}/imgui_draw.cpp"
      "${IMGUI_PATH}/imgui_tables.cpp"
      "${IMGUI_PATH}/imgui_widgets.cpp"

    # SRE source
      "src/sre_impl_imgui.cpp"
)

target_link_libraries(imgui_sre PRIVATE sre sre_werror)
target_include_directories(imgui_sre PUBLIC include ${IMGUI_PATH})
target_compile_definitions(imgui_sre PUBLIC SRE_IMGUI=1) # Macro helper to be used by user apps to determine whether imgui is there c:

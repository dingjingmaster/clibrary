include(${CMAKE_SOURCE_DIR}/cx/base/core/core.cmake)

add_library(clibrary-cx-base SHARED ${CX_CORE_SRC})
target_compile_options(clibrary-cx-base PUBLIC -fPIC)
set_target_properties(clibrary-cx-base PROPERTIES VERSION "${PROJECT_VERSION}")
target_compile_definitions(clibrary-cx-base PUBLIC
        -DCX_BOOTSTRAPPED
)

add_library(clibrary-cx-base-static STATIC ${CX_CORE_SRC})
target_compile_options(clibrary-cx-base-static PUBLIC -fPIC)
set_target_properties(clibrary-cx-base-static PROPERTIES OUTPUT_NAME "clibrary-cx-base")
set_target_properties(clibrary-cx-base-static PROPERTIES VERSION "${PROJECT_VERSION}")
target_compile_definitions(clibrary-cx-base-static PUBLIC
        -DCX_BOOTSTRAPPED
)
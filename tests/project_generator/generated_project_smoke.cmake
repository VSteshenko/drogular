if(NOT DEFINED DROGULAR_CLI)
    message(FATAL_ERROR "DROGULAR_CLI is required")
endif()
if(NOT DEFINED DROGULAR_SOURCE_DIR)
    message(FATAL_ERROR "DROGULAR_SOURCE_DIR is required")
endif()
if(NOT DEFINED SMOKE_ROOT)
    message(FATAL_ERROR "SMOKE_ROOT is required")
endif()
if(NOT DEFINED TEMPLATE_ID)
    message(FATAL_ERROR "TEMPLATE_ID is required")
endif()

set(project_name "Smoke_${TEMPLATE_ID}")
set(project_dir "${SMOKE_ROOT}/${project_name}")
set(build_dir "${project_dir}/build")

file(REMOVE_RECURSE "${project_dir}")

execute_process(
    COMMAND "${DROGULAR_CLI}" new "${project_dir}" --template "${TEMPLATE_ID}"
    RESULT_VARIABLE generate_result
    OUTPUT_VARIABLE generate_stdout
    ERROR_VARIABLE generate_stderr
)
if(NOT generate_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to generate ${TEMPLATE_ID} project.\n"
        "stdout:\n${generate_stdout}\n"
        "stderr:\n${generate_stderr}")
endif()

set(configure_args
    -S "${project_dir}"
    -B "${build_dir}"
    "-DFETCHCONTENT_SOURCE_DIR_DROGULAR=${DROGULAR_SOURCE_DIR}"
    "-DCMAKE_BUILD_TYPE=Debug"
)

# This is a nested CMake configure. Reuse the dependency resolution selected
# by the parent Drogular build instead of independently discovering another
# Drogon installation (for example an x86_64 package under /usr/local on an
# Apple Silicon host).
if(DEFINED PARENT_DROGON_DIR AND NOT PARENT_DROGON_DIR STREQUAL "")
    list(APPEND configure_args "-DDrogon_DIR=${PARENT_DROGON_DIR}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" ${configure_args}
    RESULT_VARIABLE configure_result
    OUTPUT_VARIABLE configure_stdout
    ERROR_VARIABLE configure_stderr
)
if(NOT configure_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to configure generated ${TEMPLATE_ID} project.\n"
        "stdout:\n${configure_stdout}\n"
        "stderr:\n${configure_stderr}")
endif()

execute_process(
    COMMAND "${CMAKE_COMMAND}" --build "${build_dir}" --parallel 2
    RESULT_VARIABLE build_result
    OUTPUT_VARIABLE build_stdout
    ERROR_VARIABLE build_stderr
)
if(NOT build_result EQUAL 0)
    message(FATAL_ERROR
        "Failed to build generated ${TEMPLATE_ID} project.\n"
        "stdout:\n${build_stdout}\n"
        "stderr:\n${build_stderr}")
endif()

file(REMOVE_RECURSE "${project_dir}")
function(drogular_cpp_escape INPUT OUTPUT)
    set(value "${INPUT}")
    string(REPLACE "\\" "\\\\" value "${value}")
    string(REPLACE "\"" "\\\"" value "${value}")
    string(REPLACE "\n" "\\n" value "${value}")
    string(REPLACE "\r" "\\r" value "${value}")
    set(${OUTPUT} "${value}" PARENT_SCOPE)
endfunction()

function(drogular_embed_project_templates TEMPLATE_ROOT OUTPUT_FILE)
    file(GLOB manifests CONFIGURE_DEPENDS "${TEMPLATE_ROOT}/*/project.json")

    set(file_entries "")
    set(template_entries "")

    foreach(manifest IN LISTS manifests)
        file(READ "${manifest}" json)
        get_filename_component(template_dir "${manifest}" DIRECTORY)

        string(JSON template_id GET "${json}" id)
        string(JSON template_name GET "${json}" name)
        string(JSON template_description GET "${json}" description)
        string(JSON file_count LENGTH "${json}" files)

        drogular_cpp_escape("${template_id}" template_id_cpp)
        drogular_cpp_escape("${template_name}" template_name_cpp)
        drogular_cpp_escape("${template_description}" template_description_cpp)

        set(template_file_entries "")
        if(file_count GREATER 0)
            math(EXPR last_file "${file_count} - 1")
            foreach(index RANGE 0 ${last_file})
                string(JSON source GET "${json}" files ${index} source)
                string(JSON destination GET "${json}" files ${index} destination)
                string(JSON executable ERROR_VARIABLE executable_error GET "${json}" files ${index} executable)
                if(executable_error)
                    set(executable false)
                endif()

                set(asset_path "${template_dir}/${source}")
                if(NOT EXISTS "${asset_path}")
                    message(FATAL_ERROR "Project template asset not found: ${asset_path}")
                endif()

                file(READ "${asset_path}" content)
                set(key "${template_id}/${source}")

                drogular_cpp_escape("${key}" key_cpp)
                drogular_cpp_escape("${destination}" destination_cpp)

                string(FIND "${content}" ")DRGLTPL\"" delimiter_position)
                if(NOT delimiter_position EQUAL -1)
                    message(FATAL_ERROR "Embedded project template contains reserved raw-string delimiter: ${asset_path}")
                endif()

                string(APPEND file_entries
                    "        {\"${key_cpp}\", R\"DRGLTPL(${content})DRGLTPL\"},\n")
                string(APPEND template_file_entries
                    "                {\"${key_cpp}\", \"${destination_cpp}\", ${executable}},\n")
            endforeach()
        endif()

        string(APPEND template_entries
            "        {\n"
            "            \"${template_id_cpp}\",\n"
            "            \"${template_name_cpp}\",\n"
            "            \"${template_description_cpp}\",\n"
            "            {\n${template_file_entries}            }\n"
            "        },\n")
    endforeach()

    set(DROGULAR_EMBEDDED_FILE_ENTRIES "${file_entries}")
    set(DROGULAR_EMBEDDED_TEMPLATE_ENTRIES "${template_entries}")
    configure_file(
        "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/embedded_template_data.cpp.in"
        "${OUTPUT_FILE}"
        @ONLY
    )
endfunction()
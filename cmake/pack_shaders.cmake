cmake_policy(SET CMP0053 NEW)

# Function to generate key
function(generate_key LENGTH RESULT_VAR)
    # Use the `openssl` command to generate random characters
    execute_process(
        COMMAND openssl rand -base64 ${LENGTH}
        OUTPUT_VARIABLE random_string
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    # Trim to the requested length (if necessary)
    string(SUBSTRING "${random_string}" 0 ${LENGTH} key)

    # Set the result
    set(${RESULT_VAR} "${key}" PARENT_SCOPE)
endfunction()

# Obfuscate data by XOR with a key
function(obfuscate_data DATA KEY FILE_PATH)
    # Initialize the obfuscated data
    file(READ ${null_path} null_char)
    file(WRITE ${FILE_PATH} "")

    # Loop through the characters and apply XOR
    string(LENGTH "${KEY}" key_length)
    string(LENGTH "${DATA}" data_length)
    math(EXPR end_range "${data_length} - 1")
    foreach(i RANGE 0 ${end_range})
        string(SUBSTRING "${DATA}" ${i} 1 char)
        math(EXPR key_index "${i} % ${key_length}")
        string(SUBSTRING "${KEY}" ${key_index} 1 key_char)

        # Convert characters to HEX values
        string(HEX "${char}" char_hex)
        string(PREPEND char_hex "0x")
        string(HEX "${key_char}" key_hex)
        string(PREPEND key_hex "0x")

        # XOR operation using HEX values
        math(EXPR obfuscated_val "${char_hex} ^ ${key_hex}")
        if(obfuscated_val EQUAL 0)
            file(APPEND ${FILE_PATH} "${null_char}")
        else()
            string(ASCII "${obfuscated_val}" obfuscated_ascii)
            file(APPEND ${FILE_PATH} "${obfuscated_ascii}")
        endif()
    endforeach()
endfunction()

# Pack given shader files to one binary file
function(pack_shaders KEY PACKED_SHADERS_FILE SHADERS_TO_PACK)
    message(STATUS "Packing shaders into ${PACKED_SHADERS_FILE}")
    file(WRITE ${PACKED_SHADERS_FILE} "") # Clean file content

    list(LENGTH SHADERS_TO_PACK NUM_SHADERS)
    set(CURRENT_INDEX 0)

    foreach(SHADER_FILE ${SHADERS_TO_PACK})
        math(EXPR CURRENT_INDEX "${CURRENT_INDEX} + 1")
        # Read shader content
        file(READ ${SHADER_FILE} CONTENTS)

        # Get shader file name
        get_filename_component(FILE_NAME ${SHADER_FILE} NAME)

        # Save name and content in binary file
        file(APPEND ${PACKED_SHADERS_FILE}
            "[BEGIN_SHADER]\n"
            "FILE_NAME=${FILE_NAME}\n"
            "${CONTENTS}\n"
            "[END_SHADER]\n"
        )
        message(STATUS "[${CURRENT_INDEX}/${NUM_SHADERS}] ${FILE_NAME} packed!")
    endforeach()

    file(READ ${PACKED_SHADERS_FILE} TEXT)
    obfuscate_data("${TEXT}" "${KEY}" "${PACKED_SHADERS_FILE}")
endfunction()
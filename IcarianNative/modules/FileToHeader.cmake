#FileToHeader(SOURCE_FILE HEADER_FILE VARIABLE_NAME)
function(FILETOHEADER)
    set(singleValueArgs SOURCE_FILE HEADER_FILE VARIABLE_NAME)    
    cmake_parse_arguments(FILETOHEADER "" "${singleValueArgs}" "" ${ARGN})

    file(READ "${FILETOHEADER_SOURCE_FILE}" FILESTRING)
    #It just works do not question it
    string(REPLACE "\n" "\\n\\\n" FILESTRING "${FILESTRING}")

    string(MAKE_C_IDENTIFIER "${FILETOHEADER_VARIABLE_NAME}" FILETOHEADER_VARIABLE_NAME)
    string(TOUPPER "${FILETOHEADER_VARIABLE_NAME}" FILETOHEADER_VARIABLE_NAME)

    #I have decided to not mess around and it will always be a string so I am just gonna do a define
    set(DEFINITION "#define ${FILETOHEADER_VARIABLE_NAME} \"${FILESTRING}\"")

    file(WRITE ${FILETOHEADER_HEADER_FILE} "#pragma once \n\n${DEFINITION}")
endfunction()
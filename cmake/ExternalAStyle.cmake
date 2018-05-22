# ------------------------------------------------------------------------------
# Astyle
# ------------------------------------------------------------------------------

# AStyle is a style check tool. It automatically fixes basic whitespace and other problems when run.
# This tool only runs on Linux. It should support Windows, but that's not tested.
# TO use, run:
# cmake -DENABLE_ASTYLE=TRUE ..
# make && make format
# this will automatically make changes to your source files.

if(ENABLE_ASTYLE)

    # ExternalProject is a CMake library that provides the ability to download dependencies from github.
    include(ExternalProject)


    list(APPEND ASTYLE_CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}"
    )

    ExternalProject_Add(
        astyle
        GIT_REPOSITORY      https://github.com/Bareflank/astyle.git
        GIT_TAG             e6bf1cfa95ea173e8276e6d28197db31bb52df08 #v1.2
        GIT_SHALLOW         1
        CMAKE_ARGS          ${ASTYLE_CMAKE_ARGS}
        PREFIX              ${CMAKE_BINARY_DIR}/external/astyle/prefix
        TMP_DIR             ${CMAKE_BINARY_DIR}/external/astyle/tmp
        STAMP_DIR           ${CMAKE_BINARY_DIR}/external/astyle/stamp
        DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/astyle/download
        SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/astyle/src
        BINARY_DIR          ${CMAKE_BINARY_DIR}/external/astyle/build
    )

    list(APPEND ASTYLE_ARGS
        --style=allman              #Brace style, Saleae standard.
        --suffix=none               #Do not retain a backup of the original file. The original file is purged after it is formatted.
        --pad-oper                  #Insert space padding around operators
        --unpad-paren               #Remove extra space padding around parens on the inside and outside. can be used with other pad operators.
        --pad-paren-in              #Insert space padding around paren on the inside only.
        --break-blocks              #Pad empty lines around header blocks (e.g. 'if', 'for', 'while'...).
        --align-pointer=type        #Attach a pointer or reference operator (*, &, or ^) to either the variable type (left)
        --align-reference=type      #This option will align references separate from pointers.
        --indent-preproc-define     #indents multi-line preprocessor definitions
        --indent-switches           #indents switch statement contents.
        --indent-col1-comments      #Indent C++ comments beginning in column one.
        --indent-classes            #indents entire class contents.
        --indent-namespaces         #indents contents of namespaces.
        --keep-one-line-statements  #Don't break complex statements and multiple statements residing on a single line.
        --keep-one-line-blocks      #Don't break one-line blocks.
        --pad-header                #Insert space padding between a header (e.g. 'if', 'for', 'while'...) and the following paren.
        --convert-tabs              #Converts tabs into spaces in the non-indentation part of the line.
        --min-conditional-indent=0  #Set the minimal indent that is added when a header is built of multiple lines.
        --indent=spaces=4           #4 intents are 4 spaces.
        --add-brackets              #Add braces to unbraced one line conditional statements (e.g. 'if', 'for', 'while'...). The statement must be on a single line.
        --break-after-logical       #The option break‑after‑logical will cause the logical conditionals to be placed last on the previous line. Not sure if used without Max line length.
        --indent-preproc-block      #indent block preprocessor statements
    )

    list(APPEND ASTYLE_FILES ${SOURCES})
    list(APPEND ASTYLE_ARGS ${ASTYLE_FILES})

    if(NOT WIN32 STREQUAL "1")
        add_custom_target(
            format
            COMMAND ${CMAKE_BINARY_DIR}/bin/astyle ${ASTYLE_ARGS}
            COMMENT "running astyle"
        )
    else()
        add_custom_target(
            format
            COMMAND ${CMAKE_BINARY_DIR}/bin/astyle.exe ${ASTYLE_ARGS}
            COMMENT "running astyle"
        )
    endif()
endif()

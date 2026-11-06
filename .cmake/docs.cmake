# Output paths (override JUG_DOCS_BASE_DIR via -DJUG_DOCS_BASE_DIR=/path/to/docusaurus/docs)
set(JUG_DOCS_XML_DIR "${JUG_BUILD_DIR}/docs_xml")
if(NOT DEFINED JUG_DOCS_BASE_DIR)
    set(JUG_DOCS_BASE_DIR "${JUG_MAIN_DIR}/.out/docs")
endif()
set(JUG_DOCS_OUTPUT_DIR "${JUG_DOCS_BASE_DIR}/api")

find_package(Doxygen QUIET)
find_program(NPX_EXECUTABLE NAMES npx.cmd npx)

if(DOXYGEN_FOUND AND NPX_EXECUTABLE)
    message(STATUS "[DOCS] Doxygen (${DOXYGEN_EXECUTABLE}) and npx (${NPX_EXECUTABLE}) found. Target 'jug-api-docs' enabled.")

    set(DOXYFILE_IN "${JUG_CMAKE_DIR}/docs/Doxyfile.in")
    set(DOXYFILE_OUT "${JUG_BUILD_DIR}/Doxyfile")
    set(DOXY2DOCU_IN "${JUG_CMAKE_DIR}/docs/doxygen2docusaurus.json.in")
    set(DOXY2DOCU_OUT "${JUG_BUILD_DIR}/doxygen2docusaurus.json")

    configure_file("${DOXYFILE_IN}" "${DOXYFILE_OUT}" @ONLY)
    configure_file("${DOXY2DOCU_IN}" "${DOXY2DOCU_OUT}" @ONLY)

    add_custom_target(jug-api-docs
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${JUG_DOCS_XML_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E rm -rf "${JUG_DOCS_BASE_DIR}"
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${JUG_DOCS_OUTPUT_DIR}"
        COMMAND "${DOXYGEN_EXECUTABLE}" "${DOXYFILE_OUT}"
        COMMAND "${NPX_EXECUTABLE}" --yes @xpack/doxygen2docusaurus
                --config "${DOXY2DOCU_OUT}"
        WORKING_DIRECTORY "${JUG_BUILD_DIR}"
        COMMENT "[DOCS] Extracting C++ XML via Doxygen and generating Docusaurus Markdown via @xpack/doxygen2docusaurus..."
        VERBATIM
    )
else()
    add_custom_target(jug-api-docs
        COMMAND "${CMAKE_COMMAND}" -E echo "[DOCS] Cannot build 'jug-api-docs': please ensure both 'doxygen' and Node.js ('npx') are installed and in PATH."
        COMMAND "${CMAKE_COMMAND}" -E false
        VERBATIM
    )
endif()

# Copy license
if(NOT DEFINED JUG_COPIED_LICENSES)
    message(STATUS "[LEGAL] Copying licenses...")
    file(COPY ${JUG_LICENSE_FILE} DESTINATION ${JUG_DIST_FINAL_DIR})
    set(JUG_COPIED_LICENSES ON CACHE BOOL "Jug licensing tracking" FORCE)
endif()

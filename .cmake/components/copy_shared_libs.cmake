file(GLOB LIBS "${SRC_DIR}/*.dll" "${SRC_DIR}/*.so*" "${SRC_DIR}/*.dylib*")
if(LIBS)
    file(COPY ${LIBS} DESTINATION "${DST_DIR}")
endif()

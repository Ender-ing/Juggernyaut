# apply_patches.cmake
message(STATUS "[LSP] Applying patches sequentially...")

execute_process(
    COMMAND "${PATCH_EXE}" -N --forward --force --fuzz=0 
            CMakeLists.txt 
            -i "${PATCH_DIR}/LSPFramework_patch__CMakeLists.txt.diff"
)

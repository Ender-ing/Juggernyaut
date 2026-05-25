# apply_patches.cmake
message(STATUS "[ANTLR4] Applying patches sequentially...")

execute_process(
    COMMAND "${PATCH_EXE}" -N --forward --force --fuzz=0 
            runtime/Cpp/runtime/CMakeLists.txt 
            -i "${PATCH_DIR}/antlr4_runtime_patch__CMakeLists.txt.diff"
)

execute_process(
    COMMAND "${PATCH_EXE}" -N --forward --force --fuzz=0 
            runtime/Cpp/runtime/src/atn/ProfilingATNSimulator.cpp 
            -i "${PATCH_DIR}/antlr4_runtime_patch__ProfilingATNSimulator.cpp.diff"
)

execute_process(
    COMMAND "${PATCH_EXE}" -N --forward --force --fuzz=0 
            runtime/Cpp/runtime/antlrcpp.xcodeproj/project.pbxproj 
            -i "${PATCH_DIR}/antlr4_runtime_patch__project.pbxproj.diff"
)

execute_process(
    COMMAND "${PATCH_EXE}" -N --forward --force --fuzz=0 -p1 -d . 
            -i "${PATCH_DIR}/antlr4_runtime_patch__WindowsMacroFix.diff"
)

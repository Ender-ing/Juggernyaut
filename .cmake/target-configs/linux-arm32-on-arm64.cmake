message(STATUS "[TOOLCHAIN] Applying configs: ${CMAKE_CURRENT_LIST_FILE}")

# Internal configs
#set(CMAKE_GENERATOR_PLATFORM ? CACHE STRING "" FORCE)
set(CMAKE_UNIX_GENERATOR_PLATFORM ARM CACHE STRING "" FORCE)
set(JUG_BUILD_PLATFORM_NAME arm32 CACHE STRING "" FORCE)
set(JUG_MATCH_INSTALL_ARCH ON CACHE STRING "" FORCE)

# CMake configs
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR arm)
#set(CMAKE_LIBRARY_PATH "?")
#set(CMAKE_INCLUDE_PATH "?")
set(CMAKE_FIND_ROOT_PATH "/usr/arm-linux-gnueabihf")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# CTest Configs
set(CMAKE_TEST_ENVIRONMENT
    "LSAN_OPTIONS=fast_unwind_on_malloc=0"
    "ASAN_OPTIONS=fast_unwind_on_malloc=0:detect_leaks=1"
)

# C/C++ compiler configs
set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_CROSSCOMPILING_EMULATOR "qemu-arm-static"
    "-E" "LSAN_OPTIONS=fast_unwind_on_malloc=0"
    "-E" "ASAN_OPTIONS=fast_unwind_on_malloc=0:detect_leaks=1"
    "-L" "/usr/arm-linux-gnueabihf"
)

# C/C++ flags & linker flags
set(CMAKE_C_FLAGS "-mfloat-abi=hard" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "-mfloat-abi=hard" CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "-mfloat-abi=hard" CACHE STRING "" FORCE)
set(CMAKE_SHARED_LINKER_FLAGS "-mfloat-abi=hard" CACHE STRING "" FORCE)

# OpenSSL configs (for libgit2)
set(USE_HTTPS OpenSSL CACHE STRING "" FORCE)
set(OPENSSL_INCLUDE_DIR "/usr/include/arm-linux-gnueabihf" CACHE STRING "" FORCE)
set(OPENSSL_CRYPTO_LIBRARY "/usr/lib/arm-linux-gnueabihf/libcrypto.so" CACHE STRING "" FORCE)
set(OPENSSL_SSL_LIBRARY "/usr/lib/arm-linux-gnueabihf/libssl.so" CACHE STRING "" FORCE)

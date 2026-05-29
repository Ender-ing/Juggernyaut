set(GLOBAL_PACKAGE_EMAIL_ADDRESS "admin@ender.ing")
set(GLOBAL_PACKAGE_DESCRIPTION "A complete Toolchain for the Juggernyaut general programming language.")
set(GLOBAL_PACKAGE_DOCS "https://ender.ing/docs/juggernyaut/")
set(GLOBAL_DISPLAY_PACKAGE_NAME "Juggernyaut Toolchain")
set(GLOBAL_WIN_ICO "${JUG_CMAKE_DIR}/installer/assets/jug_icon.ico")

# Package Identity
set(CPACK_PACKAGE_NAME ${GLOBAL_DISPLAY_PACKAGE_NAME})
set(CPACK_PACKAGE_VENDOR "Ender-ing")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${GLOBAL_PACKAGE_DESCRIPTION})
set(CPACK_PACKAGE_CONTACT ${GLOBAL_PACKAGE_EMAIL_ADDRESS})
set(CPACK_IFW_PRODUCT_URL ${GLOBAL_PACKAGE_DOCS})
if (WIN32)
    set(CPACK_IFW_PACKAGE_ICON ${GLOBAL_WIN_ICO})
elseif(APPLE)
    set(CPACK_IFW_PACKAGE_ICON "${JUG_CMAKE_DIR}/installer/assets/jug_icon.icns")
endif()

sanitize_version("${JUG_RELEASE_VERSION}" PROPER_RELEASE_VERSION)
set(CPACK_PACKAGE_VERSION "${PROPER_RELEASE_VERSION}")

# Generators' Installer configs
configure_file(
    "${JUG_CMAKE_DIR}/installer/package-gen.cmake.in"
    "${CMAKE_BINARY_DIR}/package-gen.cmake"
    @ONLY
)
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_BINARY_DIR}/package-gen.cmake")

# Generators
set(CPACK_GENERATOR "IFW")
if(WIN32)
    # Limit IFW to the available public binaries
    if((${JUG_BINARY_PLATFORM} STREQUAL "arm64") OR (${JUG_BINARY_PLATFORM} STREQUAL "x86_64"))
        list(APPEND CPACK_GENERATOR "NSIS")
    else()
        set(CPACK_GENERATOR "NSIS")
    endif()

    # Package Identity
    set(CPACK_NSIS_MUI_ICON ${GLOBAL_WIN_ICO})
    set(CPACK_NSIS_MUI_UNIICON ${GLOBAL_WIN_ICO})

    # modify PATH
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_PATH_EXTRA "%INSTDIR%\\\\bin")

    # Prevent files duplication
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

    # Ensure it doesn't try to add desktop shortcuts if you don't want them
    set(CPACK_NSIS_MENU_LINKS "bin/Juggernyaut.exe" "Juggernyaut")
elseif(APPLE)
    # Limit IFW to the available public binaries
    if((NOT ${JUG_BINARY_PLATFORM} STREQUAL "arm64") AND (NOT ${JUG_BINARY_PLATFORM} STREQUAL "x86_64"))
        message(WARNING "Generating an installer for this platform is not supported!")
        set(CPACK_GENERATOR "")
    endif()
elseif(UNIX)
    # Limit IFW to the available public binaries
    if((${JUG_BINARY_PLATFORM} STREQUAL "arm64") OR (${JUG_BINARY_PLATFORM} STREQUAL "x86_64"))
        list(APPEND CPACK_GENERATOR "DEB")
    else()
        set(CPACK_GENERATOR "DEB")
    endif()

    # Configs
    set(CPACK_DEBIAN_PACKAGE_NAME "Juggernyaut-Toolchain")
    set(CPACK_DEBIAN_PACKAGE_VERSION ${PROPER_RELEASE_VERSION})
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER ${GLOBAL_PACKAGE_EMAIL_ADDRESS})
    set(DEB_ARCH ${CMAKE_SYSTEM_PROCESSOR})
    if(DEB_ARCH STREQUAL "x86_64")
        set(DEB_ARCH "amd64")
    elseif(DEB_ARCH STREQUAL "aarch64")
        set(DEB_ARCH "arm64")
    endif()
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE ${DEB_ARCH})
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION ${GLOBAL_PACKAGE_DESCRIPTION})
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    
    # Let CPack find dependencies automatically
    set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)
endif()
list(FIND CPACK_GENERATOR "IFW" IFW_INDEX)

list(FIND CPACK_GENERATOR "IFW" IFW_INDEX)
if(IFW_INDEX GREATER -1)
    set(JUG_USES_IFW ON)
else()
    set(JUG_USES_IFW OFF)
endif()

# Configure IFW (QT Installer framework)
#set(CPACK_IFW_FRAMEWORK_VERSION 4.8.1)
# READ: https://cmake.org/cmake/help/latest/cpack_gen/ifw.html
# READ: https://doc.qt.io/qtinstallerframework/ifw-reference.html

# IFW Branding & Theming
set(CPACK_IFW_PACKAGE_TITLE ${GLOBAL_DISPLAY_PACKAGE_NAME})
set(CPACK_IFW_PACKAGE_PUBLISHER "ender.ing")

set(CPACK_RESOURCE_FILE_LICENSE "${JUG_LICENSE_FILE}")

# Theming
set(CPACK_IFW_PACKAGE_WIZARD_STYLE "Modern")
# The left-side tall image (Classic setup wizard style)
set(CPACK_IFW_PACKAGE_WATERMARK "${JUG_CMAKE_DIR}/installer/assets/ifw_watermark.png")
# The top-right header logo
set(CPACK_IFW_PACKAGE_LOGO "${JUG_CMAKE_DIR}/installer/assets/ifw_backdrop_logo.png")
# The small window icon
set(CPACK_IFW_PACKAGE_WINDOW_ICON "${JUG_CMAKE_DIR}/installer/assets/jug_icon.png")
# Top bar banner
set(CPACK_IFW_PACKAGE_BANNER "${JUG_CMAKE_DIR}/installer/assets/ifw_banner.png")
# Colouring
set(CPACK_IFW_PACKAGE_TITLE_COLOR "#FFF3E3")
set(CPACK_IFW_PACKAGE_STYLE_SHEET "${JUG_CMAKE_DIR}/installer/assets/style.qss")

# Wizard styling
set(CPACK_IFW_PACKAGE_CONTROL_SCRIPT "${JUG_CMAKE_DIR}/installer/controller.qs")
set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_WIDTH 800)
set(CPACK_IFW_PACKAGE_WIZARD_DEFAULT_HEIGHT 600)
set(CPACK_IFW_PACKAGE_WIZARD_SHOW_PAGE_LIST ON)
# CPACK_IFW_PACKAGE_RESOURCES

# Behaviour
set(CPACK_IFW_PACKAGE_REMOVE_TARGET_DIR ON)
set(CPACK_IFW_PACKAGE_MAINTENANCE_TOOL_NAME "maintenancetool")
set(CPACK_IFW_PACKAGE_ALLOW_NON_ASCII_CHARACTERS OFF)
set(CPACK_IFW_PACKAGE_ALLOW_SPACE_IN_PATH ON)
set(CPACK_IFW_PACKAGE_DISABLE_COMMAND_LINE_INTERFACE OFF) # Allow CLI interactions
#set(CPACK_IFW_PACKAGE_RUN_PROGRAM "@TargetDir@/bin/jug-pck")
#set(CPACK_IFW_PACKAGE_RUN_PROGRAM_ARGUMENTS install std)
#set(CPACK_IFW_PACKAGE_RUN_PROGRAM_DESCRIPTION "Install the standard Juggernyaut standard library.")

set(CPACK_COMPONENTS_ALL 
    # Project components
    CmpJuggernyautCompiler
    CmpJuggernyautServer
    CmpJuggernyautPackageManager
    CmpJuggernyautUnified

    # System components
    CmpSystemRuntimeLibs
)
if(JUG_USES_IFW)
    list(APPEND CPACK_COMPONENTS_ALL QSPathSetup)
    if(WIN32)
        list(APPEND CPACK_COMPONENTS_ALL QSMenuShortcuts)
    endif()
endif()

# Separate components
set(CPACK_COMPONENTS_GROUPING IGNORE)

# Installation options

# Toolchain specifics
cpack_add_component_group(Toolchain
    DISPLAY_NAME "Toolchain Internals"
    DESCRIPTION "The core Juggernyaut executables and developer tools."
    EXPANDED
)
cpack_add_component(CmpJuggernyautCompiler
    DISPLAY_NAME "Juggernyaut Compiler"
    DESCRIPTION "Adds the Juggernyaut Compiler that allows you to build your final binaries."
    GROUP Toolchain
    REQUIRED TRUE
)
cpack_add_component(CmpJuggernyautServer
    DISPLAY_NAME "Juggernyaut Language Server"
    DESCRIPTION "Adds the Juggernyaut Language Server, which is necessary for IDE integration."
    GROUP Toolchain
)
cpack_add_component(CmpJuggernyautPackageManager
    DISPLAY_NAME "Juggernyaut Package Manager"
    DESCRIPTION "Adds the Juggernyaut Package Manager, which allows for easy management of global Juggernyaut librarie."
    GROUP Toolchain
    DISABLED TRUE
)
cpack_add_component(CmpJuggernyautUnified
    DISPLAY_NAME "Juggernyaut unified command"
    DESCRIPTION "Adds a unified 'jug' command that can call all other toolchain components."
    GROUP Toolchain
    DISABLED TRUE
)

# System
cpack_add_component_group(SystemConfigs
    DISPLAY_NAME "System Configurations"
    DESCRIPTION "Local system configurations that can help make your experience better."
)
if(JUG_USES_IFW)
    cpack_add_component(QSPathSetup
        DISPLAY_NAME "Add ${GLOBAL_DISPLAY_PACKAGE_NAME} executables to System PATH"
        DESCRIPTION "Allows you to run toolchain commands (e.g. 'jug') from any terminal."
        GROUP SystemConfigs
    )
endif()
cpack_add_component(CmpSystemRuntimeLibs
    DISPLAY_NAME "Required system runtime components"
    DESCRIPTION "Adds system runtime components that are required by some toolchain binaries. (Unselect only if you know what you're doing!)"
    GROUP SystemConfigs
)
if(WIN32 AND JUG_USES_IFW)
    cpack_add_component(QSMenuShortcuts
        DISPLAY_NAME "Add ${GLOBAL_DISPLAY_PACKAGE_NAME} Start Menu shortcuts"
        DESCRIPTION "Adds Start Menu shortcuts. (e.g. uninstaller shortcut)"
        GROUP SystemConfigs
    )
endif()

# Handle online fetching
# cpack_ifw_add_repository()
# cpack_ifw_update_repository()
# cpack_configure_downloads()

# Initialize CPack
include(CPack)
if(JUG_USES_IFW)
    # Initialize CPackIFW (MUST come after include(CPack))
    include(CPackIFW)
endif()

if(JUG_USES_IFW)
    cpack_ifw_configure_component(CmpJuggernyautCompiler
        FORCED_INSTALLATION TRUE
        LICENSES 
            "${GLOBAL_DISPLAY_PACKAGE_NAME} License" "${JUG_LICENSE_FILE}"
    )
    cpack_ifw_configure_component(QSPathSetup
        SCRIPT "${JUG_CMAKE_DIR}/installer/path-setup.qs"
    )
    if(WIN32)
        cpack_ifw_configure_component(QSMenuShortcuts
            SCRIPT "${JUG_CMAKE_DIR}/installer/menu-shortcuts.qs"
        )
    endif()
endif()

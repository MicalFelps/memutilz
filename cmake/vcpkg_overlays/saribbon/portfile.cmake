vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO MicalFelps/SARibbon
    REF 299848bd59abd0aed41c92f3b9155259fbc72b98
    SHA512 0                    # fill in after first build
    HEAD_REF dev
)

set(SARIBBON_BUILD_STATIC_LIBS OFF)
if(VCPKG_LIBRARY_LINKAGE STREQUAL "static")
    set(SARIBBON_BUILD_STATIC_LIBS ON)
endif()

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        examples SARIBBON_BUILD_EXAMPLES
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DSARIBBON_BUILD_STATIC_LIBS=${SARIBBON_BUILD_STATIC_LIBS}
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()

vcpkg_cmake_config_fixup(
    PACKAGE_NAME SARibbon
    CONFIG_PATH lib/cmake/SARibbon
)

vcpkg_clean_executables_in_bin()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")
vcpkg_copy_pdbs()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
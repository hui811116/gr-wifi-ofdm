INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_WIFI_OFDM wifi_ofdm)

FIND_PATH(
    WIFI_OFDM_INCLUDE_DIRS
    NAMES wifi_ofdm/api.h
    HINTS $ENV{WIFI_OFDM_DIR}/include
        ${PC_WIFI_OFDM_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    WIFI_OFDM_LIBRARIES
    NAMES gnuradio-wifi_ofdm
    HINTS $ENV{WIFI_OFDM_DIR}/lib
        ${PC_WIFI_OFDM_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(WIFI_OFDM DEFAULT_MSG WIFI_OFDM_LIBRARIES WIFI_OFDM_INCLUDE_DIRS)
MARK_AS_ADVANCED(WIFI_OFDM_LIBRARIES WIFI_OFDM_INCLUDE_DIRS)


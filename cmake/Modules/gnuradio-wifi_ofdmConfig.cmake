find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_WIFI_OFDM gnuradio-wifi_ofdm)

FIND_PATH(
    GR_WIFI_OFDM_INCLUDE_DIRS
    NAMES gnuradio/wifi_ofdm/api.h
    HINTS $ENV{WIFI_OFDM_DIR}/include
        ${PC_WIFI_OFDM_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_WIFI_OFDM_LIBRARIES
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

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-wifi_ofdmTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_WIFI_OFDM DEFAULT_MSG GR_WIFI_OFDM_LIBRARIES GR_WIFI_OFDM_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_WIFI_OFDM_LIBRARIES GR_WIFI_OFDM_INCLUDE_DIRS)

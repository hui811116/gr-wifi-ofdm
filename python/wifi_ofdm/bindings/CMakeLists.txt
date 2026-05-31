# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT wifi_ofdm_sources)
    message(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT wifi_ofdm_sources)

########################################################################
# Check for pygccxml
########################################################################
gr_python_check_module_raw("pygccxml" "import pygccxml" PYGCCXML_FOUND)

include(GrPybind)

########################################################################
# Python Bindings
########################################################################
list(APPEND wifi_ofdm_python_files
    ppdu_builder_python.cc
    ppdu_sink_python.cc
    header_prefixer_python.cc
    coarse_cfo_fixer_cc_python.cc
    block_decoder_vc_python.cc
    cp_preamble_prefixer_vcc_python.cc
    symbol_mapper_bvc_python.cc
    symbol_parser_vc_python.cc
    symbol_sync_cvc_python.cc python_bindings.cc)

gr_pybind_make_oot(wifi_ofdm ../../.. gr::wifi_ofdm "${wifi_ofdm_python_files}")

# copy bindings extension for use in QA test module
add_custom_command(
    TARGET wifi_ofdm_python
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:wifi_ofdm_python>
            ${PROJECT_BINARY_DIR}/test_modules/gnuradio/wifi_ofdm/)

install(
    TARGETS wifi_ofdm_python
    DESTINATION ${GR_PYTHON_DIR}/gnuradio/wifi_ofdm
    COMPONENT pythonapi)

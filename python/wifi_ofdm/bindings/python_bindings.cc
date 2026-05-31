/*
 * Copyright 2020 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

namespace py = pybind11;

// Headers for binding functions
/**************************************/
// The following comment block is used for
// gr_modtool to insert function prototypes
// Please do not delete
/**************************************/
// BINDING_FUNCTION_PROTOTYPES(
    void bind_ppdu_builder(py::module& m);
    void bind_ppdu_sink(py::module& m);
    void bind_header_prefixer(py::module& m);
    void bind_coarse_cfo_fixer_cc(py::module& m);
    void bind_block_decoder_vc(py::module& m);
    void bind_preamble_prefixer_vcc(py::module& m);
    void bind_symbol_mapper_bvc(py::module& m);
    void bind_symbol_parser_vc(py::module& m);
    void bind_symbol_sync_cvc(py::module& m);
// ) END BINDING_FUNCTION_PROTOTYPES


// We need this hack because import_array() returns NULL
// for newer Python versions.
// This function is also necessary because it ensures access to the C API
// and removes a warning.
void* init_numpy()
{
    import_array();
    return NULL;
}

PYBIND11_MODULE(wifi_ofdm_python, m)
{
    // Initialize the numpy C API
    // (otherwise we will see segmentation faults)
    init_numpy();

    // Allow access to base block methods
    py::module::import("gnuradio.gr");

    /**************************************/
    // The following comment block is used for
    // gr_modtool to insert binding function calls
    // Please do not delete
    /**************************************/
    // BINDING_FUNCTION_CALLS(
    bind_ppdu_builder(m);
    bind_ppdu_sink(m);
    bind_header_prefixer(m);
    bind_coarse_cfo_fixer_cc(m);
    bind_block_decoder_vc(m);
    bind_preamble_prefixer_vcc(m);
    bind_symbol_mapper_bvc(m);
    bind_symbol_parser_vc(m);
    bind_symbol_sync_cvc(m);
    // ) END BINDING_FUNCTION_CALLS
}
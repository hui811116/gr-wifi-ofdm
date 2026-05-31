/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_COARSE_CFO_FIXER_CC_IMPL_H
#define INCLUDED_WIFI_OFDM_COARSE_CFO_FIXER_CC_IMPL_H

#include <gnuradio/wifi_ofdm/coarse_cfo_fixer_cc.h>

namespace gr {
namespace wifi_ofdm {

class coarse_cfo_fixer_cc_impl : public coarse_cfo_fixer_cc
{
private:
    gr_complex * d_in_x_delay;
    gr_complex * d_in_square;
    gr_complex * d_delay_square;
    int d_state;
    int d_ncnt;
    int d_delay;
    float d_coarse_cfo;
    float d_phase;
    pmt::pmt_t d_block_name;

public:
    coarse_cfo_fixer_cc_impl(int delay);
    ~coarse_cfo_fixer_cc_impl();

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_COARSE_CFO_FIXER_CC_IMPL_H */

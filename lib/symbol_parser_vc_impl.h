/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_SYMBOL_PARSER_VC_IMPL_H
#define INCLUDED_WIFI_OFDM_SYMBOL_PARSER_VC_IMPL_H

#include <gnuradio/wifi_ofdm/symbol_parser_vc.h>

namespace gr {
namespace wifi_ofdm {

static const gr_complex d_pilot[4] = {
    gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0)
};
static const gr_complex d_long[64] = {
    gr_complex(0,0),gr_complex(0,0),gr_complex(0,0),gr_complex(0,0),gr_complex(0,0),gr_complex(0,0),gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(-1,0),
    gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),
    gr_complex(1,0),gr_complex(-1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(1,0),
    gr_complex(1,0),gr_complex(1,0),gr_complex(0,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),
    gr_complex(-1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(-1,0),gr_complex(-1,0),gr_complex(-1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0),
    gr_complex(-1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(-1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(0,0),
    gr_complex(0,0),gr_complex(0,0),gr_complex(0,0),gr_complex(0,0)
};
static const float d_pilot_sign[127] = {
    1,1,1,1, -1,-1,-1,1, -1,-1,-1,-1, 1,1,-1,1, -1,-1,1,1, -1,1,1,-1, 1,1,1,1, 1,1,-1,1,
    1,1,-1,1, 1,-1,-1,1, 1,1,-1,1, -1,-1,-1,1, -1,1,-1,-1, 1,-1,-1,1, 1,1,1,1, -1,-1,1,1,
    -1,-1,1,-1, 1,-1,1,1, -1,-1,-1,1, 1,-1,-1,-1, -1,1,-1,-1, 1,-1,1,1, 1,1,-1,1, -1,1,-1,1,
    -1,-1,-1,-1, -1,1,-1,1, 1,-1,1,-1, 1,1,1,-1, -1,1,-1,-1, -1,1,1,1, -1,-1,-1,-1, -1,-1,-1
};
static const int d_subcarrier_type[64] = {
    0,0,0,0,0,0,1,1,1,1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,1,1,1,1,1,1,
    0,1,1,1,1,1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,1,1,1,1,1,0,0,0,0,0
};

static const int d_datacarr_idx[48] = {
    38,39,40,41,42,
    44,45,46,47,48,49,50,51,52,53,54,55,56,
    58,59,60,61,62,63,
    1,2,3,4,5,6,
    8,9,10,11,12,13,14,15,16,17,18,19,20,
    22,23,24,25,26,
};

static const int d_desubcarr_idx[64] = {
    32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
    0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
};

class symbol_parser_vc_impl : public symbol_parser_vc
{
private:
    gr_complex * d_channel_est;
    gr_complex * d_buf;
    int d_pilot_idx;
    const pmt::pmt_t d_block_name;

public:
    symbol_parser_vc_impl();
    ~symbol_parser_vc_impl();

    void symbol_eq(gr_complex* out, const gr_complex* in, int pilot_idx);
    void channel_estimation(const gr_complex* in);

    // Where all the action really happens
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_SYMBOL_PARSER_VC_IMPL_H */

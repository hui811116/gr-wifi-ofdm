/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_PPDU_BUILDER_IMPL_H
#define INCLUDED_WIFI_OFDM_PPDU_BUILDER_IMPL_H

#include <gnuradio/wifi_ofdm/ppdu_builder.h>
#include <gnuradio/thread/thread.h>

namespace gr {
namespace wifi_ofdm {

#define MAX_CODE_BUFFER_BYTES 16384
#define MAX_SCRAMBLE_BUFFER_BYTES 8192
#define MAX_ENCODING_BITS_REGISTER_BYTES 64

class ppdu_builder_impl : public ppdu_builder
{
private:
    void build_data(const uint8_t* uvec, size_t nbyte);
    void service_and_scrambler(const uint8_t* uvec, int ndata, int nzero);
    void conv_enc();
    void puncturing();
    void interleaver();
    const pmt::pmt_t d_in_port;
    const pmt::pmt_t d_out_port;
    //std::mutex d_mutex;
    gr::thread::mutex d_mutex;
    //int d_rate;
    std::atomic_int d_rate;
    std::atomic_bool d_shutdown{false};
    int d_nbpsc;
    int d_ncbps;
    int d_ndbps;
    int d_nout;
    int d_nbits;
    //unsigned char d_code[8200];
    unsigned char d_code[MAX_CODE_BUFFER_BYTES];
    unsigned char d_scramble[MAX_SCRAMBLE_BUFFER_BYTES];
    unsigned char d_breg[MAX_ENCODING_BITS_REGISTER_BYTES];
    unsigned char d_seed;
    unsigned char d_scramble_reg;

public:
    ppdu_builder_impl(int seed, int rate);
    ~ppdu_builder_impl() {};

    // Where all the action really happens
    void build_ppdu(pmt::pmt_t msg);
    void set_ppdu_rate(int rate) override;
    int get_rate() const;
    bool stop() override;
    /*
    void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    int general_work(int noutput_items,
                     gr_vector_int& ninput_items,
                     gr_vector_const_void_star& input_items,
                     gr_vector_void_star& output_items);
    */
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_PPDU_BUILDER_IMPL_H */

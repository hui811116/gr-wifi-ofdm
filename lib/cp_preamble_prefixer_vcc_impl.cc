/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "cp_preamble_prefixer_vcc_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace wifi_ofdm {

static const int d_nfft = 64;
static const int d_ncp = 16;
static const int d_pulse_append = 16;

cp_preamble_prefixer_vcc::sptr cp_preamble_prefixer_vcc::make(const std::string& tagname)
{
    return gnuradio::make_block_sptr<cp_preamble_prefixer_vcc_impl>(tagname);
}


/*
 * The private constructor
 */
cp_preamble_prefixer_vcc_impl::cp_preamble_prefixer_vcc_impl(const std::string& tagname)
    : gr::tagged_stream_block(
          "cp_preamble_prefixer_vcc",
          gr::io_signature::make(1,1,sizeof(gr_complex) * d_nfft),
          gr::io_signature::make(1,1,sizeof(gr_complex)),tagname),
          d_tagname(pmt::intern(tagname)),
          d_block_name(pmt::intern(alias()))
{
    set_tag_propagation_policy(TPP_DONT);
}

/*
 * Our virtual destructor.
 */
cp_preamble_prefixer_vcc_impl::~cp_preamble_prefixer_vcc_impl() {}

int cp_preamble_prefixer_vcc_impl::calculate_output_stream_length(
    const gr_vector_int& ninput_items)
{
    // should be number of ofdm symbols
    int noutput_items = ninput_items[0] * SAMPLES_PER_SYMBOL + PREAMBLE_SAMPLES + d_pulse_append; 
    // additional sample for smoothing...
    return noutput_items;
}

int cp_preamble_prefixer_vcc_impl::work(int noutput_items,
                                     gr_vector_int& ninput_items,
                                     gr_vector_const_void_star& input_items,
                                     gr_vector_void_star& output_items)
{
    auto in = static_cast<const gr_complex*>(input_items[0]);
    auto out = static_cast<gr_complex*>(output_items[0]);

    // copy preamble to first position
    memcpy(out,d_preamble,sizeof(gr_complex)*PREAMBLE_SAMPLES);
    int nout = PREAMBLE_SAMPLES-1; // offset 1 sample for smoothing
    //int nsynCnt=0; // counting the ofdm symbols
    gr_complex smooth_scalar(0.5,0);
    for(int i=0;i<ninput_items[0];++i){
        out[nout] += smooth_scalar* in[i*d_nfft+d_nfft-d_ncp];
        memcpy(&out[nout+1],&in[(i*d_nfft+d_nfft-d_ncp+1)],sizeof(gr_complex)*(d_ncp-1));
        memcpy(&out[nout+d_ncp],&in[i*d_nfft],sizeof(gr_complex)*d_nfft);
        out[nout+d_nfft+d_ncp] = smooth_scalar * in[i*d_nfft+d_nfft-d_ncp];
        nout+= SAMPLES_PER_SYMBOL;
    }
    // manual appends additional samples for pushing out remainder samples in pulse shaping function?
    out[nout] += smooth_scalar * in[ninput_items[0]*d_nfft-d_pulse_append];
    memcpy(&out[nout+1], &in[(ninput_items[0])*d_nfft-d_pulse_append+1],sizeof(gr_complex)*(d_pulse_append-1));
    nout += d_pulse_append;
    // Tell runtime system how many output items we produced.
    return nout;
}

} /* namespace wifi_ofdm */
} /* namespace gr */

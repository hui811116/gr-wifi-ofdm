/* -*- c++ -*- */
/* 
 * Copyright 2018 <+YOU OR YOUR COMPANY+>.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/io_signature.h>
#include "cp_preamble_prefixer_vcc_impl.h"

namespace gr {
  namespace wifi_ofdm {
    static const int d_nfft = 64;
    static const int d_ncp = 16;
    static const int d_pulse_append = 16;

    cp_preamble_prefixer_vcc::sptr
    cp_preamble_prefixer_vcc::make(const std::string& tagname)
    {
      return gnuradio::get_initial_sptr
        (new cp_preamble_prefixer_vcc_impl(tagname));
    }

    /*
     * The private constructor
     */
    cp_preamble_prefixer_vcc_impl::cp_preamble_prefixer_vcc_impl(const std::string& tagname)
      : gr::tagged_stream_block("cp_preamble_prefixer_vcc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*d_nfft),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), tagname),
              d_tagname(pmt::intern(tagname))
    {
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    cp_preamble_prefixer_vcc_impl::~cp_preamble_prefixer_vcc_impl()
    {
    }

    int
    cp_preamble_prefixer_vcc_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      // should be number of ofdm symbols
      int noutput_items = ninput_items[0] * SAMPLES_PER_SYMBOL + PREAMBLE_SAMPLES + d_pulse_append; 

      // additional sample for smoothing...
      return noutput_items ;
    }

    int
    cp_preamble_prefixer_vcc_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      // copy preamble to first position
      memcpy(out,d_preamble,sizeof(gr_complex)*PREAMBLE_SAMPLES);
      noutput_items = PREAMBLE_SAMPLES;
      gr_complex smooth_scalar(0.5,0);
      for(int i=0;i<ninput_items[0];++i){
        out[noutput_items] += smooth_scalar*in[i*d_nfft+d_nfft-d_ncp];
        memcpy(&out[noutput_items+1],&in[i*d_nfft+d_nfft-d_ncp+1],sizeof(gr_complex)*(d_ncp-1));
        memcpy(&out[noutput_items+d_ncp],&in[i*d_nfft],sizeof(gr_complex)*(d_nfft-1));
        out[noutput_items+d_nfft] = smooth_scalar * in[i*d_nfft+d_nfft-d_ncp];
        noutput_items+= SAMPLES_PER_SYMBOL;
      }
      // manual appends additional samples for pushing out remainder samples in pulse shaping function?
      out[noutput_items] += smooth_scalar * in[(ninput_items[0])*d_nfft-d_ncp];
      memcpy(&out[noutput_items+1],&in[(ninput_items[0])*d_nfft-d_ncp+1],sizeof(gr_complex)*(d_ncp-1));
      noutput_items += d_pulse_append;
      // Tell runtime system how many output items we produced.
      return noutput_items;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


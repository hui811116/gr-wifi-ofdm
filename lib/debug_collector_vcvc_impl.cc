/* -*- c++ -*- */
/* 
 * Copyright 2018 Teng-Hui Huang.
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
#include "debug_collector_vcvc_impl.h"

namespace gr {
  namespace wifi_ofdm {
    static const int d_datacarr_idx[48] = {
      38,39,40,41,42,
      44,45,46,47,48,49,50,51,52,53,54,55,56,
      58,59,60,61,62,63,
      1,2,3,4,5,6,
      8,9,10,11,12,13,14,15,16,17,18,19,20,
      22,23,24,25,26,
    };
    debug_collector_vcvc::sptr
    debug_collector_vcvc::make(const std::string& tagname)
    {
      return gnuradio::get_initial_sptr
        (new debug_collector_vcvc_impl(tagname));
    }

    /*
     * The private constructor
     */
    debug_collector_vcvc_impl::debug_collector_vcvc_impl(const std::string& tagname)
      : gr::tagged_stream_block("debug_collector_vcvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)*64),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*48), tagname)
    {
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    debug_collector_vcvc_impl::~debug_collector_vcvc_impl()
    {
    }

    int
    debug_collector_vcvc_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      int noutput_items = ninput_items[0];
      return noutput_items ;
    }

    int
    debug_collector_vcvc_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {

      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];

      for(int i=0;i<ninput_items[0];++i){
        for(int j=0;j<48;++j)
          out[i*48 + j] = in[i*64 + d_datacarr_idx[j]];
      }
      add_item_tag(0,nitems_written(0),pmt::intern("hdr"),pmt::PMT_T,pmt::intern(alias()));
      // Tell runtime system how many output items we produced.
      return ninput_items[0];
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


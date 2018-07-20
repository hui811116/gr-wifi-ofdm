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
#include "symbol_parser_vc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace wifi_ofdm {
    static const int d_nfft = 64;
    static const int d_ndata = 52;
    static const pmt::pmt_t d_preTag = pmt::intern("long_pre");
    symbol_parser_vc::sptr
    symbol_parser_vc::make()
    {
      return gnuradio::get_initial_sptr
        (new symbol_parser_vc_impl());
    }

    /*
     * The private constructor
     */
    symbol_parser_vc_impl::symbol_parser_vc_impl()
      : gr::block("symbol_parser_vc",
              gr::io_signature::make(1, 1, sizeof(gr_complex) * d_nfft),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
    {
      d_channel_est = (gr_complex *) volk_malloc(sizeof(gr_complex)*d_nfft,volk_get_alignment());
      d_buf = (gr_complex *) volk_malloc(sizeof(gr_complex)*d_nfft,volk_get_alignment());
      d_pilot_idx = 0;
    }

    /*
     * Our virtual destructor.
     */
    symbol_parser_vc_impl::~symbol_parser_vc_impl()
    {
      volk_free(d_channel_est);
      volk_free(d_buf);
    }

    void
    symbol_parser_vc_impl::symbol_eq(gr_complex* out,const gr_complex* in, int pilot_idx)
    {
      // maybe consider MMSE

    }    

    void
    symbol_parser_vc_impl::channel_estimation(const gr_complex* in)
    {

    }

    void
    symbol_parser_vc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = noutput_items/d_nfft;
    }

    int
    symbol_parser_vc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex * out = (gr_complex*) output_items[0];
      int nin = std::min(ninput_items[0],noutput_items/d_nfft);
      int nout = 0;
      std::vector<tag_t> tags;
      get_tags_in_range(tags,0,nitems_read(0),nitems_read(0)+nin,d_preTag);
      if(!tags.empty()){
        uint64_t offset = tags[0].offset;
        if(offset == nitems_read(0)){
          // found a long preamble
          channel_estimation(&in[0]);
          d_pilot_idx = 0;
        }else{
          // 
          nin = (int)(tags[0].offset - nitems_read(0));
        }
      }
      for(int i=0;i<nin;++i){
        // 1. Feq
        // 2. pilot 
        // channel gain & carrier phase
        symbol_eq(&out[nout],&in[i],d_pilot_idx);
        d_pilot_idx++;
        d_pilot_idx %= 4;
        nout+= d_ndata;
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (nin);

      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


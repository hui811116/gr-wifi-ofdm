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

namespace gr {
  namespace wifi_ofdm {

    symbol_parser_vc::sptr
    symbol_parser_vc::make(int nfft)
    {
      return gnuradio::get_initial_sptr
        (new symbol_parser_vc_impl(nfft));
    }

    /*
     * The private constructor
     */
    symbol_parser_vc_impl::symbol_parser_vc_impl(int nfft)
      : gr::block("symbol_parser_vc",
              gr::io_signature::make(1, 1, sizeof(gr_complex) * nfft),
              gr::io_signature::make(0, 0, 0))
    {
      d_nfft = nfft;
    }

    /*
     * Our virtual destructor.
     */
    symbol_parser_vc_impl::~symbol_parser_vc_impl()
    {
    }

    void
    symbol_parser_vc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = (noutput_items/d_nfft) * d_nfft;
    }

    int
    symbol_parser_vc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      //<+OTYPE+> *out = (<+OTYPE+> *) output_items[0];
      int nin = noutput_items/d_nfft*d_nfft;
      int nout = 0;
      // Do <+signal processing+>
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (nin);

      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


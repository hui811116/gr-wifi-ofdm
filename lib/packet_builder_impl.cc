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
#include "packet_builder_impl.h"

#define WIFI_SIGNAL_LENGTH 48
static const gr_complex d_bpsk_map[2] = {gr_complex(-1,0),gr_complex(1,0)};
static const gr_complex d_qpsk_map[4] = {
  gr_complex(-sqrt(0.5),-sqrt(0.5)),
  gr_complex(sqrt(0.5),-sqrt(0.5)),
  gr_complex(-sqrt(0.5),sqrt(0.5)), 
  gr_complex(sqrt(0.5),sqrt(0.5))};
static const unsigned char testHdr[6] = {0x0F,0x1E,0x2D,0x3C,0x4B,0x5A};

namespace gr {
  namespace wifi_ofdm {

    packet_builder::sptr
    packet_builder::make(int rateType, const std::string& tagname)
    {
      return gnuradio::get_initial_sptr
        (new packet_builder_impl(rateType, tagname));
    }

    /*
     * The private constructor
     */
    packet_builder_impl::packet_builder_impl(int rateType, const std::string& tagname)
      : gr::tagged_stream_block("packet_builder",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)), tagname)
    {
      // Current version only support r=1/2
      switch(rateType){
        // BPSK for payload
        case 0:
          d_modRate = 1;
          d_copybits = 0x03;
          d_mapPtr = &d_bpsk_map[0];
        break;
        // QPSK for payload
        case 1:
          d_modRate = 2;
          d_copybits = 0x01;
          d_mapPtr = &d_qpsk_map[0];
        break;
        default:
          throw std::invalid_argument("Undefined datarate, please change...");
        break;
      }
      d_rateType = rateType;
    }

    /*
     * Our virtual destructor.
     */
    packet_builder_impl::~packet_builder_impl()
    {
    }

    int
    packet_builder_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      // input should be bytes, and depends on:
      // 1. data rate
      // 2. padded zeros
      int noutput_items;
      switch(d_rateType){
        // use BPSK for payload
        case 0:
          noutput_items = ninput_items[0]*8+WIFI_SIGNAL_LENGTH;
        break;
        // use QPSK for payload
        case 1:
          noutput_items = ninput_items[0]*8+WIFI_SIGNAL_LENGTH;
        break;
        default:
          throw std::runtime_error("Undefined data rate, abort");
        break;
      }
      return noutput_items ;
    }

    int
    packet_builder_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const unsigned char *in = (const unsigned char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      int byte_cnt=0,bits_shift=0;
      int nout = 0;
      d_shiftreg = 0x00;
      // TODO
      // step 1, add header (symbol level)
      for(int i=0;i<WIFI_SIGNAL_LENGTH;++i){
        int idx = (int)( (in[i/8] >> (i%8)) & 0x01);
        out[i] = d_bpsk_map[idx];
      }
      nout += WIFI_SIGNAL_LENGTH; 
      
      // step 2, map payload
      // make sure noutput_items is enough for adding another 48 BPSK symbols
      while(byte_cnt<ninput_items[0]){
        int idx = (int) ((in[byte_cnt] >> bits_shift) & d_copybits);
        bits_shift += d_modRate;
        byte_cnt += bits_shift/8;
        bits_shift %= 8;
        // map symbols from idx
        out[nout++] = d_mapPtr[idx];
      }

      // TODO
      // step 3, add tags, for convenience of the rest of the process.
      add_item_tag(0, nitems_written(0), pmt::intern("load_symbols"),pmt::from_long(nout-WIFI_SIGNAL_LENGTH));
      add_item_tag(0, nitems_written(0), pmt::intern("datarate"),pmt::intern("BPSK, Test"));
      // END
      return nout;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


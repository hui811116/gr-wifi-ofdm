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

#ifndef INCLUDED_WIFI_OFDM_PACKET_BUILDER_IMPL_H
#define INCLUDED_WIFI_OFDM_PACKET_BUILDER_IMPL_H

#include <wifi_ofdm/packet_builder.h>

/*
Current version only for rate=1/2
with the following modulations:
1. BPSK
2. QPSK
3. 16QAM

*/

namespace gr {
  namespace wifi_ofdm {

    class packet_builder_impl : public packet_builder
    {
     private:
      // Nothing to declare in this block.
      int d_rateType;
      int d_modRate;
      //int d_coderate;
      unsigned char d_shiftreg;
      unsigned char d_copybits;
      const gr_complex* d_mapPtr;

     protected:
      int calculate_output_stream_length(const gr_vector_int &ninput_items);

     public:
      packet_builder_impl(int rateType,const std::string& tagname);
      ~packet_builder_impl();

      // Where all the action really happens
      int work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_PACKET_BUILDER_IMPL_H */


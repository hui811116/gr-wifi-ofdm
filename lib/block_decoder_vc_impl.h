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

#ifndef INCLUDED_WIFI_OFDM_BLOCK_DECODER_VC_IMPL_H
#define INCLUDED_WIFI_OFDM_BLOCK_DECODER_VC_IMPL_H

#include <wifi_ofdm/block_decoder_vc.h>

namespace gr {
  namespace wifi_ofdm {
    static const unsigned char d_output_table[64][2]={
      {0,3},{2,1},{3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{1,2},{3,0},
      {2,1},{0,3},{2,1},{0,3},{1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},{1,2},{3,0},{3,0},{1,2},{0,3},{2,1},
      {0,3},{2,1},{3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},{2,1},{0,3},{1,2},{3,0},{1,2},{3,0},
      {2,1},{0,3},{2,1},{0,3},{1,2},{3,0},{1,2},{3,0},{2,1},{0,3}
    };
    static const unsigned int d_deint[48]={
      0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47
    };

    class block_decoder_vc_impl : public block_decoder_vc
    {
     private:
      uint8_t d_rate;
      uint16_t d_length;

      uint32_t d_hdr_reg;
      //uint64_t d_raw_hdr_reg;
      uint8_t d_coded_buf[8192];
      uint8_t d_hdr_debytes[6];
      void (block_decoder_vc_impl::*d_data_demod)(uint8_t* out, const gr_complex* in, int nin) const;
      void demod_BPSK(uint8_t* out, const gr_complex* in, int nin) const;
      void demod_QPSK(uint8_t* out, const gr_complex* in, int nin) const;
      void demod_QAM16(uint8_t* out, const gr_complex* in, int nin) const;
      void demod_QAM64(uint8_t* out, const gr_complex* in, int nin) const;
      bool decode_hdr(const gr_complex* in);
      unsigned int d_hdr_cost[24][64];
      unsigned char d_hdr_track[24][64];
     public:
      block_decoder_vc_impl();
      ~block_decoder_vc_impl();

      // Where all the action really happens
      int work(int noutput_items,
         gr_vector_const_void_star &input_items,
         gr_vector_void_star &output_items);
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_BLOCK_DECODER_VC_IMPL_H */


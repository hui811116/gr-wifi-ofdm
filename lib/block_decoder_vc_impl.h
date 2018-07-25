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
#include <random>
#include <chrono>

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
    static const float d_qam16_half[4] = {-3,3,-1,1};
    static const float d_qam64_half[8] = {-7,7,-1,1,-5,5,-3,3};
    // deinterleaving
    static const unsigned int d_deint48[48] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47};
    static const unsigned int d_deint96[96] = {0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 1, 7, 13, 19, 25, 31, 37, 43, 49, 55, 61, 67, 73, 79, 85, 91, 2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 3, 9, 15, 21, 27, 33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 4, 10, 16, 22, 28, 34, 40, 46, 52, 58, 64, 70, 76, 82, 88, 94, 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71, 77, 83, 89, 95};
    static const unsigned int d_deint192_2[192] = {0, 13, 24, 37, 48, 61, 72, 85, 96, 109, 120, 133, 144, 157, 168, 181, 1, 12, 25, 36, 49, 60, 73, 84, 97, 108, 121, 132, 145, 156, 169, 180, 2, 15, 26, 39, 50, 63, 74, 87, 98, 111, 122, 135, 146, 159, 170, 183, 3, 14, 27, 38, 51, 62, 75, 86, 99, 110, 123, 134, 147, 158, 171, 182, 4, 17, 28, 41, 52, 65, 76, 89, 100, 113, 124, 137, 148, 161, 172, 185, 5, 16, 29, 40, 53, 64, 77, 88, 101, 112, 125, 136, 149, 160, 173, 184, 6, 19, 30, 43, 54, 67, 78, 91, 102, 115, 126, 139, 150, 163, 174, 187, 7, 18, 31, 42, 55, 66, 79, 90, 103, 114, 127, 138, 151, 162, 175, 186, 8, 21, 32, 45, 56, 69, 80, 93, 104, 117, 128, 141, 152, 165, 176, 189, 9, 20, 33, 44, 57, 68, 81, 92, 105, 116, 129, 140, 153, 164, 177, 188, 10, 23, 34, 47, 58, 71, 82, 95, 106, 119, 130, 143, 154, 167, 178, 191, 11, 22, 35, 46, 59, 70, 83, 94, 107, 118, 131, 142, 155, 166, 179, 190};
    static const unsigned int d_deint288_2[288] = {0, 20, 37, 54, 74, 91, 108, 128, 145, 162, 182, 199, 216, 236, 253, 270, 1, 18, 38, 55, 72, 92, 109, 126, 146, 163, 180, 200, 217, 234, 254, 271, 2, 19, 36, 56, 73, 90, 110, 127, 144, 164, 181, 198, 218, 235, 252, 272, 3, 23, 40, 57, 77, 94, 111, 131, 148, 165, 185, 202, 219, 239, 256, 273, 4, 21, 41, 58, 75, 95, 112, 129, 149, 166, 183, 203, 220, 237, 257, 274, 5, 22, 39, 59, 76, 93, 113, 130, 147, 167, 184, 201, 221, 238, 255, 275, 6, 26, 43, 60, 80, 97, 114, 134, 151, 168, 188, 205, 222, 242, 259, 276, 7, 24, 44, 61, 78, 98, 115, 132, 152, 169, 186, 206, 223, 240, 260, 277, 8, 25, 42, 62, 79, 96, 116, 133, 150, 170, 187, 204, 224, 241, 258, 278, 9, 29, 46, 63, 83, 100, 117, 137, 154, 171, 191, 208, 225, 245, 262, 279, 10, 27, 47, 64, 81, 101, 118, 135, 155, 172, 189, 209, 226, 243, 263, 280, 11, 28, 45, 65, 82, 99, 119, 136, 153, 173, 190, 207, 227, 244, 261, 281, 12, 32, 49, 66, 86, 103, 120, 140, 157, 174, 194, 211, 228, 248, 265, 282, 13, 30, 50, 67, 84, 104, 121, 138, 158, 175, 192, 212, 229, 246, 266, 283, 14, 31, 48, 68, 85, 102, 122, 139, 156, 176, 193, 210, 230, 247, 264, 284, 15, 35, 52, 69, 89, 106, 123, 143, 160, 177, 197, 214, 231, 251, 268, 285, 16, 33, 53, 70, 87, 107, 124, 141, 161, 178, 195, 215, 232, 249, 269, 286, 17, 34, 51, 71, 88, 105, 125, 142, 159, 179, 196, 213, 233, 250, 267, 287};
    class block_decoder_vc_impl : public block_decoder_vc
    {
     private:
      const pmt::pmt_t d_out_port;
      pmt::pmt_t d_rate_key;
      uint8_t d_rate;
      uint16_t d_length;

      const unsigned int* d_deint_ptr;
      const unsigned char* d_depun_ptr;
      int d_ncbps;

      int d_sym_cnt;
      int d_nsymbol;
      int d_ndbits;
      int d_dbits_cnt;

      //uint32_t d_hdr_reg;
      uint8_t d_hdr_reg[3];
      uint8_t d_coded_buf[8192];
      uint8_t d_depun_buf[8192];
      uint8_t d_deint_buf[72];
      uint8_t d_hdr_debytes[6];
      void (block_decoder_vc_impl::*d_data_demod)(uint8_t* out, const gr_complex* in, int nin);
      void demod_BPSK(uint8_t* out, const gr_complex* in, int nin);
      void demod_QPSK(uint8_t* out, const gr_complex* in, int nin);
      void demod_QAM16(uint8_t* out, const gr_complex* in, int nin);
      void demod_QAM64(uint8_t* out, const gr_complex* in, int nin);
      bool decode_hdr(const gr_complex* in);
      void deint_depunc_and_pub();
      unsigned int d_hdr_cost[24][64];
      unsigned char d_hdr_track[24][64];

      std::mt19937 gen;
      std::bernoulli_distribution dist;

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


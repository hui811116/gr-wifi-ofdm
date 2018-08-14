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

#ifndef INCLUDED_WIFI_OFDM_SYMBOL_MAPPER_BVC_IMPL_H
#define INCLUDED_WIFI_OFDM_SYMBOL_MAPPER_BVC_IMPL_H

#include <wifi_ofdm/symbol_mapper_bvc.h>

namespace gr {
  namespace wifi_ofdm {

    static const gr_complex d_bpsk[2]={gr_complex(-1,0), gr_complex(1,0)};
    static const gr_complex d_qpsk[4]={gr_complex(-1,-1),gr_complex(1,-1),
                                       gr_complex(-1,1),gr_complex(1,1)};
    static const gr_complex d_qam16[16]={gr_complex(-3,-3),gr_complex(3,-3),gr_complex(-1,-3),gr_complex(1,-3),
                                         gr_complex(-3,3),gr_complex(3,3),gr_complex(-1,3),gr_complex(1,3),
                                         gr_complex(-3,-1),gr_complex(3,-1),gr_complex(-1,-1),gr_complex(1,-1),
                                         gr_complex(-3,1),gr_complex(3,1),gr_complex(-1,1),gr_complex(1,1)};

    static const gr_complex d_qam64[64]={gr_complex(-7,-7),gr_complex(7,-7),gr_complex(-1,-7),gr_complex(1,-7),gr_complex(-5,-7),gr_complex(5,-7),gr_complex(-3,-7),gr_complex(3,-7),
                                         gr_complex(-7,7),gr_complex(7,7),gr_complex(-1,7),gr_complex(1,7),gr_complex(-5,7),gr_complex(5,7),gr_complex(-3,7),gr_complex(3,7),
                                         gr_complex(-7,-1),gr_complex(7,-1),gr_complex(-1,-1),gr_complex(1,-1),gr_complex(-5,-1),gr_complex(5,-1),gr_complex(-3,-1),gr_complex(3,-1),
                                         gr_complex(-7,1),gr_complex(7,1),gr_complex(-1,1),gr_complex(1,1),gr_complex(-5,1),gr_complex(5,1),gr_complex(-3,1),gr_complex(3,1),
                                         gr_complex(-7,-5),gr_complex(7,-5),gr_complex(-1,-5),gr_complex(1,-5),gr_complex(-5,-5),gr_complex(5,-5),gr_complex(-3,-5),gr_complex(3,-5),
                                         gr_complex(-7,5),gr_complex(7,5),gr_complex(-1,5),gr_complex(1,5),gr_complex(-5,5),gr_complex(5,5),gr_complex(-3,5),gr_complex(3,5),
                                         gr_complex(-7,-3),gr_complex(7,-3),gr_complex(-1,-3),gr_complex(1,-3),gr_complex(-5,-3),gr_complex(5,-3),gr_complex(-3,-3),gr_complex(3,-3),
                                         gr_complex(-7,3),gr_complex(7,3),gr_complex(-1,3),gr_complex(1,3),gr_complex(-5,3),gr_complex(5,3),gr_complex(-3,3),gr_complex(3,3)};
    static const gr_complex d_pilot[4] = {
      gr_complex(1,0),gr_complex(1,0),gr_complex(1,0),gr_complex(-1,0)
    };
    static const float d_pilot_sign[127] = {
      1,1,1,1, -1,-1,-1,1, -1,-1,-1,-1, 1,1,-1,1, -1,-1,1,1, -1,1,1,-1, 1,1,1,1, 1,1,-1,1,
      1,1,-1,1, 1,-1,-1,1, 1,1,-1,1, -1,-1,-1,1, -1,1,-1,-1, 1,-1,-1,1, 1,1,1,1, -1,-1,1,1,
      -1,-1,1,-1, 1,-1,1,1, -1,-1,-1,1, 1,-1,-1,-1, -1,1,-1,-1, 1,-1,1,1, 1,1,-1,1, -1,1,-1,1,
      -1,-1,-1,-1, -1,1,-1,1, 1,-1,1,-1, 1,1,1,-1, -1,1,-1,-1, -1,1,1,1, -1,-1,-1,-1, -1,-1,-1
    };
    static const int d_subcarrier_type[64] = {
      0,0,0,0,0,0,1,1,1,1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,1,1,1,1,1,1,
      0,1,1,1,1,1,1,-1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,1,1,1,1,1,0,0,0,0,0
    };
    static const int d_subcarrier_idx[64] = {
      32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,
      0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31
    };
    class symbol_mapper_bvc_impl : public symbol_mapper_bvc
    {
     private:
      void configureRate(int rate);
      int mapRate(unsigned char raw) const;
      int d_rate;
      float d_norm;
      int d_psign_cnt;
      int d_psym_cnt;
      const pmt::pmt_t d_tagname;
      const pmt::pmt_t d_blockname;
      const gr_complex* d_mod_ptr;
      int d_bits_per_point;
      unsigned char d_breg;

     protected:
      int calculate_output_stream_length(const gr_vector_int &ninput_items);

     public:
      symbol_mapper_bvc_impl(const std::string& tagname);
      ~symbol_mapper_bvc_impl();

      // Where all the action really happens
      int work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_SYMBOL_MAPPER_BVC_IMPL_H */


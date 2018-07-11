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

#ifndef INCLUDED_WIFI_OFDM_SYMBOL_SYNC_CVC_IMPL_H
#define INCLUDED_WIFI_OFDM_SYMBOL_SYNC_CVC_IMPL_H

#include <wifi_ofdm/symbol_sync_cvc.h>

namespace gr {
  namespace wifi_ofdm {
    static const gr_complex d_long_pre[64] = {
      gr_complex(0.156250,0.000000), gr_complex(-0.005121,-0.120325), gr_complex(0.039750,-0.111158), gr_complex(0.096832,0.082798), gr_complex(0.021112,0.027886), gr_complex(0.059824,-0.087707), gr_complex(-0.115131,-0.055180), gr_complex(-0.038316,-0.106171), 
gr_complex(0.097541,-0.025888), gr_complex(0.053338,0.004076), gr_complex(0.000989,-0.115005), gr_complex(-0.136805,-0.047380), gr_complex(0.024476,-0.058532), gr_complex(0.058669,-0.014939), gr_complex(-0.022483,0.160657), gr_complex(0.119239,-0.004096), gr_complex(0.062500,-0.062500), gr_complex(0.036918,0.098344), 
gr_complex(-0.057206,0.039299), gr_complex(-0.131263,0.065227), gr_complex(0.082218,0.092357), gr_complex(0.069557,0.014122), gr_complex(-0.060310,0.081286), gr_complex(-0.056455,-0.021804), gr_complex(-0.035041,-0.150888), gr_complex(-0.121887,-0.016566), gr_complex(-0.127324,-0.020501), gr_complex(0.075074,-0.074040), 
gr_complex(-0.002806,0.053774), gr_complex(-0.091888,0.115129), gr_complex(0.091717,0.105872), gr_complex(0.012285,0.097600), gr_complex(-0.156250,0.000000), gr_complex(0.012285,-0.097600), gr_complex(0.091717,-0.105872), gr_complex(-0.091888,-0.115129), gr_complex(-0.002806,-0.053774), gr_complex(0.075074,0.074040), 
gr_complex(-0.127324,0.020501), gr_complex(-0.121887,0.016566), gr_complex(-0.035041,0.150888), gr_complex(-0.056455,0.021804), gr_complex(-0.060310,-0.081286), gr_complex(0.069557,-0.014122), gr_complex(0.082218,-0.092357), gr_complex(-0.131263,-0.065227), gr_complex(-0.057206,-0.039299), gr_complex(0.036918,-0.098344), 
gr_complex(0.062500,0.062500), gr_complex(0.119239,0.004096), gr_complex(-0.022483,-0.160657), gr_complex(0.058669,0.014939), gr_complex(0.024476,0.058532), gr_complex(-0.136805,0.047380), gr_complex(0.000989,0.115005), gr_complex(0.053338,-0.004076), gr_complex(0.097541,0.025888), gr_complex(-0.038316,0.106171), 
gr_complex(-0.115131,0.055180), gr_complex(0.059824,0.087707), gr_complex(0.021112,-0.027886), gr_complex(0.096832,-0.082798), gr_complex(0.039750,0.111158), gr_complex(-0.005121,0.120325)
    };
    class symbol_sync_cvc_impl : public symbol_sync_cvc
    {
     private:
      int d_state;
      int d_symbol_cnt;
      gr_complex d_first_long;
      gr_complex d_second_long;
      pmt::pmt_t d_bname;
      gr_complex* d_conj_long;
      gr_complex d_long_eng;

     public:
      symbol_sync_cvc_impl();
      ~symbol_sync_cvc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_SYMBOL_SYNC_CVC_IMPL_H */


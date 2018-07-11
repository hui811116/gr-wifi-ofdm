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

#ifndef INCLUDED_WIFI_OFDM_COARSE_CFO_FIXER_CC_IMPL_H
#define INCLUDED_WIFI_OFDM_COARSE_CFO_FIXER_CC_IMPL_H

#include <wifi_ofdm/coarse_cfo_fixer_cc.h>

namespace gr {
  namespace wifi_ofdm {

    class coarse_cfo_fixer_cc_impl : public coarse_cfo_fixer_cc
    {
     private:
      gr_complex * d_inXdelay;
      gr_complex * d_inSqu;
      gr_complex * d_delaySqu;
      int d_state;
      int d_nCnt;
      int d_delay;
      float d_coarse_cfo;
      float d_phase;
      pmt::pmt_t d_bname;

     public:
      coarse_cfo_fixer_cc_impl(int delay);
      ~coarse_cfo_fixer_cc_impl();

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_COARSE_CFO_FIXER_CC_IMPL_H */


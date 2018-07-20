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

#ifndef INCLUDED_WIFI_OFDM_SYMBOL_PARSER_VC_IMPL_H
#define INCLUDED_WIFI_OFDM_SYMBOL_PARSER_VC_IMPL_H

#include <wifi_ofdm/symbol_parser_vc.h>

namespace gr {
  namespace wifi_ofdm {

    class symbol_parser_vc_impl : public symbol_parser_vc
    {
     private:
      // Nothing to declare in this block.
      gr_complex * d_channel_est;
      gr_complex * d_buf;
      int d_pilot_idx;

     public:
      symbol_parser_vc_impl();
      ~symbol_parser_vc_impl();

      void symbol_eq(gr_complex* out, const gr_complex* in, int pilot_idx);
      void channel_estimation(const gr_complex* in);

      // Where all the action really happens
      void forecast (int noutput_items, gr_vector_int &ninput_items_required);

      int general_work(int noutput_items,
           gr_vector_int &ninput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_SYMBOL_PARSER_VC_IMPL_H */


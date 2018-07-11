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
#include "symbol_sync_cvc_impl.h"
#include <volk/volk.h>

namespace gr {
  namespace wifi_ofdm {
    static const int d_nfft = 64;
    symbol_sync_cvc::sptr
    symbol_sync_cvc::make()
    {
      return gnuradio::get_initial_sptr
        (new symbol_sync_cvc_impl());
    }

    /*
     * The private constructor
     */
    symbol_sync_cvc_impl::symbol_sync_cvc_impl()
      : gr::block("symbol_sync_cvc",
              gr::io_signature::make(1, 1, sizeof(gr_complex)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*d_nfft)),
              d_bname(pmt::intern(alias()))
    {
      d_state = 0;
      d_symbol_cnt =0;
      d_conj_long = (gr_complex*) volk_malloc(sizeof(gr_complex)*64,volk_get_alignment());
      volk_32fc_conjugate_32fc(d_conj_long,d_long_pre,64);
      volk_32fc_x2_conjugate_dot_prod_32fc(&d_long_eng,d_conj_long,d_conj_long,64);
    }

    /*
     * Our virtual destructor.
     */
    symbol_sync_cvc_impl::~symbol_sync_cvc_impl()
    {
      volk_free(d_conj_long);
    }

    void
    symbol_sync_cvc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = noutput_items;
    }

    int
    symbol_sync_cvc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      gr_complex eng_fir,eng_sec;
      gr_complex tmp_auto;
      int nout=0, nin, ncon=0;
      float first_cross, second_cross, fine_cfo;
      if(d_state==0){
        if(ninput_items[0]<128 || noutput_items<64){
          consume_each(0);
          return 0;
        }
        nin = ninput_items[0]-128;
        while(ncon<nin){
          volk_32fc_x2_dot_prod_32fc(&d_first_long,&in[ncon],d_conj_long,64);
          volk_32fc_x2_dot_prod_32fc(&d_second_long,&in[ncon+64],d_conj_long,64);
          volk_32fc_x2_conjugate_dot_prod_32fc(&eng_fir,&in[ncon],&in[ncon],64);
          volk_32fc_x2_conjugate_dot_prod_32fc(&eng_sec,&in[ncon+64],&in[ncon+64],64);
          first_cross = abs(d_first_long)/(abs(eng_fir*d_long_eng)+1e-7);
          second_cross = abs(d_second_long)/(abs(eng_sec*d_long_eng)+1e-7);
          if( first_cross > 0.9 && second_cross > 0.9){
            // sync of long preamble
            // fine-tune the first one for it contains no smoothed symbols
            // only use the first 32 samples
            volk_32fc_x2_conjugate_dot_prod_32fc(&tmp_auto, &in[ncon], &in[ncon+64], 32);
            fine_cfo = arg(tmp_auto)/(float)64;
            // change state
            d_state =1;
            add_item_tag(0,nitems_written(0),pmt::intern("symbol_idx"),pmt::from_long(d_symbol_cnt++),d_bname);
            add_item_tag(0,nitems_written(0),pmt::intern("cfo_est"),pmt::from_float(fine_cfo),d_bname);
            memcpy(out,&in[ncon],sizeof(gr_complex)*64);
            nout += 64;
            ncon += 128;
            break;
          }
          ncon++;
        }
      }else{
        if(ninput_items[0]<80 || noutput_items<64){
          consume_each(0);
          return 0;
        }
        nin = ninput_items[0]/80 * 80;
        while(nout < noutput_items/64*64 && ncon<nin){
          volk_32fc_x2_conjugate_dot_prod_32fc(&tmp_auto, &in[ncon], &in[ncon+64], 16);
          volk_32fc_x2_conjugate_dot_prod_32fc(&eng_fir,&in[ncon],&in[ncon],16);
          volk_32fc_x2_conjugate_dot_prod_32fc(&eng_sec,&in[ncon+64],&in[ncon+64],16);
          first_cross = abs(tmp_auto)/(abs(eng_fir*eng_sec)+1e-7);
          if(first_cross>0.9){
            // still sync
            fine_cfo = arg(tmp_auto)/(float)64;
            add_item_tag(0,nitems_written(0)+nout,pmt::intern("symbol_idx"),pmt::from_long(d_symbol_cnt++),d_bname);
            add_item_tag(0,nitems_written(0)+nout,pmt::intern("cfo_est"),pmt::from_float(fine_cfo),d_bname);
            memcpy(&out[nout],&in[ncon+16],sizeof(gr_complex)*64);
            ncon += 80;
            nout += 64;
          }else{
            // lose sync --> reset all --> change state
            d_symbol_cnt =0;
            d_state = 0;
            break;
          }
        }
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (ncon);

      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


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
#include <volk/volk.h>
#include <gnuradio/expj.h>

namespace gr {
  namespace wifi_ofdm {
    static const int d_nfft = 64;
    static const int d_ndata = 48;
    static const pmt::pmt_t d_preTag = pmt::intern("long_pre");
    symbol_parser_vc::sptr
    symbol_parser_vc::make()
    {
      return gnuradio::get_initial_sptr
        (new symbol_parser_vc_impl());
    }

    /*
     * The private constructor
     */
    symbol_parser_vc_impl::symbol_parser_vc_impl()
      : gr::block("symbol_parser_vc",
              gr::io_signature::make(1, 1, sizeof(gr_complex) * d_nfft),
              gr::io_signature::make(1, 1, sizeof(gr_complex) * d_ndata)),
              d_bname(pmt::intern(alias()))
    {
      d_channel_est = (gr_complex *) volk_malloc(sizeof(gr_complex)*d_nfft,volk_get_alignment());
      d_buf = (gr_complex *) volk_malloc(sizeof(gr_complex)*d_nfft,volk_get_alignment());
      d_pilot_idx = 0;
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    symbol_parser_vc_impl::~symbol_parser_vc_impl()
    {
      volk_free(d_channel_est);
      volk_free(d_buf);
    }

    void
    symbol_parser_vc_impl::symbol_eq(gr_complex* out,const gr_complex* in, int pilot_idx)
    {
      int nout = 0;
      // maybe consider MMSE
      for(int i=0;i<d_nfft;i++){
        d_buf[d_desubcarr_idx[i]] = in[i] * d_channel_est[i];
      }
      // estimate carrier phase
      // pilot subcarriers index: 11, 25, 39, 53
      gr_complex scalar=d_buf[11]*d_pilot[0];
      scalar+= d_buf[25]*d_pilot[1];
      scalar+= d_buf[39]*d_pilot[2];
      scalar+= d_buf[53]*d_pilot[3];
      float carrier_phase = (d_pilot_sign[pilot_idx]==1)? std::arg(scalar) : -std::arg(scalar);
      // FIXME:estimate attennuation
      for(int i=0;i<d_ndata;++i){
        if(d_subcarrier_type[i]==1)
          out[nout++] = gr_expj(-carrier_phase) * d_buf[i];
      }
    }    

    void
    symbol_parser_vc_impl::channel_estimation(const gr_complex* in)
    {
      gr_complex noise_pwr_est = gr_complex(1e-16,0);
      gr_complex mmse_tmp[64];

      // there are 12 null subcarriers
      for(int i=0;i<d_nfft;++i){
        if(d_subcarrier_type [d_desubcarr_idx[i]] == 0){
          noise_pwr_est += std::norm(in[i]);
        }else{
          // including pilots and data subcarriers
          // zero-forcing
          //d_channel_est[i] = d_long[d_desubcarr_idx[i]]/in[i];
          // MMSE
          mmse_tmp[i] = std::conj(d_long[d_desubcarr_idx[i]]) * in[i];
        }
      }
      noise_pwr_est/=gr_complex(12.0,0);
      // mmse normalization
      for(int i=0;i<d_nfft;++i){
        if(d_subcarrier_type[ d_desubcarr_idx[i]] != 0){
          d_channel_est[i] = (std::norm(in[i]) + noise_pwr_est) / mmse_tmp[i];
        }
      }
    }

    void
    symbol_parser_vc_impl::forecast (int noutput_items, gr_vector_int &ninput_items_required)
    {
      /* <+forecast+> e.g. ninput_items_required[0] = noutput_items */
      ninput_items_required[0] = noutput_items * d_ndata / d_nfft;
    }

    int
    symbol_parser_vc_impl::general_work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      gr_complex * out = (gr_complex*) output_items[0];
      int nin = std::min(ninput_items[0],noutput_items);
      int nout = 0;
      int ncon = 0;
      if(nin==0){
        consume_each(0);
        return 0;
      }
      
      std::vector<tag_t> tags;
      get_tags_in_range(tags,0,nitems_read(0),nitems_read(0)+nin,d_preTag);
      if(!tags.empty()){
        uint64_t offset = tags[0].offset;
        if(offset == nitems_read(0)){
          // found a long preamble
          channel_estimation(&in[0]);
          d_pilot_idx = 0;
          add_item_tag(0,nitems_written(0),pmt::intern("hdr"),pmt::PMT_T,d_bname);
          ncon++;
        }else{
          // 
          nin = (int)(tags[0].offset - nitems_read(0));
        }
      }
      for(;ncon<nin;++ncon){
        // 1. Feq
        // 2. pilot 
        // channel gain & carrier phase
        symbol_eq(&out[nout],&in[ncon*d_nfft],d_pilot_idx);
        // FIXME: find a stable way to fine tune CFO
        /*
        for(int j=0;j<d_ndata;++j)
          out[d_ndata*ncon+j] = in[ncon*d_nfft+d_datacarr_idx[j]];
        */
        add_item_tag(0,nitems_written(0)+nout/d_ndata,pmt::intern("symbol_idx"),pmt::from_long(d_pilot_idx),d_bname);
        d_pilot_idx++;
        d_pilot_idx %= 127;
        nout+= d_ndata;
      }
      // Tell runtime system how many input items we consumed on
      // each input stream.
      consume_each (ncon);

      // Tell runtime system how many output items we produced.
      return nout/d_ndata;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


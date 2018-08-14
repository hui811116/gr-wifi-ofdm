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
#include "symbol_mapper_bvc_impl.h"
#include <gnuradio/math.h>

namespace gr {
  namespace wifi_ofdm {
    static const int d_nsub=52;
    static const int d_nfft=64;
    static const float d_fft_norm = 1.0f/(float)64;

    symbol_mapper_bvc::sptr
    symbol_mapper_bvc::make(const std::string& tagname)
    {
      return gnuradio::get_initial_sptr
        (new symbol_mapper_bvc_impl(tagname));
    }

    /*
     * The private constructor
     */
    symbol_mapper_bvc_impl::symbol_mapper_bvc_impl(const std::string& tagname)
      : gr::tagged_stream_block("symbol_mapper_bvc",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)*d_nfft), tagname),
              d_blockname(pmt::intern(alias())),
              d_tagname(pmt::intern(tagname))
    {
      d_rate = 0;
      d_norm =1.0 * d_fft_norm;
      d_mod_ptr = d_bpsk;
      d_bits_per_point = 1;
      set_tag_propagation_policy(TPP_DONT);
    }

    /*
     * Our virtual destructor.
     */
    symbol_mapper_bvc_impl::~symbol_mapper_bvc_impl()
    {
    }
    void
    symbol_mapper_bvc_impl::configureRate(int rate)
    {
      if(rate == d_rate){
        return;
      }
      switch(rate){
        case 0:
          d_norm =1.0 * d_fft_norm;
          d_mod_ptr = d_bpsk;
          d_bits_per_point = 1;
        break;
        case 1:
          d_norm =1.0 * d_fft_norm;
          d_mod_ptr = d_bpsk;
          d_bits_per_point = 1;
        break;
        case 2:
          d_norm =1.0/sqrt(2) * d_fft_norm;
          d_mod_ptr = d_qpsk;
          d_bits_per_point = 2;
        break;
        case 3:
          d_norm =1.0/sqrt(2) * d_fft_norm;
          d_mod_ptr = d_qpsk;
          d_bits_per_point = 2;
        break;
        case 4:
          d_norm =1.0/sqrt(10) * d_fft_norm;
          d_mod_ptr = d_qam16;
          d_bits_per_point = 4;
        break;
        case 5:
          d_norm =1.0/sqrt(10) * d_fft_norm;
          d_mod_ptr = d_qam16;
          d_bits_per_point = 4;
        break;
        case 6:
          d_norm =1.0/sqrt(42) * d_fft_norm;
          d_mod_ptr = d_qam64;
          d_bits_per_point = 6;
        break;
        case 7:
          d_norm = 1.0/sqrt(42) * d_fft_norm;
          d_mod_ptr = d_qam64;
          d_bits_per_point = 6;
        break;
        default:
          throw std::invalid_argument("Undefined data rate, abort");
        break;
      }
      d_rate = rate;
    }
    int
    symbol_mapper_bvc_impl::mapRate(unsigned char raw) const
    {
      if(raw == 0x0B){
        // 6Mbps
        return 0;
      }else if(raw == 0x0F){
        // 9
        return 1;
      }else if(raw == 0x0A){
        // 12
        return 2;
      }else if(raw == 0x0E){
        // 18
        return 3;
      }else if(raw == 0x09){
        // 24
        return 4;
      }else if(raw == 0x0D){
        // 36
        return 5;
      }else if(raw == 0x08){
        // 48
        return 6;
      }else if(raw == 0x0C){
        // 54
        return 7;
      }else{
        throw std::runtime_error("Encounter an undefined rate tag, abort");
      }
    }
    int
    symbol_mapper_bvc_impl::calculate_output_stream_length(const gr_vector_int &ninput_items)
    {
      /*
      int noutput_items;
      switch(d_rate){
        case 0:
          noutput_items = 1 + (ninput_items[0]-7)/6;
        break;
        case 1:
          noutput_items = 1 + (ninput_items[0]-7)/6;
        break;
        case 2:
          noutput_items = 1 + (ninput_items[0]-7)/12;
        break;
        case 3:
          noutput_items = 1 + (ninput_items[0]-7)/12;
        break;
        case 4:
          noutput_items = 1 + (ninput_items[0]-7)/24;
        break;
        case 5:
          noutput_items = 1 + (ninput_items[0]-7)/24;
        break;
        case 6:
          noutput_items = 1 + (ninput_items[0]-7)/48;
        break;
        case 7:
          noutput_items = 1 + (ninput_items[0]-7)/48;
        break;
        default:
          // default assume 6Mbps
          noutput_items = 1 + (ninput_items[0]-7)/6;
          //throw std::runtime_error("Undefined data rate, abort");
        break;
      }*/
      int noutput_items = 1 + (ninput_items[0]-7)/6; // assume maximum size, 6Mbps
      return noutput_items ;
    }

    int
    symbol_mapper_bvc_impl::work (int noutput_items,
                       gr_vector_int &ninput_items,
                       gr_vector_const_void_star &input_items,
                       gr_vector_void_star &output_items)
    {
      const unsigned char *in = (const unsigned char *) input_items[0];
      gr_complex *out = (gr_complex *) output_items[0];
      // first 6 bytes represents the header, use 6Mbps rate for that part
      // NOTE: last byte is hidden rate tag, see ppdu_builder
      noutput_items = 1 + (ninput_items[0]*8-56)/d_bits_per_point/48;
      configureRate(mapRate(in[ninput_items[0]-1]));
      int bin=0, nout=0;
      d_psign_cnt=0;
      d_psym_cnt=0;
      // step 1: modulate HEADER bits
      while( nout < d_nfft){
        if(d_subcarrier_type[nout]==1){
          out[ d_subcarrier_idx[nout] ] = d_fft_norm * d_bpsk[ (in[bin/8]>>(bin%8)) & 0x01 ];
          bin++; // consume one bit
        }else if(d_subcarrier_type[nout]==-1){
          // insert one pilot before these subcarrier indices
          out[ d_subcarrier_idx[nout] ] = d_fft_norm * d_pilot_sign[d_psign_cnt] * d_pilot[d_psym_cnt];
          // add counter for next pilot symbol
          d_psym_cnt++;
        }else{
          out[ d_subcarrier_idx[nout] ] = gr_complex(0,0);
        }
        nout++;
      }// header and pilot insertion
      d_psign_cnt++; // move to next sign for other data symbols
      d_psym_cnt = 0; // reset pilot counter
      d_breg = 0x00; // reset bit collector
      while(nout < noutput_items*d_nfft){
        if(d_subcarrier_type[nout%d_nfft]==1){
          d_breg = 0x00;
          for(int biter=0;biter<d_bits_per_point;++biter){
            d_breg |= ( ( (in[bin/8] >> (bin%8)) & 0x01) << biter );
            bin++;
          }
          out[nout/d_nfft*d_nfft + d_subcarrier_idx[nout%d_nfft]] = d_norm * d_mod_ptr[d_breg];
        }else if(d_subcarrier_type[nout%d_nfft]==-1){
          out[ nout/d_nfft*d_nfft + d_subcarrier_idx[nout%d_nfft]] = d_fft_norm * d_pilot_sign[d_psign_cnt % 127] * d_pilot[d_psym_cnt++];
          if(d_psym_cnt==4){
            d_psym_cnt = 0;
            d_psign_cnt = (d_psign_cnt+1)%127;
          }
        }else{
          out[nout/d_nfft*d_nfft + d_subcarrier_idx[nout%d_nfft]] = gr_complex(0,0);
        }
        nout++;
      }
      return noutput_items;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


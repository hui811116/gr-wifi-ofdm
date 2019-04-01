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
#include "block_decoder_vc_impl.h"
#include <gnuradio/math.h>
#include <climits>
#include <cfloat>
#include <cstring>

namespace gr {
  namespace wifi_ofdm {
    #define d_debug 0
    #define dout d_debug && std::cout
    static const int d_ndata = 48;
    static const pmt::pmt_t d_hdr_tag = pmt::intern("hdr");
    enum RATESET{
      RATE6MBPS=0x0B,
      RATE9MBPS=0x0F,
      RATE12MBPS=0x0A,
      RATE18MBPS=0x0E,
      RATE24MBPS=0x09,
      RATE36MBPS=0x0D,
      RATE48MBPS=0x08,
      RATE54MBPS=0x0C
    };
    // for depuncturing
    static const unsigned char d_pun23[12] = {0,0,0,1,0,0,0,1,0,0,0,1};
    static const unsigned char d_pun34[18] = {0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0};
    static const float d_norms[2] = {sqrt(10.0),sqrt(42)};

    block_decoder_vc::sptr
    block_decoder_vc::make()
    {
      return gnuradio::get_initial_sptr
        (new block_decoder_vc_impl());
    }

    /*
     * The private constructor
     */
    block_decoder_vc_impl::block_decoder_vc_impl()
      : gr::sync_block("block_decoder_vc",
              gr::io_signature::make(1, 1, sizeof(gr_complex) * d_ndata),
              gr::io_signature::make(0, 0, 0)),
              d_out_port(pmt::mp("ppdu_out")),
              gen(std::chrono::system_clock::now().time_since_epoch().count()),
              dist(0.5)
    {
      d_nsymbol =0;
      d_sym_cnt =0;
      message_port_register_out(d_out_port);
    }

    /*
     * Our virtual destructor.
     */
    block_decoder_vc_impl::~block_decoder_vc_impl()
    {
    }

    void
    block_decoder_vc_impl::demod_BPSK(uint8_t* out, const gr_complex* in)
    {
      for(int i=0;i<48;++i){
        out[d_ncbits_cnt/8] |= (( (std::real(in[i])>0)?0x01:0x00 ) << (d_ncbits_cnt%8));
        d_ncbits_cnt++;
      }
    }

    void
    block_decoder_vc_impl::demod_QPSK(uint8_t* out, const gr_complex* in)
    {
      for(int i=0;i<48;++i){
        out[d_ncbits_cnt/8] |= (((std::real(in[i])>0)?0x01:0x00) << ( d_ncbits_cnt %8));
        out[(d_ncbits_cnt+1)/8] |= (((std::imag(in[i])>0)?0x01:0x00) << ((d_ncbits_cnt+1)%8) );
        d_ncbits_cnt+=2;
      }
    }

    void
    block_decoder_vc_impl::demod_QAM16(uint8_t* out, const gr_complex* in)
    {
      float i_min = FLT_MAX, q_min = FLT_MAX, i_tmp,q_tmp;
      uint8_t i_idx = 0, q_idx = 0;
      for(int i=0;i<48;++i){
        //std::cout<<"1("<<std::real(in[i])<<","<<std::imag(in[i])<<")"<<std::endl;
        for(uint8_t j=0;j<4;++j){
          // FIXME: contellation point normalization constant required
          i_tmp = std::real(d_norms[0]*in[i])-d_qam16_half[j];
          i_tmp *= i_tmp;
          q_tmp = std::imag(d_norms[0]*in[i])-d_qam16_half[j];
          q_tmp *= q_tmp;
          if(i_tmp<i_min){
            i_min = i_tmp; i_idx = j;
          }
          if(q_tmp<q_min){
            q_min = q_tmp; q_idx = j;
          }
        }
        // insert four bits
        out[d_ncbits_cnt/8] |= (i_idx << (d_ncbits_cnt%8));
        out[d_ncbits_cnt/8] |= (q_idx << ( (d_ncbits_cnt%8) + 2));
        d_ncbits_cnt += 4;
      }
    }

    void
    block_decoder_vc_impl::demod_QAM64(uint8_t* out, const gr_complex* in)
    {
      float i_min = FLT_MAX, q_min = FLT_MAX, i_tmp,q_tmp;
      uint8_t i_idx = 0, q_idx = 0;
      for(int i=0;i<48;++i){
        for(uint8_t j=0;j<8;++j){
          i_tmp = std::real(d_norms[1]*in[i])-d_qam64_half[j];
          i_tmp *= i_tmp;
          q_tmp = std::imag(d_norms[1]*in[i])-d_qam64_half[j];
          q_tmp *= q_tmp;
          if(i_tmp < i_min){
            i_min = i_tmp; i_idx = j;
          }
          if(q_tmp < q_min){
            q_min = q_tmp; q_idx = j;
          }
        }
        i_idx |= (q_idx << 3); // tmp holder
        for(int k=0;k<6;++k){
          out[d_ncbits_cnt/8] |= (((i_idx>>k)& 0x01) << (d_ncbits_cnt%8));
          d_ncbits_cnt++;
        }
      }
    }

    bool
    block_decoder_vc_impl::decode_hdr(const gr_complex* in)
    {
      std::memset(d_hdr_debytes,0,6);
      std::memset(d_hdr_reg,0,3);
      uint8_t tmp_reg2[6] = {0x00,0x00,0x00,0x00,0x00,0x00};
      for(int i=0;i<48;++i){
        tmp_reg2[i/8] |= (((std::real(in[i])>0)? 0x01: 0x00)<<(i%8));
      }
      for(int i=0;i<48;++i){
        int idx = d_deint[i];
        uint8_t tmpbit = (tmp_reg2[idx/8] >> (idx%8)) & 0x01;
        d_hdr_debytes[i/8] |= (tmpbit << (i%8));
      }
      for(int j=0;j<64;++j){
        d_hdr_cost[0][j] = UINT_MAX;
        d_hdr_track[0][j] = 0;
      }
      
      unsigned char cur=0x00, nex0=0x00,nex1=0x01;
      uint8_t out_mask = 0x03;
      unsigned char output = d_hdr_debytes[0] & out_mask;
      unsigned char tmpCmp0 = output ^ d_output_table[cur][0];
      unsigned char tmpCmp1 = output ^ d_output_table[cur][1];
      uint32_t costCnt0 = 0, costCnt1 =0;
      costCnt0 += (int) (tmpCmp0 & 0x01) + ((tmpCmp0>>1) & 0x01);
      costCnt1 += (int) (tmpCmp1 & 0x01) + ((tmpCmp1>>1) & 0x01);
      d_hdr_cost[0][cur] = costCnt0;
      d_hdr_cost[0][nex1] = costCnt1;
      d_hdr_track[0][cur] = 0;//dummy tracking
      d_hdr_track[0][nex1] = 0;
      for(int i=1;i<24;++i){
        output = (d_hdr_debytes[i*2/8] >> (i*2 % 8)) & out_mask;
        for(nex0=0;nex0<64;++nex0){
          if(i>=19 && (nex0 & 0x01))
            continue;
          // to nex0
          // first case, 0
          cur = (nex0>>1);
          tmpCmp0 = output ^ d_output_table[cur][nex0 & 0x01];
          costCnt0 = (d_hdr_cost[i-1][cur] == UINT_MAX)? UINT_MAX : d_hdr_cost[i-1][cur] + (uint32_t)((tmpCmp0 & 0x01) + ((tmpCmp0>>1) & 0x01));             
          // second case, 1
          nex1 = (nex0>>1) | 0x20;
          tmpCmp1 = output ^ d_output_table[nex1][nex0 & 0x01];
          costCnt1 = (d_hdr_cost[i-1][nex1] == UINT_MAX)? UINT_MAX : d_hdr_cost[i-1][nex1] + (uint32_t)((tmpCmp1 & 0x01) + ((tmpCmp1>>1) & 0x01));
          d_hdr_cost[i][nex0] = (costCnt0<costCnt1)? costCnt0 : costCnt1;
          d_hdr_track[i][nex0] = (costCnt0<costCnt1)? cur : nex1;
        }
          
      }// forward viterbi algorithm
      // back tracking part of viterbi
      // step 1: initialization
      if(d_hdr_cost[23][0]==UINT_MAX){
        // error, abort
        dout<<" ,min_Cost=UINT_MAX, abort"<<std::endl;
        return false;
      }
      cur = 0;
      // step 2: backward tracking
      for(int i=1;i<24;i++){
        nex0 = d_hdr_track[24-i][cur]; // tmp_holder
        d_hdr_reg[(24-i)/8] |= ( (cur&0x01) << ((24-i)%8) );
        cur = nex0;
      }
      // last bit
      d_hdr_reg[0] |= (cur & 0x01);
      uint8_t parity = 0x00;
      for(int i=0;i<18;++i){
        parity ^= ((d_hdr_reg[i/8]>>(i%8)) & 0x01);
      }
      if(parity == 0x00){
        dout<<"decoded hdr="<<std::hex<<(int)d_hdr_reg[0]<<","<<(int)d_hdr_reg[1]<<","<<(int)d_hdr_reg[2]<<std::dec<<std::endl;
        d_rate = d_hdr_reg[0] & 0x0f;
        d_length = 0x0000;
        for(int i=0;i<12;++i){
          d_length = (d_length<<1) | ((d_hdr_reg[(16-i)/8] >> ((16-i)%8)) & 0x0001);
        }
        d_ndbits = 16 + d_length * 8 + 6;
        switch(d_rate){
          case RATE6MBPS:
            dout<<", datarate:[ 6Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_BPSK;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)24 );
            d_ncbps = 48;
            d_deint_ptr = d_deint;
            d_rate_key = pmt::from_long(6);
            d_ncbits_len = d_ndbits*2;
          break;
          case RATE9MBPS:
            dout<<", datarate:[ 9Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_BPSK;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)36 );
            d_ncbps = 48;
            d_deint_ptr = d_deint;
            d_rate_key = pmt::from_long(9);
            d_depun_ptr = d_pun34;
            d_ncbits_len = ceil(d_ndbits*4/(float)3.0);
          break;
          case RATE12MBPS:
            dout<<", datarate:[ 12Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_QPSK;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)48 );
            d_ncbps = 96;
            d_deint_ptr = d_deint96;
            d_rate_key = pmt::from_long(12);
            d_ncbits_len = d_ndbits*2;
          break;
          case RATE18MBPS:
            dout<<", datarate:[ 18Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_QPSK;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)72 );
            d_ncbps = 96;
            d_deint_ptr = d_deint96;
            d_rate_key = pmt::from_long(18);
            d_depun_ptr = d_pun34;
            d_ncbits_len = ceil(d_ndbits*4/(float)3.0);
          break;
          case RATE24MBPS:
            dout<<", datarate:[ 24Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_QAM16;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)96 );
            d_ncbps = 192;
            d_deint_ptr = d_deint192_2;
            d_rate_key = pmt::from_long(24);
            d_ncbits_len = d_ndbits*2;
          break;
          case RATE36MBPS:
            dout<<", datarate:[ 36Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_QAM16;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)144 );
            d_ncbps = 192;
            d_deint_ptr = d_deint192_2;
            d_rate_key = pmt::from_long(36);
            d_depun_ptr = d_pun34;
            d_ncbits_len = ceil(d_ndbits*4/(float)3.0);
          break;
          case RATE48MBPS:
            dout<<", datarate:[ 48Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_QAM64;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)192 );
            d_ncbps = 288;
            d_deint_ptr = d_deint288_2;
            d_rate_key = pmt::from_long(48);
            d_depun_ptr = d_pun23;
            d_ncbits_len = ceil(d_ndbits*3/(float)2.0);
          break;
          case RATE54MBPS:
            dout<<", datarate:[ 54Mbps ], length="<<(int)d_length<<" bytes"<<std::endl;
            d_data_demod = &block_decoder_vc_impl::demod_QAM64;
            d_nsymbol = ceil( (16 + d_length * 8 + 6)/(float)216 );
            d_ncbps = 288;
            d_deint_ptr = d_deint288_2;
            d_rate_key = pmt::from_long(54);
            d_depun_ptr = d_pun34;
            d_ncbits_len = ceil(d_ndbits*4/(float)3.0);
          break;
          default:
            dout<<", undefined data rate, abort"<<std::endl;
            return false;
          break;
        }
        return true;
      }else{
        dout<<" , parity check failed, abort"<<std::endl;
        return false;
      }
    }

    void
    block_decoder_vc_impl::deint_depunc_and_pub()
    {
      int nout=0, ncon=0, nbytes = d_nsymbol * d_ncbps / 8;
      pmt::pmt_t blob; // output deinterleaved and depuntured bytes
      for(int i=0;i<d_nsymbol;++i){
        std::memset(d_deint_buf,0,36);
        int offset = i*d_ncbps/8;
        for(int j=0;j<d_ncbps;++j){
          int idx = d_deint_ptr[j];
          d_deint_buf[j/8] |= (((d_coded_buf[offset + idx/8 ] >> (idx%8)) & 0x01 ) << (j % 8));
        }
        // repaste to coded buf
        memcpy(&d_coded_buf[offset],d_deint_buf,sizeof(char)*(d_ncbps/8));
      }
      // puncturing
      if(d_rate == RATE6MBPS || d_rate == RATE12MBPS || d_rate == RATE24MBPS){
        // rate 1/2, do nothing
        blob = pmt::make_blob(d_coded_buf,2*d_length+6); // service=4 bytes, 6 padded zeros= 1.5 bytes
      }else{
        std::memset(d_depun_buf,0,nbytes);
        int punsize = (d_rate == RATE48MBPS)? 12 : 18;
        while(nout<d_ndbits){
          if(d_depun_ptr[ ncon % punsize] == 1){
            uint8_t rndbit = (dist(gen))? 0x01 : 0x00;
            d_depun_buf[nout/8] |= (rndbit << (nout%8));
          }else{
            d_depun_buf[nout/8] |= ( ((d_coded_buf[ncon/8] >> (ncon%8)) & 0x01 ) << (nout%8));
          }
          ncon++;
          nout++;
        }
        blob = pmt::make_blob(d_coded_buf,2*d_length+6);
      }
      message_port_pub(d_out_port,pmt::cons(d_rate_key,blob));
    }

    int
    block_decoder_vc_impl::work(int noutput_items,
        gr_vector_const_void_star &input_items,
        gr_vector_void_star &output_items)
    {
      const gr_complex *in = (const gr_complex *) input_items[0];
      int nout = noutput_items;
      
      std::vector<tag_t> tags;
      get_tags_in_window(tags,0,0,noutput_items,d_hdr_tag);
      if(!tags.empty()){
        if(tags[0].offset==nitems_read(0)){
          if(decode_hdr(&in[0])){
            // valid header
            std::memset(d_coded_buf,0,d_ndbits/4); //(d_ndbits * 2 / 8) , 133,171 coded, change to bytes form
            d_ncbits_cnt = 0;
            d_symbol_cnt = 0;
          }else{
            d_ndbits =0;
          }
          return 1;
        }else{
          nout = tags[0].offset-nitems_read(0);
        }
      }
      for(int i=0;i<nout;++i){
        if(d_ndbits>0){
          d_symbol_cnt++;
          (*this.*d_data_demod)(d_coded_buf,&in[d_ndata*i]);
          if(d_symbol_cnt == d_nsymbol){
            // all data bits collected
            // deinterleave
            deint_depunc_and_pub();
            // reset
            d_ndbits = 0;
            d_symbol_cnt = 0;
          }
        }
      }
      // Tell runtime system how many output items we produced.
      return nout;
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


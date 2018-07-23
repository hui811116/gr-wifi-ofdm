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
#include <wifi_ofdm/decode_header.h>
#include <gnuradio/block_detail.h>
#include <cstring>

namespace gr {
  namespace wifi_ofdm {
    #define d_debug 1
    #define dout d_debug && std::cout
  	static const unsigned int d_deint[48]={
  		0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47
  	};
    static constexpr unsigned char d_rateSet[8]= {
        0x0B, 0x0F, 0x0A, 0x0E, 0x09, 0x0D, 0x08, 0x0C
    };
  	static const unsigned char d_output_table[64][2]={
        {0,3},{2,1},{3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},
        {3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{1,2},{3,0},{2,1},{0,3},
        {2,1},{0,3},{1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},
        {1,2},{3,0},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},
        {3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},{2,1},{0,3},
        {1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},{1,2},{3,0},
        {1,2},{3,0},{2,1},{0,3}
    };
    
    class decode_header_impl : public decode_header
    {
    public:
    	decode_header_impl():block("decode_header",
    		gr::io_signature::make(0,0,0),
    		gr::io_signature::make(0,0,0)),
    		d_in_port(pmt::mp("hdr_in")),
    		d_out_port(pmt::mp("trigger_out"))
    	{
    		message_port_register_out(d_out_port);
    		message_port_register_in(d_in_port);
    		set_msg_handler(d_in_port,boost::bind(&decode_header_impl::msg_in,this,_1));
    	}
    	~decode_header_impl(){}
    	void msg_in(pmt::pmt_t msg)
    	{
    		pmt::pmt_t k = pmt::car(msg);
    		pmt::pmt_t v = pmt::cdr(msg);
    		size_t io(0);
    		const uint8_t* uvec = pmt::u8vector_elements(v,io);
    		// first, do deinterleaving
    		deinterleave(uvec);
            // initialization
            for(int i=0;i<24;++i){
                for(int j=0;j<64;++j){
                    d_cost[i][j] = 1023;
                }
            }
            std::memset(d_uncode,0,3);
    		if(!conv_dec()){
                // decoding failure
                dout<<"DEBUG: decode_header--conv dec failure"<<std::endl;
                return;
            }

            // first checking parity check
            // in d_uncode
            unsigned char pcnt = d_uncode[0] ^ d_uncode[1];
            unsigned char parity=0x00;
            unsigned char rate;
            unsigned char length[2];
            for(int i=0;i<8;++i)
                parity ^= ((pcnt>>i) & 0x01);
            parity ^= (d_uncode[2] & 0x01);
            if( (parity ^ ((d_uncode[2]>>1) & 0x01)) == 0x00 ){
                // check sum passed
                rate = d_uncode[0] & 0x0f;
                length[0] = ((d_uncode[0]>>5) & 0x07) | ((d_uncode[1]<<3) & 0xf8);
                length[1] = ((d_uncode[1]>>5) & 0x07) | ((d_uncode[2]<<3) & 0x08);
                int len_sum = (int)length[0] + 256*(int)length[1];
                switch(rate){
                    case d_rateSet[0]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 6Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[1]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 9Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[2]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 12Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[3]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 18Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[4]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 24Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[5]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 36Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[6]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 48Mbps ]"<<std::endl;
                    break;
                    case d_rateSet[7]:
                        dout<<"DEBUG-decode_header: decoded data rate [ 54Mbps ]"<<std::endl;
                    break;
                    default:
                        dout<<"DEBUG-decode_header: undefined rate, declare failure"<<std::endl;
                        return;
                    break;
                }
                dout<<"DEBUG-decode_header: decoded PSDU length[ "<<len_sum<<" ]bytes"<<std::endl;
            }else{
                // check sum failed 
                dout<<"DEBUG-decode_header: decode failure--parity check failed"<<std::endl;
            }
    	}
    private:
    	void deinterleave(const uint8_t* uvec)
    	{
    		int idx;
    		uint8_t tmpbit;
    		std::memset(d_code,0,8);
    		for(int i=0;i<48;++i){
    			idx = d_deint[i];
    			tmpbit = (uvec[idx/8]>>(idx%8)) & 0x01;
    			d_code[i/8] |= ( tmpbit << (i % 8) );
    		}
    	}
    	bool conv_dec()
    	{
    		unsigned char cur=0x00, nex0=0x00,nex1=0x01;
    		unsigned char out_mask = 0x03;
    		unsigned char output = d_code[0] & out_mask;
    		unsigned char tmpCmp0 = output ^ d_output_table[cur][0];
    		unsigned char tmpCmp1 = output ^ d_output_table[cur][1];
    		int costCnt0 = 0, costCnt1 =0;
    		costCnt0 += (int) (tmpCmp0 & 0x01) + ((tmpCmp0>>1) & 0x01);
    		costCnt1 += (int) (tmpCmp1 & 0x01) + ((tmpCmp1>>1) & 0x01);
    		d_cost[0][cur] = costCnt0;
            d_cost[0][nex1] = costCnt1;
    		d_track[0][cur] = 0;//dummy tracking
            d_track[0][nex1] = 0;
    		for(int i=1;i<24;++i){
                output = (d_code[i*2/8] >> (i*2 %8)) & out_mask;
    				for(nex0=0;nex0<64;++nex0){
                        if(i>=19 && (nex0 & 0x01))
                            continue;
    					
    					// to nex0
    					// first case, 0
    					cur = (nex0>>1);
    					tmpCmp0 = output ^ d_output_table[cur][nex0 & 0x01];
    					costCnt0 = (d_cost[i-1][cur]==1023)? 1023 : d_cost[i-1][cur] + (int)((tmpCmp0 & 0x01) + ((tmpCmp0>>1) & 0x01));    					
    					// second case, 1
    					nex1 = (nex0>>1) | 0x20;
    					tmpCmp1 = output ^ d_output_table[nex1][nex0 & 0x01];
    					costCnt1 = (d_cost[i-1][nex1]==1023)? 1023 : d_cost[i-1][nex1] + (int)((tmpCmp1 & 0x01) + ((tmpCmp1>>1) & 0x01));
    					d_cost[i][nex0] = (costCnt0<costCnt1)? costCnt0 : costCnt1;
    					d_track[i][nex0] = (costCnt0<costCnt1)? cur : nex1;
    				}
    			
    		}// forward viterbi algorithm
            // back tracking part of viterbi
            // step 1: initialization
            if(d_cost[23][0]==1023){
                // error, abort
                return false;
            }
            cur = d_track[23][0];
            nex0 = 0;
            std::memset(d_uncode,0,4);
            d_uncode[2] |= ( ((nex0 ^ (cur<<1) )&0x01) << 7);
            // step 2: backward tracking
            for(int i=1;i<24;i++){
                nex0 = cur; // tmp_holder
                cur = d_track[23-i][nex0];
                d_uncode[(23-i)/8] |= ( ((nex0 ^ (cur<<1) )&0x01) << ((23-i)%8) );
            }
            return true;
    	}
    	pmt::pmt_t d_in_port;
    	pmt::pmt_t d_out_port;
    	unsigned char d_code[8];
        unsigned char d_uncode[4];
    	unsigned int d_cost[24][64];
    	unsigned char d_track[24][64];
    };

    decode_header::sptr
    decode_header::make()
    {
    	return gnuradio::get_initial_sptr(new decode_header_impl());
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


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
#include <wifi_ofdm/ppdu_sink.h>
#include <gnuradio/block_detail.h>
#include <cstring>
#include <climits>

namespace gr {
  namespace wifi_ofdm {
    #define d_debug 0
    #define dout d_debug && std::cout
  	static const unsigned char d_output_table[64][2]={
        {0,3},{2,1},{3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},
        {3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{1,2},{3,0},{2,1},{0,3},
        {2,1},{0,3},{1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},
        {1,2},{3,0},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},
        {3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},{2,1},{0,3},
        {1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},{1,2},{3,0},
        {1,2},{3,0},{2,1},{0,3}
    };

    class ppdu_sink_impl : public ppdu_sink
    {
    public:
    	ppdu_sink_impl():block("ppdu_sink",
    		gr::io_signature::make(0,0,0),
    		gr::io_signature::make(0,0,0)),
    		d_in_port(pmt::mp("ppdu_in")),
    		d_out_port(pmt::mp("msg_out"))
    	{
    		message_port_register_out(d_out_port);
    		message_port_register_in(d_in_port);
    		set_msg_handler(d_in_port,boost::bind(&ppdu_sink_impl::msg_in,this,_1));
    	}
    	~ppdu_sink_impl(){}
    	void msg_in(pmt::pmt_t msg)
    	{
    		pmt::pmt_t k = pmt::car(msg); // contain rate tag
    		pmt::pmt_t v = pmt::cdr(msg);
    		// the message bytes are deinterleaved bytes
    		d_rate = pmt::to_long(k);
    		size_t io(0);
    		const uint8_t* uvec = pmt::u8vector_elements(v,io);
    		// conv_dec
    		if(conv_dec(uvec,io)){
    			// finish decoding
          /*
    			dout<<"decoded output:"<<std::endl;
          for(int i=0;i<io/2-1;++i)
            dout<<" "<<(int)d_decode[i];
          dout<<std::endl;
          */
    			// check service field and seed
    			service_and_seed();
          //dout<<"estimated seed="<<(int)d_seed_est<<std::endl;
    			// next step, descramble decoded bits
    			descramble(io*4-2-6);
    			// publish payload
    			pmt::pmt_t blob = pmt::make_blob(d_decode+2, io/2-3);
    			message_port_pub(d_out_port,pmt::cons(pmt::PMT_NIL,blob));
    		}else{
    			// decoding failure
    			return;
    		}
    	}

    private:
    	void descramble(int size)
    	{
    		uint8_t buf = 0x00, tmpbit, d_state = d_seed_est;
    		for(int i=0;i<size;++i){
    			tmpbit = ((d_state>>3) ^ (d_state >>6)) & 0x01;
    			buf |= (((tmpbit ^ (d_decode[i/8]>>(i%8) ) ) & 0x01) << (i%8));
    			// update state
    			d_state = ( (d_state << 1) | tmpbit) & 0x7F;
    			if( ((i+1) % 8) == 0){
    				d_decode[i/8] = buf;
            buf = 0x00;
          }
    		}
    	}
    	void service_and_seed()
    	{
    		d_seed_est = 0x00;
    		d_seed_est |= (((d_decode[0]>>2)  ^ (d_decode[0]>>6)) & 0x01);
    		d_seed_est |= ((((d_decode[0]>>1) ^ (d_decode[0]>>5)) & 0x01) << 1);
    		d_seed_est |= ((((d_decode[0])    ^ (d_decode[0]>>4)) & 0x01) << 2);
    		d_seed_est |= ((((d_decode[0]>>2) ^ (d_decode[0]>>6) ^ (d_decode[0]>>3)) & 0x01) << 3);
    		d_seed_est |= ((((d_decode[0]>>1) ^ (d_decode[0]>>5) ^ (d_decode[0]>>2)) & 0x01) << 4);
    		d_seed_est |= ((((d_decode[0]>>0) ^ (d_decode[0]>>4) ^ (d_decode[0]>>1)) & 0x01) << 5);
    		d_seed_est |= ((((d_decode[0]>>2) ^ (d_decode[0]>>6) ^ (d_decode[0]>>3) ^ d_decode[0]) & 0x01) << 6);
    		// service
    		// d_decode[0], d_decode[1]
    	}
    	bool conv_dec(const uint8_t* uvec, size_t noutputs)
    	{
    		const int nbit = noutputs *4 - 2; // last byte has only 6 zeros
    		int nwrite = 0; // data bits that are decoded
    		int ncnt = 0;
    		std::memset(d_decode,0,noutputs/2);
    		for(int i=0;i<64;++i){
    			d_cost[0][i] = UINT_MAX;
    			d_track[0][i] = 0;
    		}
        //dout<<"nbit = "<<nbit<<std::endl;
    		unsigned char cur=0x00, nex0=0x00,nex1=0x01;
    		const uint8_t out_mask = 0x03;
    		unsigned char output = uvec[0] & out_mask;
      	unsigned char tmpCmp0 = output ^ d_output_table[cur][0];
      	unsigned char tmpCmp1 = output ^ d_output_table[cur][1];
      	uint32_t costCnt0 = 0, costCnt1 =0;
      	while(ncnt<nbit){
      			if(ncnt%1024 ==0){
      				// initialization and update
      				output = (uvec[ncnt*2/8] >> (2*ncnt % 8)) & out_mask;
      				tmpCmp0 = output ^ d_output_table[cur][0];
      				tmpCmp1 = output ^ d_output_table[cur][1];
      				costCnt0 = (int) (tmpCmp0 & 0x01) + ((tmpCmp0>>1) & 0x01);
      				costCnt1 = (int) (tmpCmp1 & 0x01) + ((tmpCmp1>>1) & 0x01);
      				nex0 = cur<<1;
      				nex1 = (cur<<1) | 0x01;
      				d_cost[0][nex0] = costCnt0;
      				d_cost[0][nex1] = (ncnt>nbit-6)? UINT_MAX : costCnt1;
      				d_track[0][nex0] = cur;
      				d_track[0][nex1] = cur;
      				ncnt++;
      				if(ncnt == nbit){
      					break;
      				}
      			}
      			output = (uvec[ncnt*2/8] >> (2*ncnt % 8)) & out_mask;
            //dout<<"iteration="<<ncnt<<std::endl;
            //dout<<"costs:";
      			for(nex0=0;nex0<64;++nex0){
      				if( (ncnt> nbit-6) && (nex0 & 0x01) ){
                d_cost[ncnt][nex0] = UINT_MAX;
      					continue;
              }
      				cur = (nex0 >> 1);
      				tmpCmp0 = output ^ d_output_table[cur][nex0 & 0x01];
      				costCnt0 = (d_cost[ncnt%1024-1][cur] == UINT_MAX)? UINT_MAX : d_cost[ncnt%1024-1][cur] + (uint32_t)((tmpCmp0 & 0x01) + ((tmpCmp0>>1) & 0x01));             
          			// second case, 1
          			nex1 = (nex0>>1) | 0x20;
          			tmpCmp1 = output ^ d_output_table[nex1][nex0 & 0x01];
          			costCnt1 = (d_cost[ncnt%1024-1][nex1] == UINT_MAX)? UINT_MAX : d_cost[ncnt%1024-1][nex1] + (uint32_t)((tmpCmp1 & 0x01) + ((tmpCmp1>>1) & 0x01));
          			d_cost[ncnt][nex0] = (costCnt0<costCnt1)? costCnt0 : costCnt1;
          			d_track[ncnt][nex0] = (costCnt0<costCnt1)? cur : nex1;
                //dout<<" "<<d_cost[ncnt][nex0];
      			}
            //dout<<std::endl;
      			ncnt++;
      			if(ncnt%1024 == 0){
      				// truncated 
      				uint32_t minCost = d_cost[1023][0];
      				uint8_t minIdx = 0;
      				for(int i=1;i<64;++i){
      					if(d_cost[1023][i]<minCost){
      						minCost = d_cost[1023][i];
      						minIdx = i;
      					}
      				}
      				if(minCost == UINT_MAX)
      					return false;
      				// back tracking
      				cur = minIdx;
      				for(int i=1;i<1024;++i){
      					nex0 = d_track[1024-i][cur];
      					d_decode[(nwrite+1024-i)/8] |= ((cur&0x01) <<(nwrite+1024-i)%8);
      					cur = nex0;
      				}
      				// last one
      				d_decode[nwrite/8] |= ((cur&0x01) << (nwrite%8));
      				cur = minIdx;
      				nwrite += 1024;
      			} // if
      	}// while
      	// last tracking
      	cur = 0;
      	for(int i=1;i< (ncnt%1024) ;++i){
      		nex0 = d_track[ncnt%1024-i][cur];
      		d_decode[(ncnt-i)/8] |= ((cur&0x01) << ((ncnt-i)%8) );
      		cur = nex0;
      	}
      	// last one
      	d_decode[nwrite/8] |= ((cur & 0x01) << (nwrite%8));
      	nwrite += (ncnt%1024);
      	return true;
    	}
    	
    	const pmt::pmt_t d_in_port;
    	const pmt::pmt_t d_out_port;
    	int d_rate;
    	unsigned char d_decode[4096];
    	unsigned int d_cost[1024][64];
    	unsigned char d_track[1024][64];
    	uint8_t d_seed_est;
    };

    ppdu_sink::sptr
    ppdu_sink::make()
    {
    	return gnuradio::get_initial_sptr(new ppdu_sink_impl());
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


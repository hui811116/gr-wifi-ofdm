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
#include <wifi_ofdm/header_prefixer.h>
#include <gnuradio/block_detail.h>
#include <cstring>

namespace gr {
  namespace wifi_ofdm {
    #define d_debug 0
    #define dout d_debug && std::cout
    #define WIFI80211A_HEADER_BYTES 6
  	static const unsigned char d_rateSet[8]= {
  		0x0B, 0x0F, 0x0A, 0x0E, 0x09, 0x0D, 0x08, 0x0C
  	};
    static const size_t d_inter[48] = {
        0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47
    };
    class header_prefixer_impl : public header_prefixer
    {
    public:
    	header_prefixer_impl(int rate): block("header_prefixer",
    		gr::io_signature::make(0,0,0),
    		gr::io_signature::make(0,0,0)),
    		d_in_port(pmt::mp("psdu_in")),
    		d_out_port(pmt::mp("ppdu_out"))
    	{
    		message_port_register_in(d_in_port);
    		message_port_register_out(d_out_port);
    		set_msg_handler(d_in_port,boost::bind(&header_prefixer_impl::msg_in,this,_1));
    		// check rate
    		switch(rate)
    		{
    			case 0://6Mbps
                    d_rate_k = pmt::intern("6Mbps");
    			break;
    			case 1://9Mbps
                    d_rate_k = pmt::intern("9Mbps");
    			break;
    			case 2://12Mbps
                    d_rate_k = pmt::intern("12Mbps");
    			break;
    			case 3://18Mbps
                    d_rate_k = pmt::intern("18Mbps");
    			break;
    			case 4://24Mbps
                    d_rate_k = pmt::intern("24Mbps");
    			break;
    			case 5://36Mbps
                    d_rate_k = pmt::intern("36Mbps");
    			break;
    			case 6://48Mbps
                    d_rate_k = pmt::intern("48Mbps");
    			break;
    			case 7://54Mbps
                    d_rate_k = pmt::intern("54Mbps");
    			break;
    			default:
    				throw std::invalid_argument("Undefined data rate, aborted");
    			break;
    		}
    		d_rate = rate;
    	}
    	~header_prefixer_impl(){}

    	void msg_in(pmt::pmt_t msg)
    	{
    		pmt::pmt_t k = pmt::car(msg);
    		pmt::pmt_t v = pmt::cdr(msg);
    		size_t io(0);
    		const uint8_t* uvec = pmt::u8vector_elements(v,io);
    		//assert(io<4096); // 2**12
    		d_u16len = (uint16_t) io;
    		d_u8ptr = (uint8_t*) & d_u16len;
    		d_hdr_buf[0] = d_rateSet[d_rate];
    		// first 4 bits are rate, which is already set
    		// the 5th bits is reserved and set to zero
    		d_hdr_buf[0] |= (d_u8ptr[0] << 5);
    		d_hdr_buf[1] = (d_u8ptr[0] >> 3);
    		d_hdr_buf[1] |= (d_u8ptr[1] << 5);
    		d_hdr_buf[2] = ((d_u8ptr[1] >> 3) & 0x01);
    		unsigned char bsum = d_hdr_buf[0] ^ d_hdr_buf[1] ^ d_hdr_buf[2];
    		unsigned char parity = 0x00;
    		for(int i=0;i<8;++i){
    			parity ^= ((bsum >> i) & 0x01);
    		}
    		d_hdr_buf[2] |= (parity<<1);//parity check moved to 18th bit
            dout<<"DEBUG-header_prefixer: uncoded header..."<<std::endl;
            for(int i=0;i<24;++i)
                dout<<" "<<(int) ((d_hdr_buf[i/8] >> (i%8))&0x01);
            dout<<std::endl;
            //dout<<(int)d_hdr_buf[0]<<", "<<(int)d_hdr_buf[1]<<", "<<(int)d_hdr_buf[2]<<std::endl;
    		// the rest 6 bits should be zeros
    		// performing 133,171 conv coding
    		conv_enc(d_hdr_buf); // store in d_enc
            dout<<"DEBUG-header_prefixer: coded bits..."<<std::endl;
            for(int i=0;i<48;++i)
                dout<<" "<<(int) ((d_enc[i/8] >> (i%8))&0x01);
            dout<<std::endl;
    		// do interleaving
    		interleaver();
            // d_out contains the produced header, 6 bytes
            std::memcpy(d_copy+WIFI80211A_HEADER_BYTES, uvec, sizeof(char) * io);
            pmt::pmt_t blob = pmt::make_blob(d_copy,io+WIFI80211A_HEADER_BYTES);
            message_port_pub(d_out_port,pmt::cons(d_rate_k,blob));
    	}

    private:
    	// standard 133,171 conv_encoder
    	void conv_enc(const uint8_t* uvec)
    	{
    		uint8_t enc_reg = 0x00, tmp_lsb = 0x00;
    		uint8_t tmp_bit[2];
    		std::memset(d_enc, 0, sizeof d_enc);
    		for(int i=0;i < WIFI80211A_HEADER_BYTES*8/2 ;++i){
    			tmp_lsb = (uvec[i/8] >> (i%8)) & 0x01;
    			tmp_bit[0] = (((tmp_lsb) ^ (enc_reg >> 1) ^ (enc_reg >> 2)
    					     ^ (enc_reg >> 4) ^ (enc_reg >> 5)) & 0x01); // first output
    			tmp_bit[1] = (((tmp_lsb) ^ enc_reg ^ (enc_reg >> 1) ^ (enc_reg >> 2)
    						 ^ (enc_reg >> 5)) & 0x01); // second output
                d_enc[i/4] |= (tmp_bit[0]<< ( (2*i)  % 8));
                d_enc[i/4] |= tmp_bit[1]<< ((2*i+1) % 8);
                enc_reg = ((enc_reg<<1) | tmp_lsb) & 0x3f;
    		}
    	}
    	void interleaver()
    	{
            std::memset(d_out, 0, sizeof d_out);
            for(int i=0;i<WIFI80211A_HEADER_BYTES*8;++i){
                int newPos = d_inter[i];
                d_out[newPos/8] |= (  ((d_enc[i/8] >> (i%8)) & 0x01) << (newPos%8) );
            }
            std::memcpy(d_copy,d_out,sizeof(char)*WIFI80211A_HEADER_BYTES);
    	}
    	const pmt::pmt_t d_in_port;
    	const pmt::pmt_t d_out_port;
        pmt::pmt_t d_rate_k;
    	int d_rate;
    	uint16_t d_u16len;
    	uint8_t* d_u8ptr;
    	unsigned char d_hdr_buf[8];
    	unsigned char d_enc[8];
        unsigned char d_out[8];
        unsigned char d_copy[4102];
    };

    header_prefixer::sptr
    header_prefixer::make(int rate)
    {
    	return gnuradio::get_initial_sptr(new header_prefixer_impl(rate));
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


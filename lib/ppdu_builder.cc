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
#include <wifi_ofdm/ppdu_builder.h>
#include <gnuradio/block_detail.h>
#include <gnuradio/math.h>
#include <cstring>

namespace gr {
  namespace wifi_ofdm {
    #define d_debug 0
    #define dout d_debug && std::cout
  	#define PLCP_HEADER_BYTES 6
  	#define WIFI_SERVICE_BYTES 2
    // for puncturing
    static const unsigned char d_pun23[12] = {0,0,0,1,0,0,0,1,0,0,0,1};
    static const unsigned char d_pun34[18] = {0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0};
    // for interleaving
    static const unsigned int d_int48[48] = {0, 3, 6, 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42, 45, 1, 4, 7, 10, 13, 16, 19, 22, 25, 28, 31, 34, 37, 40, 43, 46, 2, 5, 8, 11, 14, 17, 20, 23, 26, 29, 32, 35, 38, 41, 44, 47};
    static const unsigned int d_int96[96] = {0, 6, 12, 18, 24, 30, 36, 42, 48, 54, 60, 66, 72, 78, 84, 90, 1, 7, 13, 19, 25, 31, 37, 43, 49, 55, 61, 67, 73, 79, 85, 91, 2, 8, 14, 20, 26, 32, 38, 44, 50, 56, 62, 68, 74, 80, 86, 92, 3, 9, 15, 21, 27, 33, 39, 45, 51, 57, 63, 69, 75, 81, 87, 93, 4, 10, 16, 22, 28, 34, 40, 46, 52, 58, 64, 70, 76, 82, 88, 94, 5, 11, 17, 23, 29, 35, 41, 47, 53, 59, 65, 71, 77, 83, 89, 95};
    //static const unsigned int d_int192_1[192] = {0, 12, 24, 36, 48, 60, 72, 84, 96, 108, 120, 132, 144, 156, 168, 180, 1, 13, 25, 37, 49, 61, 73, 85, 97, 109, 121, 133, 145, 157, 169, 181, 2, 14, 26, 38, 50, 62, 74, 86, 98, 110, 122, 134, 146, 158, 170, 182, 3, 15, 27, 39, 51, 63, 75, 87, 99, 111, 123, 135, 147, 159, 171, 183, 4, 16, 28, 40, 52, 64, 76, 88, 100, 112, 124, 136, 148, 160, 172, 184, 5, 17, 29, 41, 53, 65, 77, 89, 101, 113, 125, 137, 149, 161, 173, 185, 6, 18, 30, 42, 54, 66, 78, 90, 102, 114, 126, 138, 150, 162, 174, 186, 7, 19, 31, 43, 55, 67, 79, 91, 103, 115, 127, 139, 151, 163, 175, 187, 8, 20, 32, 44, 56, 68, 80, 92, 104, 116, 128, 140, 152, 164, 176, 188, 9, 21, 33, 45, 57, 69, 81, 93, 105, 117, 129, 141, 153, 165, 177, 189, 10, 22, 34, 46, 58, 70, 82, 94, 106, 118, 130, 142, 154, 166, 178, 190, 11, 23, 35, 47, 59, 71, 83, 95, 107, 119, 131, 143, 155, 167, 179, 191};
    static const unsigned int d_int192_2[192] = {0, 13, 24, 37, 48, 61, 72, 85, 96, 109, 120, 133, 144, 157, 168, 181, 1, 12, 25, 36, 49, 60, 73, 84, 97, 108, 121, 132, 145, 156, 169, 180, 2, 15, 26, 39, 50, 63, 74, 87, 98, 111, 122, 135, 146, 159, 170, 183, 3, 14, 27, 38, 51, 62, 75, 86, 99, 110, 123, 134, 147, 158, 171, 182, 4, 17, 28, 41, 52, 65, 76, 89, 100, 113, 124, 137, 148, 161, 172, 185, 5, 16, 29, 40, 53, 64, 77, 88, 101, 112, 125, 136, 149, 160, 173, 184, 6, 19, 30, 43, 54, 67, 78, 91, 102, 115, 126, 139, 150, 163, 174, 187, 7, 18, 31, 42, 55, 66, 79, 90, 103, 114, 127, 138, 151, 162, 175, 186, 8, 21, 32, 45, 56, 69, 80, 93, 104, 117, 128, 141, 152, 165, 176, 189, 9, 20, 33, 44, 57, 68, 81, 92, 105, 116, 129, 140, 153, 164, 177, 188, 10, 23, 34, 47, 58, 71, 82, 95, 106, 119, 130, 143, 154, 167, 178, 191, 11, 22, 35, 46, 59, 70, 83, 94, 107, 118, 131, 142, 155, 166, 179, 190};
    //static const unsigned int d_int288_1[288] = {0, 18, 36, 54, 72, 90, 108, 126, 144, 162, 180, 198, 216, 234, 252, 270, 1, 19, 37, 55, 73, 91, 109, 127, 145, 163, 181, 199, 217, 235, 253, 271, 2, 20, 38, 56, 74, 92, 110, 128, 146, 164, 182, 200, 218, 236, 254, 272, 3, 21, 39, 57, 75, 93, 111, 129, 147, 165, 183, 201, 219, 237, 255, 273, 4, 22, 40, 58, 76, 94, 112, 130, 148, 166, 184, 202, 220, 238, 256, 274, 5, 23, 41, 59, 77, 95, 113, 131, 149, 167, 185, 203, 221, 239, 257, 275, 6, 24, 42, 60, 78, 96, 114, 132, 150, 168, 186, 204, 222, 240, 258, 276, 7, 25, 43, 61, 79, 97, 115, 133, 151, 169, 187, 205, 223, 241, 259, 277, 8, 26, 44, 62, 80, 98, 116, 134, 152, 170, 188, 206, 224, 242, 260, 278, 9, 27, 45, 63, 81, 99, 117, 135, 153, 171, 189, 207, 225, 243, 261, 279, 10, 28, 46, 64, 82, 100, 118, 136, 154, 172, 190, 208, 226, 244, 262, 280, 11, 29, 47, 65, 83, 101, 119, 137, 155, 173, 191, 209, 227, 245, 263, 281, 12, 30, 48, 66, 84, 102, 120, 138, 156, 174, 192, 210, 228, 246, 264, 282, 13, 31, 49, 67, 85, 103, 121, 139, 157, 175, 193, 211, 229, 247, 265, 283, 14, 32, 50, 68, 86, 104, 122, 140, 158, 176, 194, 212, 230, 248, 266, 284, 15, 33, 51, 69, 87, 105, 123, 141, 159, 177, 195, 213, 231, 249, 267, 285, 16, 34, 52, 70, 88, 106, 124, 142, 160, 178, 196, 214, 232, 250, 268, 286, 17, 35, 53, 71, 89, 107, 125, 143, 161, 179, 197, 215, 233, 251, 269, 287};
    static const unsigned int d_int288_2[288] = {0, 20, 37, 54, 74, 91, 108, 128, 145, 162, 182, 199, 216, 236, 253, 270, 1, 18, 38, 55, 72, 92, 109, 126, 146, 163, 180, 200, 217, 234, 254, 271, 2, 19, 36, 56, 73, 90, 110, 127, 144, 164, 181, 198, 218, 235, 252, 272, 3, 23, 40, 57, 77, 94, 111, 131, 148, 165, 185, 202, 219, 239, 256, 273, 4, 21, 41, 58, 75, 95, 112, 129, 149, 166, 183, 203, 220, 237, 257, 274, 5, 22, 39, 59, 76, 93, 113, 130, 147, 167, 184, 201, 221, 238, 255, 275, 6, 26, 43, 60, 80, 97, 114, 134, 151, 168, 188, 205, 222, 242, 259, 276, 7, 24, 44, 61, 78, 98, 115, 132, 152, 169, 186, 206, 223, 240, 260, 277, 8, 25, 42, 62, 79, 96, 116, 133, 150, 170, 187, 204, 224, 241, 258, 278, 9, 29, 46, 63, 83, 100, 117, 137, 154, 171, 191, 208, 225, 245, 262, 279, 10, 27, 47, 64, 81, 101, 118, 135, 155, 172, 189, 209, 226, 243, 263, 280, 11, 28, 45, 65, 82, 99, 119, 136, 153, 173, 190, 207, 227, 244, 261, 281, 12, 32, 49, 66, 86, 103, 120, 140, 157, 174, 194, 211, 228, 248, 265, 282, 13, 30, 50, 67, 84, 104, 121, 138, 158, 175, 192, 212, 229, 246, 266, 283, 14, 31, 48, 68, 85, 102, 122, 139, 156, 176, 193, 210, 230, 247, 264, 284, 15, 35, 52, 69, 89, 106, 123, 143, 160, 177, 197, 214, 231, 251, 268, 285, 16, 33, 53, 70, 87, 107, 124, 141, 161, 178, 195, 215, 232, 249, 269, 286, 17, 34, 51, 71, 88, 105, 125, 142, 159, 179, 196, 213, 233, 250, 267, 287};
    class ppdu_builder_impl : public ppdu_builder
    {
    public:
    	ppdu_builder_impl(int seed) : block("ppdu_builder",
    		gr::io_signature::make(0,0,0),
    		gr::io_signature::make(0,0,0)),
    		d_in_port(pmt::mp("plcp_in")),
    		d_out_port(pmt::mp("ppdu_out"))
    	{
    		message_port_register_out(d_out_port);
    		message_port_register_in(d_in_port);
    		set_msg_handler(d_in_port,boost::bind(&ppdu_builder_impl::msg_in,this,_1));
    		d_seed = (unsigned char)seed;
    	}
    	~ppdu_builder_impl(){}
    	void msg_in(pmt::pmt_t msg)
    	{
    		pmt::pmt_t k = pmt::car(msg);
    		pmt::pmt_t v = pmt::cdr(msg);
    		size_t io(0);
    		const uint8_t* uvec = pmt::u8vector_elements(v,io);
    		// assert(io>6)
    		std::memcpy(d_copy,uvec,sizeof(char)*PLCP_HEADER_BYTES); // first 6 bytes comprise plcp header
    		if(k == pmt::intern("6Mbps")){
    			d_nbpsc = 1;
    			d_ncbps = 48;
    			d_ndbps = 24;
    		}else if(k == pmt::intern("9Mbps")){
    			d_nbpsc = 1;
    			d_ncbps = 48;
    			d_ndbps = 36;
    		}else if(k == pmt::intern("12Mbps")){
    			d_nbpsc = 2;
    			d_ncbps = 96;
    			d_ndbps = 48;
    		}else if(k == pmt::intern("18Mbps")){
    			d_nbpsc = 2;
    			d_ncbps = 96;
    			d_ndbps = 72;
    		}else if(k == pmt::intern("24Mbps")){
    			d_nbpsc = 4;
    			d_ncbps = 192;
    			d_ndbps = 96;
    		}else if(k == pmt::intern("36Mbps")){
    			d_nbpsc = 4;
    			d_ncbps = 192;
    			d_ndbps = 144;
    		}else if(k == pmt::intern("48Mbps")){
    			d_nbpsc = 6;
    			d_ncbps = 288;
    			d_ndbps = 192;
    		}else if(k == pmt::intern("54Mbps")){
    			d_nbpsc = 6;
    			d_ncbps = 288;
    			d_ndbps = 216;
    		}else{
    			// undefined abort
    			return;
    		}
    		build_data(uvec + PLCP_HEADER_BYTES,io - PLCP_HEADER_BYTES);
            pmt::pmt_t blob = pmt::make_blob(d_copy,d_nout+PLCP_HEADER_BYTES);
            message_port_pub(d_out_port,pmt::cons(pmt::intern("PPDU"),blob));
    	}
    private:
    	void build_data(const uint8_t* uvec, size_t nbyte){
            dout<<"DEBUG: call build data"<<std::endl;
    		// copy
    		int databits = (16+8*nbyte+6);
            dout<<"DEBUG: data bits="<<databits<<std::endl;
    		std::memset(d_code,0,WIFI_SERVICE_BYTES); // first 2 bytes to zeros
    		std::memcpy(d_code + WIFI_SERVICE_BYTES,uvec,sizeof(char)*nbyte);
    		int nsym = ceil(databits/(float)d_ndbps); // 16 bits service, 6 bits zeros
    		int npad = nsym * d_ndbps - databits;
            // only count the psdu, service, tails and padded zeros
    		d_nout = ceil(nsym * d_ncbps /(float) 8.0);
    		// padding zeros
    		std::memset(d_code+(WIFI_SERVICE_BYTES+nbyte),0,d_nout-(WIFI_SERVICE_BYTES+nbyte));
    		// scrambling
    		d_nbits = databits+npad;
    		scrambler();
    		// null 6 bits (TAIL)
    		d_scramble[WIFI_SERVICE_BYTES+nbyte] &= 0xC0; // null the scrambled six bits
            dout<<"DEBUG: print scrambled bytes after nulling:"<<std::endl;
            for(int i=0;i<d_nbits/8;++i)
                dout<<" "<<(int)d_scramble[i];
            dout<<std::endl;
    		// conv_coding
    		// (register code)
    		conv_enc();
    		// pucturing (optional)
    		puncturing();
            // interleaving
            interleaver();
            // ready to build bit-level packet
    	}
    	void scrambler(){
    		// initial seed
    		std::memset(d_scramble,0,d_nout);
    		d_scramble_reg = d_seed;
            dout<<"Debug: in scrambler, nbits="<<d_nbits<<", seed="<<(int)d_scramble_reg<<std::endl;
    		unsigned char tmp; 
    		for(int i=0;i<d_nbits;++i){
    			tmp = (d_scramble_reg>>3) ^ (d_scramble_reg>>6);
    			d_scramble[i/8] |= ( ( (tmp & 0x01) ^ ((d_code[i/8]>>(i%8)) & 0x01) ) << (i%8) );
    			d_scramble_reg = (d_scramble_reg<<1) | (tmp & 0x01);
    		}
            // DEBUGGING
            dout<<"DEBUG: print scrambled bytes:"<<std::endl;
            for(int i=0;i<d_nbits/8;++i)
                dout<<" "<<(int)d_scramble[i];
            dout<<std::endl;
    	}
    	void conv_enc(){
    		std::memset(d_code,0,(int)ceil(d_nbits/8));
    		unsigned char enc_reg=0x00, tmp_bit=0x00;
    		unsigned char outbit[2];
    		for(int i=0;i<d_nbits;++i){
    			tmp_bit = (d_scramble[i/8] >> (i/8)) & 0x01;
    			outbit[0] = (tmp_bit ^ (enc_reg>>1) ^ (enc_reg>>2) ^ (enc_reg>>4) ^ (enc_reg>>5)) & 0x01;
    			outbit[1] = (tmp_bit ^ (enc_reg>>1) ^ (enc_reg>>2) ^ (enc_reg>>3) ^ (enc_reg>>5)) & 0x01;
    			d_code[(2*i)/8] |= ((outbit[0]<< ((2*i) % 8)) | (outbit[1]<< (2*i+1)%8));
    			enc_reg = (enc_reg<<1) | tmp_bit;
    		}
            dout<<"DEBUG: printing conv_coded data bytes"<<std::endl;
            for(int i=0;i<d_nbits/4;++i)
                dout<<" "<<(int)d_code[i];
            dout<<std::endl;
    	}
    	void puncturing(){
            // reuse d_scramble_reg
            std::memset(d_scramble,0,d_nout);
            int pcnt=0;
            unsigned char tmpbit;
            if(d_ndbps==24 || d_ndbps==48 || d_ndbps == 96){
                memcpy(d_scramble,d_code,sizeof(char)*d_nout);
                return;
            }else if(d_ndbps == 192){
                // rate r= 2/3
                for(int i=0;i<d_nbits*2;++i){
                    tmpbit = (d_code[i/8] >> (i%8)) & 0x01;
                    if(d_pun23[i % 12] == 0){
                        d_scramble[pcnt/8] |= (tmpbit<<(pcnt % 8));
                        pcnt++;
                    }
                }
            }else{
                // rate r=3/4
                for(int i=0;i<d_nbits*2;++i){
                    tmpbit = (d_code[i/8] >> (i%8)) & 0x01;
                    if(d_pun34[i % 18] == 0){
                        d_scramble[pcnt/8] |= (tmpbit<<(pcnt % 8));
                        pcnt++;
                    }
                }
            }
    	}
        void interleaver(){
            const unsigned int* int_ptr;
            if(d_ncbps==48){
                int_ptr = d_int48;
            }else if(d_ncbps == 96){
                int_ptr = d_int96;
            }else if(d_ncbps == 192){
                int_ptr = d_int192_2;
            }else{
                int_ptr = d_int288_2;
            }
            std::memset(d_copy+PLCP_HEADER_BYTES,0,d_nout);
            unsigned char tmpbit;
            unsigned int intidx;

            for(int i=0;i<d_nout*8;++i){
                tmpbit = (d_scramble[i/8] >> (i%8)) & 0x01;
                intidx = (i/d_ncbps) *d_ncbps + int_ptr[ i % d_ncbps];
                d_copy[PLCP_HEADER_BYTES + intidx/8] |= (tmpbit << (intidx % 8));
            }
        }
    	const pmt::pmt_t d_in_port;
    	const pmt::pmt_t d_out_port;
    	unsigned char d_copy[8200];
    	unsigned char d_code[8200];
    	unsigned char d_scramble[8200];
    	unsigned char d_seed;
    	unsigned char d_scramble_reg;
    	int d_ncbps;
    	int d_nbpsc;
    	int d_ndbps;
    	int d_nout;
    	int d_nbits;
    };

    ppdu_builder::sptr 
    ppdu_builder::make(int seed)
    {
    	return gnuradio::get_initial_sptr(new ppdu_builder_impl(seed));
    }

  } /* namespace wifi_ofdm */
} /* namespace gr */


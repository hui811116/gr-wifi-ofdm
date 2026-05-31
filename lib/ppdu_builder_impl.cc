/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ppdu_builder_impl.h"
#include <gnuradio/io_signature.h>



namespace gr {
namespace wifi_ofdm {

#define WIFI_SERVICE_BYTES 2
// for puncturing
static const unsigned char d_pun23[12] = {0,0,0,1,0,0,0,1,0,0,0,1};
static const unsigned char d_pun34[18] = {0,0,0,1,1,0,0,0,0,1,1,0,0,0,0,1,1,0};
// for interleaving
static const unsigned int d_int48[48] = {
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
    30, 33, 36, 39, 42, 45, 1, 4, 7, 10,
    13, 16, 19, 22, 25, 28, 31, 34, 37, 40,
    43, 46, 2, 5, 8, 11, 14, 17, 20, 23,
    26, 29, 32, 35, 38, 41, 44, 47};
static const unsigned int d_int96[96] = {
    0, 6, 12, 18, 24, 30, 36, 42, 48, 54,
    60, 66, 72, 78, 84, 90, 1, 7, 13, 19,
    25, 31, 37, 43, 49, 55, 61, 67, 73, 79,
    85, 91, 2, 8, 14, 20, 26, 32, 38, 44,
    50, 56, 62, 68, 74, 80, 86, 92, 3, 9,
    15, 21, 27, 33, 39, 45, 51, 57, 63, 69,
    75, 81, 87, 93, 4, 10, 16, 22, 28, 34,
    40, 46, 52, 58, 64, 70, 76, 82, 88, 94,
    5, 11, 17, 23, 29, 35, 41, 47, 53, 59,
    65, 71, 77, 83, 89, 95};
static const unsigned int d_int192_2[192] = {
    0, 13, 24, 37, 48, 61, 72, 85, 96, 109,
    120, 133, 144, 157, 168, 181, 1, 12, 25, 36,
    49, 60, 73, 84, 97, 108, 121, 132, 145, 156,
    169, 180, 2, 15, 26, 39, 50, 63, 74, 87,
    98, 111, 122, 135, 146, 159, 170, 183, 3, 14,
    27, 38, 51, 62, 75, 86, 99, 110, 123, 134,
    147, 158, 171, 182, 4, 17, 28, 41, 52, 65,
    76, 89, 100, 113, 124, 137, 148, 161, 172, 185,
    5, 16, 29, 40, 53, 64, 77, 88, 101, 112,
    125, 136, 149, 160, 173, 184, 6, 19, 30, 43,
    54, 67, 78, 91, 102, 115, 126, 139, 150, 163,
    174, 187, 7, 18, 31, 42, 55, 66, 79, 90,
    103, 114, 127, 138, 151, 162, 175, 186, 8, 21,
    32, 45, 56, 69, 80, 93, 104, 117, 128, 141,
    152, 165, 176, 189, 9, 20, 33, 44, 57, 68,
    81, 92, 105, 116, 129, 140, 153, 164, 177, 188,
    10, 23, 34, 47, 58, 71, 82, 95, 106, 119,
    130, 143, 154, 167, 178, 191, 11, 22, 35, 46,
    59, 70, 83, 94, 107, 118, 131, 142, 155, 166,
    179, 190};
static const unsigned int d_int288_2[288] = {
    0, 20, 37, 54, 74, 91, 108, 128, 145, 162,
    182, 199, 216, 236, 253, 270, 1, 18, 38, 55,
    72, 92, 109, 126, 146, 163, 180, 200, 217, 234,
    254, 271, 2, 19, 36, 56, 73, 90, 110, 127,
    144, 164, 181, 198, 218, 235, 252, 272, 3, 23,
    40, 57, 77, 94, 111, 131, 148, 165, 185, 202,
    219, 239, 256, 273, 4, 21, 41, 58, 75, 95,
    112, 129, 149, 166, 183, 203, 220, 237, 257, 274,
    5, 22, 39, 59, 76, 93, 113, 130, 147, 167,
    184, 201, 221, 238, 255, 275, 6, 26, 43, 60,
    80, 97, 114, 134, 151, 168, 188, 205, 222, 242,
    259, 276, 7, 24, 44, 61, 78, 98, 115, 132,
    152, 169, 186, 206, 223, 240, 260, 277, 8, 25,
    42, 62, 79, 96, 116, 133, 150, 170, 187, 204,
    224, 241, 258, 278, 9, 29, 46, 63, 83, 100,
    117, 137, 154, 171, 191, 208, 225, 245, 262, 279,
    10, 27, 47, 64, 81, 101, 118, 135, 155, 172,
    189, 209, 226, 243, 263, 280, 11, 28, 45, 65,
    82, 99, 119, 136, 153, 173, 190, 207, 227, 244,
    261, 281, 12, 32, 49, 66, 86, 103, 120, 140,
    157, 174, 194, 211, 228, 248, 265, 282, 13, 30,
    50, 67, 84, 104, 121, 138, 158, 175, 192, 212,
    229, 246, 266, 283, 14, 31, 48, 68, 85, 102,
    122, 139, 156, 176, 193, 210, 230, 247, 264, 284,
    15, 35, 52, 69, 89, 106, 123, 143, 160, 177,
    197, 214, 231, 251, 268, 285, 16, 33, 53, 70,
    87, 107, 124, 141, 161, 178, 195, 215, 232, 249,
    269, 286, 17, 34, 51, 71, 88, 105, 125, 142,
    159, 179, 196, 213, 233, 250, 267, 287};

#define RATE6MBPS 0
#define RATE9MBPS 1
#define RATE12MBPS 2
#define RATE18MBPS 3
#define RATE24MBPS 4
#define RATE36MBPS 5
#define RATE48MBPS 6
#define RATE54MBPS 7
//#pragma message("set the following appropriately and remove this warning")
//using input_type = float;
//#pragma message("set the following appropriately and remove this warning")
//using output_type = float;
ppdu_builder::sptr ppdu_builder::make(int seed, int rate)
{
    return gnuradio::make_block_sptr<ppdu_builder_impl>(seed, rate);
}


/*
 * The private constructor
 */
ppdu_builder_impl::ppdu_builder_impl(int seed, int rate)
    : gr::block("ppdu_builder",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
      d_in_port(pmt::mp("mac_in")),
      d_out_port(pmt::mp("plcp_out"))

{
    message_port_register_out(d_out_port);
    message_port_register_in(d_in_port);
    set_msg_handler(d_in_port,
                        [this](pmt::pmt_t msg) {this->build_ppdu(msg); });
    d_seed = (unsigned char)seed;
    set_ppdu_rate(rate);
}

// graceful shut down
bool ppdu_builder_impl::stop()
{
    d_shutdown.store(true);
    return gr::block::stop();
}

void ppdu_builder_impl::build_ppdu(pmt::pmt_t msg)
{
    gr::thread::scoped_lock lock(d_mutex);
    pmt::pmt_t k = pmt::car(msg);
    pmt::pmt_t v = pmt::cdr(msg);
    size_t io(0);
    const uint8_t* uvec = pmt::u8vector_elements(v, io);

    build_data(uvec,io);
    if ( d_shutdown.load() ) return;
    pmt::pmt_t blob = pmt::make_blob(d_code, d_nout);

    message_port_pub(d_out_port, pmt::cons(pmt::from_long(io + d_rate * 8192), blob));
}


void ppdu_builder_impl::set_ppdu_rate(int rate)
{
    //std::scoped_lock lock(d_mutex);
    gr::thread::scoped_lock lock(d_mutex);
    switch(rate){
        case RATE6MBPS:
            d_nbpsc = 1;
            d_ncbps = 48;
            d_ndbps = 24;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 6MBPS");
        break;
        case RATE9MBPS:
            d_nbpsc = 1;
            d_ncbps = 48;
            d_ndbps = 36;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 9MBPS");
        break;
        case RATE12MBPS:
            d_nbpsc = 2;
            d_ncbps = 96;
            d_ndbps = 48;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 12MBPS");
        break;
        case RATE18MBPS:
            d_nbpsc = 2;
            d_ncbps = 96;
            d_ndbps = 72;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 18MBPS");
        break;
        case RATE24MBPS:
            d_nbpsc = 4;
            d_ncbps = 192;
            d_ndbps = 96;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 24MBPS");
        break;
        case RATE36MBPS:
            d_nbpsc = 4;
            d_ncbps = 192;
            d_ndbps = 144;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 36MBPS");
        break;
        case RATE48MBPS:
            d_nbpsc = 6;
            d_ncbps = 288;
            d_ndbps = 192;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 48MBPS");
        break;
        case RATE54MBPS:
            d_nbpsc = 6;
            d_ncbps = 288;
            d_ndbps = 216;
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Rate -- 54MBPS");
        break;
        default:
            // using default rate 6MBPS
            d_nbpsc = 1;
            d_ncbps = 48;
            d_ndbps = 24;
            //throw std::invalid_argument("Undefined Data rate, abort");
            GR_LOG_INFO(this->d_logger, "PPDU_builder: Unknown Rate, use default rate -- 6MBPS");
        break;
    }
    //d_rate = rate;
    d_rate.store(rate);
}

int ppdu_builder_impl::get_rate() const
{
    return d_rate.load();
}

void ppdu_builder_impl::build_data(const uint8_t* uvec, size_t nbyte)
{
    if (d_shutdown.load()) return;
    // safety check for memory buffer size
    int databits = (16+8*nbyte+6);
    int max_bits = MAX_CODE_BUFFER_BYTES * 8;
    if ( databits > max_bits ){
        std::ostringstream oss;
        oss << "Input data bits (" << databits << ") exceeds code buffer capacity ("
            << max_bits << " bits / " << MAX_CODE_BUFFER_BYTES << " bytes)";
        throw std::runtime_error(oss.str());
    }
    int nsym = ceil(databits/(float)d_ndbps); // 16 bits service, 6 bits zeros
    int npad = nsym * d_ndbps - databits;
    // only count the psdu, service, tails and padded zeros
    d_nout = nsym * d_ncbps / 8; // bytes that accounts for coded & punctured & interleaved bytes
    // padding zeros
    // reset memory
    std::memset(d_code,0,d_nout);
    // scrambling
    d_nbits = databits+npad;
    service_and_scrambler(uvec,nbyte*8,npad+6); // 6 zeros included to padded zeros
    // null 6 bits (TAIL)
    d_scramble[WIFI_SERVICE_BYTES+nbyte] = 0x00; // null the scrambled six bits
    // conv_coding
    conv_enc();
    // pucturing
    puncturing();
    // interleaving
    interleaver();
    // ready to build bit-level packet
}

void ppdu_builder_impl::service_and_scrambler(const uint8_t* uvec,int ndata, int nzero){
    if (d_shutdown.load()) return;
    // initial seed
    int ncnt;
    // NOTE: clear the full buffer
    //std::memset(d_scramble,0,d_nbits/8);
    std::memset(d_scramble,0,sizeof(d_scramble));
    d_scramble_reg = d_seed;
    unsigned char tmp;
    // service 16 zeros
    for(ncnt=0;ncnt<16;++ncnt){
        tmp = ((d_scramble_reg>>3) ^ (d_scramble_reg>>6)) & 0x01;
        d_scramble[ncnt/8] |= (tmp << (ncnt%8));
        d_scramble_reg = ((d_scramble_reg<<1) | tmp);
    }
    // data bytes
    for(int i=0;i<ndata;++i){
        tmp = ((d_scramble_reg>>3) ^ (d_scramble_reg>>6)) & 0x01;
        d_scramble[ncnt/8] |= ( ( tmp ^ ((uvec[i/8]>>(i%8)) & 0x01) ) << (ncnt%8) );
        d_scramble_reg = (d_scramble_reg<<1) | tmp;
        ncnt++;
    }
    // padded zeros
    for(int i=0;i<nzero;++i){
        tmp = ((d_scramble_reg>>3) ^ (d_scramble_reg>>6)) & 0x01;
        d_scramble[ncnt/8] |= (tmp << (ncnt%8));
        d_scramble_reg = (d_scramble_reg<<1) | tmp;
        ncnt++;
    }
}
void ppdu_builder_impl::conv_enc(){
    if (d_shutdown.load()) return;
    // NOTE: full buffer cleaning
    std::memset(d_code,0,sizeof(d_code));
    //std::memset(d_code,0,(int)ceil(d_nbits/4.0));
    unsigned char enc_reg=0x00, tmp_bit=0x00;
    unsigned char outbit[2];
    for(int i=0;i<d_nbits;++i){
        tmp_bit = (d_scramble[i/8] >> (i%8)) & 0x01;
        outbit[0] = (tmp_bit ^ (enc_reg>>1) ^ (enc_reg>>2) ^ (enc_reg>>4) ^ (enc_reg>>5)) & 0x01;
        outbit[1] = (tmp_bit ^ enc_reg ^ (enc_reg>>1) ^ (enc_reg>>2) ^ (enc_reg>>5)) & 0x01;
        d_code[i/4] |= (outbit[0]<< ( (2*i)%8 ));
        d_code[i/4] |= (outbit[1]<< ( (2*i+1)%8 )); 
        enc_reg = ((enc_reg<<1) | tmp_bit) & 0x3f;
    }
}
void ppdu_builder_impl::puncturing(){
    if (d_shutdown.load()) return;
    const unsigned char* pun_ptr;
    int punPeriod;
    std::memset(d_breg,0,36);
    int pcnt=0, rewrite_cnt =0;
    unsigned char tmpbit;
    if(d_ndbps==24 || d_ndbps==48 || d_ndbps == 96){
        return;
    }else if(d_ndbps == 192){
        // rate r= 2/3
        pun_ptr = d_pun23;
        punPeriod = 12;
    }else{
        // rate r=3/4
        pun_ptr = d_pun34;
        punPeriod = 18;
    }
    for(int i=0;i<d_nbits*2;++i){
        tmpbit = (d_code[i/8] >> (i%8)) & 0x01;
        if(pun_ptr[i % punPeriod] == 0){
            d_breg[pcnt/8] |= (tmpbit << (pcnt % 8));
            pcnt++;
            if( pcnt == d_ncbps){
                memcpy(&d_code[rewrite_cnt],d_breg,sizeof(char)*d_ncbps/8);
                rewrite_cnt+= (d_ncbps/8);
                std::memset(d_breg,0,36);
                pcnt =0;
            }
        }
    }
}
void ppdu_builder_impl::interleaver(){
    if (d_shutdown.load()) return;
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
    std::memset(d_breg,0,d_ncbps/8);
    unsigned char tmpbit;
    unsigned int intidx;
    //int icnt = 0, rewrite_cnt=0;
    int rewrite_cnt = 0;

    for(int i=0;i<d_nout*8;++i){
        tmpbit = (d_code[i/8] >> (i%8)) & 0x01;
        intidx = int_ptr[ i % d_ncbps];
        d_breg[intidx/8] |= (tmpbit << (intidx % 8));
        if( ((i+1) % d_ncbps) == 0 ){
            memcpy(&d_code[rewrite_cnt],d_breg,sizeof(char)*d_ncbps/8);
            std::memset(d_breg,0,d_ncbps/8);
            rewrite_cnt += (d_ncbps/8);
        }
    }
}


} /* namespace wifi_ofdm */
} /* namespace gr */

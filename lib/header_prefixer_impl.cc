/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "header_prefixer_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace wifi_ofdm {


header_prefixer::sptr header_prefixer::make()
{
    return gnuradio::make_block_sptr<header_prefixer_impl>();
}


/*
 * The private constructor
 */
header_prefixer_impl::header_prefixer_impl()
    : gr::block("header_prefixer",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
                d_in_port(pmt::mp("plcp_in")),
                d_out_port(pmt::mp("ppdu_out"))
{
    message_port_register_in(d_in_port);
    message_port_register_out(d_out_port);
    set_msg_handler(d_in_port,[this](pmt::pmt_t msg){this->hdr_prefix(msg);});
}

/*
 * Our virtual destructor.
 */
header_prefixer_impl::~header_prefixer_impl() {}

void header_prefixer_impl::hdr_prefix(pmt::pmt_t msg)
{
    pmt::pmt_t k = pmt::car(msg);
    pmt::pmt_t v = pmt::cdr(msg);
    d_rate_k = k;
    
    size_t io(0);
    const uint8_t* uvec = pmt::u8vector_elements(v,io);

    if (io + WIFI80211A_HEADER_BYTES + 1 > MAX_ENC_BUFFER_BYTES) {
        throw std::runtime_error("header_prefixer: Input PSDU exceeds maximum buffer size");
    }

    int datalen = io;
    d_rate = 0; // Default rate

    if (pmt::is_integer(k)) {
        long k_val = pmt::to_long(k);
        datalen = k_val % 8192;
        d_rate = k_val / 8192;
    } else if (pmt::is_dict(k)) {
        pmt::pmt_t rate_pmt = pmt::dict_ref(k, pmt::mp("rate"), pmt::PMT_NIL);
        if (pmt::is_integer(rate_pmt)) {
            d_rate = pmt::to_long(rate_pmt);
        }
    }
    if (d_rate > 7) d_rate = 0; // Ensure rate is within valid bounds

    d_u16len = (uint16_t) datalen;
    d_u8ptr = (uint8_t*) & d_u16len;
    d_hdr_buf[0] = d_rate_set[d_rate];
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
    // the rest 6 bits should be zeros
    // performing 133,171 conv coding
    conv_enc(d_hdr_buf); // store in d_enc
    // do interleaving
    interleave();
    // d_out contains the produced header, 6 bytes
    std::memcpy(d_copy+WIFI80211A_HEADER_BYTES, uvec, sizeof(char) * io);
    // NOTE: hide the rate tag to the last byte
    d_copy[io+WIFI80211A_HEADER_BYTES] = d_rate_set[d_rate];
    pmt::pmt_t blob = pmt::make_blob(d_copy,io+WIFI80211A_HEADER_BYTES+1); // hidden tag
    message_port_pub(d_out_port,pmt::cons(pmt::PMT_NIL,blob));
}

void header_prefixer_impl::conv_enc(const uint8_t* uvec)
{
    uint8_t enc_reg = 0x00, tmp_lsb = 0x00;
    uint8_t tmp_bit[2];
    std::memset(d_enc, 0, MAX_HDR_BUFFER_BYTES);
    for(int i=0;i < WIFI80211A_HEADER_BYTES * MAX_HDR_BUFFER_BYTES /2 ;++i){
        tmp_lsb = (uvec[i/MAX_HDR_BUFFER_BYTES] >> (i%MAX_HDR_BUFFER_BYTES)) & 0x01;
        tmp_bit[0] = (((tmp_lsb) ^ (enc_reg >> 1) ^ (enc_reg >> 2)
                        ^ (enc_reg >> 4) ^ (enc_reg >> 5)) & 0x01); // first output
        tmp_bit[1] = (((tmp_lsb) ^ enc_reg ^ (enc_reg >> 1) ^ (enc_reg >> 2)
                        ^ (enc_reg >> 5)) & 0x01); // second output
        d_enc[i/4] |= (tmp_bit[0]<< ( (2*i)  % MAX_HDR_BUFFER_BYTES));
        d_enc[i/4] |= (tmp_bit[1]<< ((2*i+1) % MAX_HDR_BUFFER_BYTES));
        enc_reg = ((enc_reg<<1) | tmp_lsb) & 0x3f;
    }
}

void header_prefixer_impl::interleave()
{
    std::memset(d_out, 0, MAX_HDR_BUFFER_BYTES);
    for(int i=0;i<WIFI80211A_HEADER_BYTES * MAX_HDR_BUFFER_BYTES;++i){
        int newPos = d_inter[i];
        d_out[newPos/MAX_HDR_BUFFER_BYTES] |= (  ((d_enc[i/MAX_HDR_BUFFER_BYTES] >> (i % MAX_HDR_BUFFER_BYTES)) & 0x01) << (newPos % MAX_HDR_BUFFER_BYTES) );
    }
    std::memcpy(d_copy,d_out,sizeof(char)*WIFI80211A_HEADER_BYTES);
}

} /* namespace wifi_ofdm */
} /* namespace gr */

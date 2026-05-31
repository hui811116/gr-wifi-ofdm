/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_HEADER_PREFIXER_IMPL_H
#define INCLUDED_WIFI_OFDM_HEADER_PREFIXER_IMPL_H

#include <gnuradio/wifi_ofdm/header_prefixer.h>

namespace gr {
namespace wifi_ofdm {

#define MAX_ENC_BUFFER_BYTES 8200
#define MAX_HDR_BUFFER_BYTES 8
#define WIFI80211A_HEADER_BYTES 6
static const unsigned char d_rate_set[8]= {
    0x0B, 0x0F, 0x0A, 0x0E, 0x09, 0x0D, 0x08, 0x0C
};
static const size_t d_inter[48] = {
    0, 3, 6, 9, 12, 15, 18, 21, 24, 27,
    30, 33, 36, 39, 42, 45, 1, 4, 7, 10,
    13, 16, 19, 22, 25, 28, 31, 34, 37, 40,
    43, 46, 2, 5, 8, 11, 14, 17, 20, 23,
    26, 29, 32, 35, 38, 41, 44, 47
};

class header_prefixer_impl : public header_prefixer
{
private:
    void conv_enc(const uint8_t* uvec);
    void interleave();
    const pmt::pmt_t d_in_port;
    const pmt::pmt_t d_out_port;
    pmt::pmt_t d_rate_k;
    size_t d_rate;
    uint16_t d_u16len;
    uint8_t* d_u8ptr;
    unsigned char d_hdr_buf[MAX_HDR_BUFFER_BYTES];
    unsigned char d_enc[MAX_HDR_BUFFER_BYTES];
    unsigned char d_out[MAX_HDR_BUFFER_BYTES];
    unsigned char d_copy[MAX_ENC_BUFFER_BYTES];

public:
    header_prefixer_impl();
    ~header_prefixer_impl();

    void hdr_prefix(pmt::pmt_t msg);
    
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_HEADER_PREFIXER_IMPL_H */

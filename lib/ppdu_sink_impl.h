/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_PPDU_SINK_IMPL_H
#define INCLUDED_WIFI_OFDM_PPDU_SINK_IMPL_H

#include <gnuradio/wifi_ofdm/ppdu_sink.h>
#include <gnuradio/thread/thread.h>

namespace gr {
namespace wifi_ofdm {
#define MAX_DEC_BUFFER_BYTES 8192
#define MAX_CONV_CODE_BYTES 128
#define RX_SERVICE_BYTES 4
#define FEC_WRAP 1296
#define CONV_CODE_MAX_STATE 64
static const unsigned char d_output_table[64][2]={
    {0,3},{2,1},{3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},
    {3,0},{1,2},{3,0},{1,2},{0,3},{2,1},{1,2},{3,0},{2,1},{0,3},
    {2,1},{0,3},{1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},
    {1,2},{3,0},{3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},
    {3,0},{1,2},{0,3},{2,1},{0,3},{2,1},{3,0},{1,2},{2,1},{0,3},
    {1,2},{3,0},{1,2},{3,0},{2,1},{0,3},{2,1},{0,3},{1,2},{3,0},
    {1,2},{3,0},{2,1},{0,3}
};
static const unsigned char d_pun34[18] = {0x01,0x01,0x01,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x00,0x01};
static const unsigned char d_pun23[12] = {0x01,0x01,0x01,0x00,0x01,0x01,0x01,0x00,0x01,0x01,0x01,0x00};
static const unsigned char d_punnon[2] = {0x01,0x01};
class ppdu_sink_impl : public ppdu_sink
{
enum RATESET{
    RATE6MBPS=6,
    RATE9MBPS=9,
    RATE12MBPS=12,
    RATE18MBPS=18,
    RATE24MBPS=24,
    RATE36MBPS=36,
    RATE48MBPS=48,
    RATE54MBPS=54
};
private:
    // Nothing to declare in this block.
    void descramble(int size);
    void seed_est();
    bool conv_dec(const uint8_t* uvec, size_t noutputs);

    const pmt::pmt_t d_in_port;
    const pmt::pmt_t d_out_port;
    std::atomic_int d_rate{RATE6MBPS};
    std::atomic_int d_pun_mod{18};
    std::atomic_bool d_shutdown{false};
    gr::thread::mutex d_mutex;

    unsigned char d_decode[MAX_DEC_BUFFER_BYTES];
    unsigned int d_cost[FEC_WRAP][MAX_CONV_CODE_BYTES];
    unsigned char d_track[FEC_WRAP][MAX_CONV_CODE_BYTES];
    unsigned char d_rx_service[RX_SERVICE_BYTES];
    const unsigned char* d_pun_ptr;
    uint8_t d_seed_est;
    
public:
    ppdu_sink_impl();
    ~ppdu_sink_impl(){};
    void ppdu_in(pmt::pmt_t msg);
    bool stop() override;
    // Where all the action really happens
    //void forecast(int noutput_items, gr_vector_int& ninput_items_required);

    //int general_work(int noutput_items,
    //                 gr_vector_int& ninput_items,
    //                 gr_vector_const_void_star& input_items,
    //                 gr_vector_void_star& output_items);
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_PPDU_SINK_IMPL_H */

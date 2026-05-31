/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "ppdu_sink_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace wifi_ofdm {

//#pragma message("set the following appropriately and remove this warning")
//using input_type = float;
//#pragma message("set the following appropriately and remove this warning")
//using output_type = float;
ppdu_sink::sptr ppdu_sink::make() { return gnuradio::make_block_sptr<ppdu_sink_impl>(); }


/*
 * The private constructor
 */
ppdu_sink_impl::ppdu_sink_impl()
    : gr::block("ppdu_sink",
                gr::io_signature::make(0, 0, 0),
                gr::io_signature::make(0, 0, 0)),
                d_in_port(pmt::mp("ppdu_in")),
                d_out_port(pmt::mp("plcp_out"))
{
    message_port_register_in(d_in_port);
    message_port_register_out(d_out_port);
    set_msg_handler(d_in_port,[this](pmt::pmt_t msg){this->ppdu_in(msg);});
}

bool ppdu_sink_impl::stop()
{
    d_shutdown.store(true);
    return gr::block::stop();
}


void ppdu_sink_impl::ppdu_in(pmt::pmt_t msg)
{
    if( d_shutdown.load() ) return;
    gr::thread::scoped_lock lock(d_mutex);
    auto k = pmt::car(msg);
    auto v = pmt::cdr(msg);

    d_rate.store(pmt::to_long(k));
    auto local_rate = pmt::to_long(k);
    if( local_rate == RATE48MBPS )
    {
        // rate 2/3
        d_pun_ptr = d_pun23;
        d_pun_mod.store(12);
    } else if( local_rate == RATE6MBPS || local_rate == RATE12MBPS || local_rate == RATE24MBPS )
    {
        // rate 1/2
        d_pun_ptr = d_punnon;
        d_pun_mod.store(2);
    } else
    {
        d_pun_ptr = d_pun34;
        d_pun_mod.store(18);
    }
    size_t io(0);
    int tmp_seed;
    const uint8_t* uvec = pmt::u8vector_elements(v, io);
    //return;//NOTE: debug
    if(conv_dec(uvec, io))
    {
        seed_est();
        tmp_seed = d_seed_est;
        descramble( io*4 - 2 - 6 );
        d_rx_service[0] = d_decode[0];
        d_rx_service[1] = d_decode[1];
        // debug message here
        std::ostringstream oss;
        oss <<"PPDU_SINK Received a packet --  datarate ["<<d_rate << " Mbps ]"
            <<", PPDU_LENGTH [ "<<io/2 - 3<<" Bytes ]"
            <<", Estimated SEED [ "<<tmp_seed<<" ]"
            <<", Service bytes(MSB-Left)="<<std::hex<<(int)d_rx_service[1]
            <<","<<(int)d_rx_service[0]<<std::dec<<std::endl;
        GR_LOG_INFO(this->d_logger,oss.str());
        // publish payload
        pmt::pmt_t blob = pmt::make_blob(d_decode + 2, io/2 - 3);
        message_port_pub(d_out_port, pmt::cons(pmt::PMT_NIL, blob));
    }else
    {
        // decoding failure
        return; 
    }
}

void ppdu_sink_impl::descramble(int size)
{
    if( d_shutdown.load()) return;
    //gr::thread::scoped_lock lock(d_mutex);
    uint8_t buf = 0x00, tmpbit, d_state = d_seed_est;
    for(int i=0;i<size;++i)
    {
        tmpbit = ((d_state>>3) ^ (d_state >>6)) & 0x01;
        buf |= (((tmpbit ^ (d_decode[i/8]>>(i%8) ) ) & 0x01) << (i%8));
        // update state
        d_state = ( (d_state << 1) | tmpbit) & 0x7F;
        if( ((i+1) % 8) == 0)
        {
            d_decode[i/8] = buf;
            buf = 0x00;
        }
    }
}

void ppdu_sink_impl::seed_est()
{
    if( d_shutdown.load()) return;
    //gr::thread::scoped_lock lock(d_mutex);
    d_seed_est = 0x00;
    d_seed_est |= (((d_decode[0]>>2)  ^ (d_decode[0]>>6)) & 0x01);
    d_seed_est |= ((((d_decode[0]>>1) ^ (d_decode[0]>>5)) & 0x01) << 1);
    d_seed_est |= ((((d_decode[0])    ^ (d_decode[0]>>4)) & 0x01) << 2);
    d_seed_est |= ((((d_decode[0]>>2) ^ (d_decode[0]>>6) ^ (d_decode[0]>>3)) & 0x01) << 3);
    d_seed_est |= ((((d_decode[0]>>1) ^ (d_decode[0]>>5) ^ (d_decode[0]>>2)) & 0x01) << 4);
    d_seed_est |= ((((d_decode[0]>>0) ^ (d_decode[0]>>4) ^ (d_decode[0]>>1)) & 0x01) << 5);
    d_seed_est |= ((((d_decode[0]>>2) ^ (d_decode[0]>>6) ^ (d_decode[0]>>3) ^ d_decode[0]) & 0x01) << 6);
}

bool ppdu_sink_impl::conv_dec(const uint8_t* uvec, size_t noutputs)
{
    if( d_shutdown.load()) return false;

    const int nbit = noutputs * 4 - 2; // last byte has only 6 zeros
    int nwrite = 0; // data bits that are decoded
    int ncnt = 0;
    // clear up full buffer
    std::memset(d_decode,0,MAX_DEC_BUFFER_BYTES);
    for(int i=0;i<64;++i){
        d_cost[0][i] = UINT_MAX;
        d_track[0][i] = 0;
    }
    unsigned char cur=0x00, nex0=0x00, nex1=0x01;
    const uint8_t out_mask = 0x03;
    unsigned char output = uvec[0] & out_mask;

    unsigned char tmp_cmp_0 = output ^ d_output_table[cur][0];
    unsigned char tmp_cmp_1 = output ^ d_output_table[cur][1];
    uint32_t cost_cnt_0 = 0;
    uint32_t cost_cnt_1 = 0;
    while( ncnt < nbit)
    {
        if( ncnt%FEC_WRAP == 0 )
        {
            // initialization and update
            output = (uvec[ncnt*2 / 8] >> (2*ncnt % 8)) & out_mask;
            tmp_cmp_0 = output ^ d_output_table[cur][0];
            tmp_cmp_1 = output ^ d_output_table[cur][1];
            cost_cnt_0 = (int) (tmp_cmp_0 & d_pun_ptr[(2*ncnt)%d_pun_mod]) + ((tmp_cmp_0>>1) & d_pun_ptr[(2*ncnt+1)%d_pun_mod]);
            cost_cnt_1 = (int) (tmp_cmp_1 & d_pun_ptr[(2*ncnt)%d_pun_mod]) + ((tmp_cmp_1>>1) & d_pun_ptr[(2*ncnt+1)%d_pun_mod]);
            nex0 = cur<<1;
            nex1 = (cur<<1) | 0x01;
            d_cost[0][nex0] = cost_cnt_0;
            d_cost[0][nex1] = (ncnt>nbit-6)? UINT_MAX : cost_cnt_1;
            d_track[0][nex0] = cur;
            d_track[0][nex1] = cur;
            ncnt++;
            if(ncnt == nbit){
                break;
            }
        }
        output = (uvec[ncnt * 2 / 8] >> ( 2 * ncnt % 8)) & out_mask;
        for( nex0 = 0 ; nex0 < CONV_CODE_MAX_STATE ; ++nex0)
        {
            if(( ncnt > nbit - 6 ) && (nex0 & 0x01 ))
            {
                d_cost[ncnt][nex0] = UINT_MAX;
                continue;
            }
            cur = ( nex0 >> 1 );
            tmp_cmp_0 = output ^ d_output_table[cur][nex0 & 0x01];
            uint32_t tmp_state_cost_0 = d_cost[ncnt % FEC_WRAP -1][cur]
                                        + (uint32_t)(
                                            (tmp_cmp_0 & d_pun_ptr[(2*ncnt) % d_pun_mod])
                                            + ( (tmp_cmp_0 >> 1 ) & d_pun_ptr[(2*ncnt+1) % d_pun_mod]) 
                                        );
            cost_cnt_0 = ( d_cost[ncnt % FEC_WRAP - 1 ][cur] == UINT_MAX )? UINT_MAX : tmp_state_cost_0;
            // second case, 1
            nex1 = (nex0 >> 1) | 0x20;
            tmp_cmp_1 = output ^ d_output_table[nex1][nex0 & 0x01];

            uint32_t tmp_state_cost_1 = d_cost[ncnt % FEC_WRAP - 1][nex1] 
                                + (uint32_t)(
                                    (tmp_cmp_1 & d_pun_ptr[(2*ncnt) % d_pun_mod])
                                    + ( (tmp_cmp_1 >> 1) & d_pun_ptr[(2*ncnt +1) % d_pun_mod])
                                );
            cost_cnt_1 = (d_cost[ncnt % FEC_WRAP -1][nex1] == UINT_MAX)? UINT_MAX : tmp_state_cost_1;
            d_cost[ncnt][nex0] = (cost_cnt_0 < cost_cnt_1 )? cost_cnt_0 : cost_cnt_1;
            d_track[ncnt][nex0] = (cost_cnt_0 < cost_cnt_1)? cur : nex1;
        }
        ncnt++;
        if( ncnt % FEC_WRAP ==0)
        {
            // truncated
            // NOTE: use simple function to find the minimum
            //uint32_t min_cost;
            //uint8_t min_idx;
            //_call_minimum_in_buffer(d_cost, min_cost&, min_idx&);
            uint32_t min_cost = d_cost[FEC_WRAP - 1][0];
            uint8_t min_idx = 0;
            for(int i=1; i< CONV_CODE_MAX_STATE; ++i)
            {
                if(d_cost[ FEC_WRAP -1 ][i] < min_cost){
                    min_cost = d_cost[ FEC_WRAP -1 ][i];
                    min_idx = i;
                }
            }
            if(min_cost == UINT_MAX)
            {
                return false;
            }
            // back tracking
            cur = min_idx;
            for(int i=1; i<FEC_WRAP; ++i)
            {
                nex0 = d_track[FEC_WRAP - i][cur];
                d_decode[( nwrite + FEC_WRAP - i) / 8] |= ((cur & 0x01) << (nwrite+ FEC_WRAP -i)%8);
                cur = nex0;
            }
            // last one
            d_decode[nwrite/8] |= ((cur & 0x01) << (nwrite % 8));
            cur = min_idx;
            nwrite += FEC_WRAP;
        }
    }
    //last tracking
    cur = 0;
    for(int i = 1; i < (ncnt % FEC_WRAP); ++i)
    {
        nex0 = d_track[ncnt % FEC_WRAP -i][cur];
        d_decode[(ncnt -i )/8] |= ((cur & 0x01) << ((ncnt-i) % 8));
        cur = nex0;
    }
    // last one
    d_decode[nwrite/8] |= ((cur & 0x01) << (nwrite % 8));
    nwrite += (ncnt % FEC_WRAP);
    return true;
}


} /* namespace wifi_ofdm */
} /* namespace gr */

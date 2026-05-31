/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "coarse_cfo_fixer_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <volk/volk.h>
#include <gnuradio/expj.h>

namespace gr {
namespace wifi_ofdm {

#define TWO_PI 2 * M_PI
//#define d_debug 0
//#define dout d_debug && std::cout
static const int d_cap = 8192*2;
static const float d_threshold = 0.9;
static const int d_escape = 320;
inline void phase_wrap(float& phase){
    while(phase>=TWO_PI)
    phase -= TWO_PI;
    while(phase<=-TWO_PI)
    phase += TWO_PI;
}

//#pragma message("set the following appropriately and remove this warning")
//using input_type = float;
//#pragma message("set the following appropriately and remove this warning")
//using output_type = float;
coarse_cfo_fixer_cc::sptr coarse_cfo_fixer_cc::make(int delay)
{
    return gnuradio::make_block_sptr<coarse_cfo_fixer_cc_impl>(delay);
}


/*
 * The private constructor
 */
coarse_cfo_fixer_cc_impl::coarse_cfo_fixer_cc_impl(int delay)
    : gr::block("coarse_cfo_fixer_cc",
                gr::io_signature::make2(2,2,sizeof(gr_complex),sizeof(gr_complex)),
                gr::io_signature::make(1,1,sizeof(gr_complex))),
                d_delay(delay),
                d_block_name(pmt::intern(alias()))
{
    set_tag_propagation_policy(TPP_DONT);

    set_max_noutput_items(d_cap);
    set_history(delay);
    d_in_x_delay = (gr_complex*)volk_malloc(sizeof(gr_complex) * d_cap, volk_get_alignment());
    d_in_square = (gr_complex*)volk_malloc(sizeof(gr_complex) * d_cap, volk_get_alignment());
    d_delay_square = (gr_complex*)volk_malloc(sizeof(gr_complex) * d_cap, volk_get_alignment());
    // clean up the memory
    // using volk memory set instead
    //std::memset(d_in_x_delay,0,sizeof(gr_complex) * d_cap);
    //std::memset(d_in_square,0,sizeof(gr_complex) * d_cap);
    //std::memset(d_delay_square,0,sizeof(gr_complex) * d_cap);

    d_state = 0;
    d_ncnt = 0;
}

/*
 * Our virtual destructor.
 */
coarse_cfo_fixer_cc_impl::~coarse_cfo_fixer_cc_impl()
{
    volk_free(d_in_x_delay);
    volk_free(d_in_square);
    volk_free(d_delay_square);
}

void coarse_cfo_fixer_cc_impl::forecast(int noutput_items,
                                        gr_vector_int& ninput_items_required)
{
    for(size_t i =0;i<ninput_items_required.size();++i)
        ninput_items_required[i] = noutput_items + history();
}

int coarse_cfo_fixer_cc_impl::general_work(int noutput_items,
                                           gr_vector_int& ninput_items,
                                           gr_vector_const_void_star& input_items,
                                           gr_vector_void_star& output_items)
{
    auto in = static_cast<const gr_complex*>(input_items[0]);
    auto delay = static_cast<const gr_complex*>(input_items[1]);
    auto out = static_cast<gr_complex*>(output_items[0]);

    //gr_complex *out = (gr_complex *) output_items[0];
    int nout = std::max(
                    std::min(
                            noutput_items,
                            std::min(ninput_items[0],ninput_items[1])-(int)history()+1
                        ),0);
    int nin = nout+d_delay-1;
    float tmp_auto;
    if(nout==0)
    {
        consume_each(0);
        return 0;
    }
    volk_32fc_x2_multiply_32fc(d_in_x_delay,delay,in,nin);
    volk_32fc_x2_multiply_conjugate_32fc(d_in_square,in,in,nin);
    volk_32fc_x2_multiply_conjugate_32fc(d_delay_square,delay,delay,nin);
    gr_complex nom(0,0), inpwr(0,0), delpwr(0,0);
    for(int i=0;i<d_delay-1;++i)
    {
        nom += d_in_x_delay[i];
        inpwr += d_in_square[i];
        delpwr += d_delay_square[i];
    }
    int ngen = 0;
    while(ngen<nout)
    {
        nom += d_in_x_delay[d_delay-1+ngen];
        inpwr += d_in_square[d_delay-1+ngen];
        delpwr += d_delay_square[d_delay-1+ngen];
        out[ngen] = in[ngen] * gr_expj(-d_phase); // conj(delay) * in --> cfo
        d_phase += d_coarse_cfo;
        phase_wrap(d_phase);
        tmp_auto = std::abs(nom)/(std::sqrt(std::abs(inpwr*delpwr))+1e-12);
        // main loop
        if(d_state==1)
        {
            d_ncnt--;
            if(d_ncnt == 0)
            {
                d_state = 0;
            }
        }else
        {
            if(tmp_auto>d_threshold && tmp_auto<=1.0f)
            {
                d_ncnt++;
                if(d_ncnt == d_delay)
                {
                    d_state = 1;
                    //dout<<"DEBUG-CoarseCfo: auto val--nom="<<std::abs(nom)<<",inpwr="<<std::abs(inpwr)<<" ,delpwr="<<std::abs(delpwr)<<std::endl;
                    d_ncnt = d_escape;
                    d_phase = 0;
                    d_coarse_cfo = std::arg(nom)/(float)d_delay;
                    add_item_tag(0,nitems_written(0)+ngen,pmt::intern("cfo_est"),pmt::from_float(d_coarse_cfo),d_block_name);
                    //dout<<"DEBUG-CoarseCfo:auto_calc="<<tmp_auto<<" ,cfo_est="<<d_coarse_cfo<<" ,added at:"<<nitems_written(0)+ngen<<std::endl;
                }
            }else
            {
                d_ncnt =0;
            }
        }
        nom -= d_in_x_delay[ngen];
        inpwr -= d_in_square[ngen];
        delpwr -= d_delay_square[ngen];
        ngen++;
    }
    
    consume_each (nout);
    return nout;
    // Do <+signal processing+>
    // Tell runtime system how many input items we consumed on
    // each input stream.
    //consume_each(noutput_items);

    // Tell runtime system how many output items we produced.
    //return noutput_items;
}

} /* namespace wifi_ofdm */
} /* namespace gr */

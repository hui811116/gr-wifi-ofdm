/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_PPDU_BUILDER_H
#define INCLUDED_WIFI_OFDM_PPDU_BUILDER_H

#include <gnuradio/block.h>
#include <gnuradio/wifi_ofdm/api.h>

namespace gr {
namespace wifi_ofdm {

/*!
 * \brief <+description of block+>
 * \ingroup wifi_ofdm
 *
 */
class WIFI_OFDM_API ppdu_builder : virtual public gr::block
{
public:
    typedef std::shared_ptr<ppdu_builder> sptr;

    /*!
     * \brief Construct a 802.11 PPDU
     *
     * To avoid accidental use of raw pointers, wifi_ofdm::ppdu_builder's
     * constructor is in a private implementation
     * class. wifi_ofdm::ppdu_builder::make is the public interface for
     * creating new instances.
     */
    virtual ~ppdu_builder(){};
    static sptr make(int seed, int rate);
    virtual void set_ppdu_rate(int rate)=0;
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_PPDU_BUILDER_H */

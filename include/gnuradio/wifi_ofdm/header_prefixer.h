/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_HEADER_PREFIXER_H
#define INCLUDED_WIFI_OFDM_HEADER_PREFIXER_H

#include <gnuradio/block.h>
#include <gnuradio/wifi_ofdm/api.h>

namespace gr {
namespace wifi_ofdm {

/*!
 * \brief <+description of block+>
 * \ingroup wifi_ofdm
 *
 */
class WIFI_OFDM_API header_prefixer : virtual public gr::block
{
public:
    typedef std::shared_ptr<header_prefixer> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of wifi_ofdm::header_prefixer.
     *
     * To avoid accidental use of raw pointers, wifi_ofdm::header_prefixer's
     * constructor is in a private implementation
     * class. wifi_ofdm::header_prefixer::make is the public interface for
     * creating new instances.
     */
    static sptr make();
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_HEADER_PREFIXER_H */

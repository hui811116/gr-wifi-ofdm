/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_SYMBOL_SYNC_CVC_H
#define INCLUDED_WIFI_OFDM_SYMBOL_SYNC_CVC_H

#include <gnuradio/block.h>
#include <gnuradio/wifi_ofdm/api.h>

namespace gr {
namespace wifi_ofdm {

/*!
 * \brief <+description of block+>
 * \ingroup wifi_ofdm
 *
 */
class WIFI_OFDM_API symbol_sync_cvc : virtual public gr::block
{
public:
    typedef std::shared_ptr<symbol_sync_cvc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of wifi_ofdm::symbol_sync_cvc.
     *
     * To avoid accidental use of raw pointers, wifi_ofdm::symbol_sync_cvc's
     * constructor is in a private implementation
     * class. wifi_ofdm::symbol_sync_cvc::make is the public interface for
     * creating new instances.
     */
    static sptr make();
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_SYMBOL_SYNC_CVC_H */

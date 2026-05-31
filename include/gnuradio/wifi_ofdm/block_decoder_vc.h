/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_BLOCK_DECODER_VC_H
#define INCLUDED_WIFI_OFDM_BLOCK_DECODER_VC_H

#include <gnuradio/sync_block.h>
#include <gnuradio/wifi_ofdm/api.h>

namespace gr {
namespace wifi_ofdm {

/*!
 * \brief <+description of block+>
 * \ingroup wifi_ofdm
 *
 */
class WIFI_OFDM_API block_decoder_vc : virtual public gr::sync_block
{
public:
    typedef std::shared_ptr<block_decoder_vc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of wifi_ofdm::block_decoder_vc.
     *
     * To avoid accidental use of raw pointers, wifi_ofdm::block_decoder_vc's
     * constructor is in a private implementation
     * class. wifi_ofdm::block_decoder_vc::make is the public interface for
     * creating new instances.
     */
    static sptr make();
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_BLOCK_DECODER_VC_H */

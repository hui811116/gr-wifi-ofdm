/* -*- c++ -*- */
/*
 * Copyright 2026 Teng-Hui Huang.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef INCLUDED_WIFI_OFDM_CP_PREAMBLE_PREFIXER_VCC_H
#define INCLUDED_WIFI_OFDM_CP_PREAMBLE_PREFIXER_VCC_H

#include <gnuradio/tagged_stream_block.h>
#include <gnuradio/wifi_ofdm/api.h>

namespace gr {
namespace wifi_ofdm {

/*!
 * \brief <+description of block+>
 * \ingroup wifi_ofdm
 *
 */
class WIFI_OFDM_API cp_preamble_prefixer_vcc : virtual public gr::tagged_stream_block
{
public:
    typedef std::shared_ptr<cp_preamble_prefixer_vcc> sptr;

    /*!
     * \brief Return a shared_ptr to a new instance of wifi_ofdm::cp_preamble_prefixer_vcc.
     *
     * To avoid accidental use of raw pointers, wifi_ofdm::cp_preamble_prefixer_vcc's
     * constructor is in a private implementation
     * class. wifi_ofdm::cp_preamble_prefixer_vcc::make is the public interface for
     * creating new instances.
     */
    static sptr make(const std::string& tagname);
};

} // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_CP_PREAMBLE_PREFIXER_VCC_H */

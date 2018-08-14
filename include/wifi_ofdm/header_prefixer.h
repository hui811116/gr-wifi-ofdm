/* -*- c++ -*- */
/* 
 * Copyright 2018 Teng-Hui Huang.
 * 
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this software; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */


#ifndef INCLUDED_WIFI_OFDM_HEADER_PREFIXER_H
#define INCLUDED_WIFI_OFDM_HEADER_PREFIXER_H

#include <wifi_ofdm/api.h>
#include <gnuradio/block.h>

namespace gr {
  namespace wifi_ofdm {

    /*!
     * \brief <+description+>
     *
     */
    class WIFI_OFDM_API header_prefixer : virtual public block
    {
    public:
      typedef boost::shared_ptr<header_prefixer> sptr;
      static sptr make();
    };

  } // namespace wifi_ofdm
} // namespace gr

#endif /* INCLUDED_WIFI_OFDM_HEADER_PREFIXER_H */


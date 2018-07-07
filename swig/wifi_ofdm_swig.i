/* -*- c++ -*- */

#define WIFI_OFDM_API

%include "gnuradio.i"			// the common stuff

//load generated python docstrings
%include "wifi_ofdm_swig_doc.i"

%{
#include "wifi_ofdm/symbol_parser_vc.h"
#include "wifi_ofdm/packet_builder.h"
#include "wifi_ofdm/header_prefixer.h"
#include "wifi_ofdm/ppdu_builder.h"
#include "wifi_ofdm/decode_header.h"
%}


%include "wifi_ofdm/symbol_parser_vc.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, symbol_parser_vc);
%include "wifi_ofdm/packet_builder.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, packet_builder);
%include "wifi_ofdm/header_prefixer.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, header_prefixer);
%include "wifi_ofdm/ppdu_builder.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, ppdu_builder);
%include "wifi_ofdm/decode_header.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, decode_header);

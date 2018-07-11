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
#include "wifi_ofdm/symbol_mapper_bvc.h"
#include "wifi_ofdm/cp_preamble_prefixer_vcc.h"
#include "wifi_ofdm/coarse_cfo_fixer_cc.h"
#include "wifi_ofdm/symbol_sync_cvc.h"
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
%include "wifi_ofdm/symbol_mapper_bvc.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, symbol_mapper_bvc);
%include "wifi_ofdm/cp_preamble_prefixer_vcc.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, cp_preamble_prefixer_vcc);
%include "wifi_ofdm/coarse_cfo_fixer_cc.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, coarse_cfo_fixer_cc);
%include "wifi_ofdm/symbol_sync_cvc.h"
GR_SWIG_BLOCK_MAGIC2(wifi_ofdm, symbol_sync_cvc);

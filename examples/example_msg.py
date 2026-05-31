#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#
# SPDX-License-Identifier: GPL-3.0
#
# GNU Radio Python Flow Graph
# Title: Not titled yet
# GNU Radio version: 3.10.11.0

from PyQt5 import Qt
from gnuradio import qtgui
from PyQt5.QtCore import QObject, pyqtSlot
from gnuradio import blocks
import pmt
from gnuradio import blocks, gr
from gnuradio import gr
from gnuradio.filter import firdes
from gnuradio.fft import window
import sys
import signal
from PyQt5 import Qt
from argparse import ArgumentParser
from gnuradio.eng_arg import eng_float, intx
from gnuradio import eng_notation
from gnuradio import pdu
from gnuradio import wifi_ofdm
import threading



class example_msg(gr.top_block, Qt.QWidget):

    def __init__(self):
        gr.top_block.__init__(self, "Not titled yet", catch_exceptions=True)
        Qt.QWidget.__init__(self)
        self.setWindowTitle("Not titled yet")
        qtgui.util.check_set_qss()
        try:
            self.setWindowIcon(Qt.QIcon.fromTheme('gnuradio-grc'))
        except BaseException as exc:
            print(f"Qt GUI: Could not set Icon: {str(exc)}", file=sys.stderr)
        self.top_scroll_layout = Qt.QVBoxLayout()
        self.setLayout(self.top_scroll_layout)
        self.top_scroll = Qt.QScrollArea()
        self.top_scroll.setFrameStyle(Qt.QFrame.NoFrame)
        self.top_scroll_layout.addWidget(self.top_scroll)
        self.top_scroll.setWidgetResizable(True)
        self.top_widget = Qt.QWidget()
        self.top_scroll.setWidget(self.top_widget)
        self.top_layout = Qt.QVBoxLayout(self.top_widget)
        self.top_grid_layout = Qt.QGridLayout()
        self.top_layout.addLayout(self.top_grid_layout)

        self.settings = Qt.QSettings("gnuradio/flowgraphs", "example_msg")

        try:
            geometry = self.settings.value("geometry")
            if geometry:
                self.restoreGeometry(geometry)
        except BaseException as exc:
            print(f"Qt GUI: Could not restore geometry: {str(exc)}", file=sys.stderr)
        self.flowgraph_started = threading.Event()

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 32000
        self.rate = rate = 0

        ##################################################
        # Blocks
        ##################################################

        # Create the options list
        self._rate_options = [0, 1, 2, 3, 4, 5, 6, 7]
        # Create the labels list
        self._rate_labels = ['6MBps', '9MBps', '12MBps', '18MBps', '24MBps', '36MBps', '48MBps', '54MBps']
        # Create the combo box
        self._rate_tool_bar = Qt.QToolBar(self)
        self._rate_tool_bar.addWidget(Qt.QLabel("PPDU Rate" + ": "))
        self._rate_combo_box = Qt.QComboBox()
        self._rate_tool_bar.addWidget(self._rate_combo_box)
        for _label in self._rate_labels: self._rate_combo_box.addItem(_label)
        self._rate_callback = lambda i: Qt.QMetaObject.invokeMethod(self._rate_combo_box, "setCurrentIndex", Qt.Q_ARG("int", self._rate_options.index(i)))
        self._rate_callback(self.rate)
        self._rate_combo_box.currentIndexChanged.connect(
            lambda i: self.set_rate(self._rate_options[i]))
        # Create the radio buttons
        self.top_layout.addWidget(self._rate_tool_bar)
        self.wifi_ofdm_ppdu_sink_0 = wifi_ofdm.ppdu_sink()
        self.wifi_ofdm_ppdu_builder_0 = wifi_ofdm.ppdu_builder(255, rate)
        self.wifi_ofdm_header_prefixer_0 = wifi_ofdm.header_prefixer()
        self.pdu_random_pdu_0 = pdu.random_pdu(10, 25, 0xFF, 2)
        self.blocks_message_strobe_0 = blocks.message_strobe(pmt.intern("TEST"), 5000)
        self.blocks_message_debug_0 = blocks.message_debug(True, gr.log_levels.info)


        ##################################################
        # Connections
        ##################################################
        self.msg_connect((self.blocks_message_strobe_0, 'strobe'), (self.pdu_random_pdu_0, 'generate'))
        self.msg_connect((self.pdu_random_pdu_0, 'pdus'), (self.wifi_ofdm_header_prefixer_0, 'psdu_in'))
        self.msg_connect((self.wifi_ofdm_header_prefixer_0, 'plcp_out'), (self.wifi_ofdm_ppdu_builder_0, 'plcp_in'))
        self.msg_connect((self.wifi_ofdm_ppdu_builder_0, 'ppdu_out'), (self.wifi_ofdm_ppdu_sink_0, 'ppdu_in'))
        self.msg_connect((self.wifi_ofdm_ppdu_sink_0, 'plcp_out'), (self.blocks_message_debug_0, 'print'))


    def closeEvent(self, event):
        self.settings = Qt.QSettings("gnuradio/flowgraphs", "example_msg")
        self.settings.setValue("geometry", self.saveGeometry())
        self.stop()
        self.wait()

        event.accept()

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate

    def get_rate(self):
        return self.rate

    def set_rate(self, rate):
        self.rate = rate
        self._rate_callback(self.rate)
        self.wifi_ofdm_ppdu_builder_0.set_ppdu_rate(self.rate)




def main(top_block_cls=example_msg, options=None):

    qapp = Qt.QApplication(sys.argv)

    tb = top_block_cls()

    tb.start()
    tb.flowgraph_started.set()

    tb.show()

    def sig_handler(sig=None, frame=None):
        tb.stop()
        tb.wait()

        Qt.QApplication.quit()

    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)

    timer = Qt.QTimer()
    timer.start(500)
    timer.timeout.connect(lambda: None)

    qapp.exec_()

if __name__ == '__main__':
    main()

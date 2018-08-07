# wifi-ofdm
A Software-Defined Radio implementation of IEEE 802.11a phisical layer on Gnuradio

# Installation

- prerequisite:

    gr-gadget

- install gr-wifi-dsss:
```
cd gr-wifi-ofdm/
mkdir build
cd build
cmake ..
make
sudo make install
```

# Quick start 

`cd examples` and run the scripts in the folder. 

# Examples
- 'wifi_ofdm_usrp_tx.grc': Transmitter script. Require one USRP connected to your computer.
- 'wifi_ofdm_usrp_rx.grc': Receiver script. Also require one USRP connected.

# Supported Rates
The prototype supports the following data rates:
- 6  Mbps
- 9  Mbps
- 12 Mbps
- 18 Mbps
- 24 Mbps
- 36 Mbps
- 48 Mbps
- 54 Mbps

# Notes
For convenience, the current version of the prototype use hard-decision decoding in the convolutional-codes decoder 
and may therefore suffers from several dB bit-error-rate (BER) loss in low signal-to-noise ratio (SNR) regime.

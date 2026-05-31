# wifi-ofdm
A Software-Defined Radio implementation of IEEE 802.11a physical layer on Gnuradio

**Note:** The project now supports Gnuradio 3.10 (Volk 3.1.2 is tested). 
If you are using an earlier version (Gnuradio < 3.7), you can checkout to the `legacy` branch:
```bash
git checkout legacy
```

# Demo

[![gr-wifi-ofdm demo video](http://img.youtube.com/vi/wYjP9WzbRcw/0.jpg)](http://www.youtube.com/watch?v=wYjP9WzbRcw)

# Installation

- prerequisite:

    gr-gadget
    Gnuradio 3.10
    Volk (3.1.2 is tested)

- install Gnuradio 3.10
    Please refer to the [GNU Radio Installation Guide](https://wiki.gnuradio.org/index.php/InstallingGR).
    Radioconda is recommended as this is where the project is developed.

- install gr-wifi-ofdm:
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
- 'wifi_ofdm_usrp_tx.grc': Transmitter script. Requires one USRP connected to your computer.
- 'wifi_ofdm_usrp_rx.grc': Receiver script. Also requires one USRP connected.

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
For convenience, the current version of the prototype uses hard-decision decoding in the convolutional-codes decoder 
and may therefore suffer from several dB bit-error-rate (BER) loss in low signal-to-noise ratio (SNR) regime.
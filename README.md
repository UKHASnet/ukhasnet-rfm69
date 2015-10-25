# ukhasnet-rfm69
An RFM69 library maintained by and for the UKHASnet network

## Installation on Arduino

## From Latest Release

Download the Latest Arduino Release .zip from the Releases Tab on Github.

In the Arduino IDE go to the 'Sketch' menu => 'Include Library' => 'Add .ZIP Library...'

Browse to and select the .zip you downloaded. This will add the library to your project.

## From Current Source

Alternatively git clone this repository into a folder call 'UKHASnetRFM69' with:

`git clone -b arduino https://github.com/UKHASnet/ukhasnet-rfm69.git UKHASnetRFM69`

Create a .zip file of the folder.

In the Arduino IDE go to the 'Sketch' menu => 'Include Library' => 'Add .ZIP Library...'

Browse to and select the .zip you created. This will add the library to your project.

## Usage

rfm_status_t rf69_init(void);

rfm_status_t rf69_read_temp(int8_t* temperature);

rfm_status_t rf69_receive(rfm_reg_t* buf, rfm_reg_t* len, int16_t* lastrssi, bool* rfm_packet_waiting);
        
rfm_status_t rf69_send(const rfm_reg_t* data, uint8_t len, const uint8_t power);

rfm_status_t rf69_set_mode(const rfm_reg_t newMode);

## Information

This library is maintained for use in the [UKHASnet](http://ukhas.net) low
power sensor network, whose canonical node formulation uses the RFM69 device.

Created and released into the Public Domain by [Jon Sowman](http://github.com/jonsowman) 2015.

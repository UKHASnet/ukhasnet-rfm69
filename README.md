# ukhasnet-rfm69
An RFM69 library maintained by and for the UKHASnet network

## Installation

The best way to use this library is to add it as a git submodule to your
repository. To do this, `cd` to the directory in which your firmware is
located, and run:  

`git submodule add git@github.com:UKHASnet/ukhasnet-rfm69.git ukhasnet-rfm69`

This will create a directory called `ukhasnet-rfm69` in your project directory.  

The library is built as part of your firmware, rather than separately. 

## Usage

The library is hardware agnostic. The files `spi_conf.c` and `spi_conf.h`
should be created by you, the user, which define device specific functions 
that allow the library to communicate with the RFM69 module.  

A generic example of these files can be found in `spi_conf/` and
device-specific examples in that folder. For example, a working SPI driver for
the ATMEGA168 can be found in `spi_conf/atmega168/`.  

1. Ensure the `ukhasnet-rfm69/` directory is in your include path (-I
   for gcc-type compilers).
2. `#include "ukhasnet-rfm69.h"` in your firmware.
3. Populate the blank `spi_conf.c` or copy an existing one for your hardware
   into your firmware directory.

## Updating

To update the library, `cd` into the `ukhasnet-rfm69` library directory and run
`git pull`. Building your firmware will automatically build the new version of
the library.  

## Information

This library is maintained for use in the [UKHASnet](http://ukhas.net) low
power sensor network, whose canonical node formulation uses the RFM69 device.  

Created and released into the Public Domain by [Jon
Sowman](http://github.com/jonsowman) 2015.  

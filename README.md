# ukhasnet-rfm69
An RFM69 library maintained by and for the UKHASnet network

## Installation

To begin with, you need a working AVR toolchain. To check you have this, run
`avr-gcc --version` and check you get some useful output.  

The best way to use this library is to add it as a git submodule to your
repository. To do this, `cd` to the directory in which your sensor firmware is
located, and run:  

`git submodule add git@github.com:UKHASnet/ukhasnet-rfm69.git ukhasnet-rfm69`

This will create a directory called `ukhasnet-rfm69` in your project directory.  

To build the library, first open Makefile in the `ukhasnet-rfm69` directory,
and adjust the settings to the AVR part you are using (DEVICE, e.g. m168) and
the clock speed in Hertz (CLOCK, e.g. 8000000 for 8MHz).  

Run `make` to build the library. This will create `libukhasnet-rfm69.a` in this
directory.  

## Usage

The library is hardware agnostic. You will now need to configure `spi_conf.c`
to define the device specific functions that allow the library to communicate
with the RFM69 module. These are the SPI device driver functions and are
described in the file. 

## Updating

To update the library, `cd` into the `ukhasnet-rfm69` library directory and run
`git pull`. You will then need to run `make` again to build the new version of
the library.  

## Information

This library is maintained for use in the [UKHASnet](http://ukhas.net) low
power sensor network, whose canonical node formulation uses the RFM69 device.  

Created and released into the Public Domain by [Jon
Sowman](http://github.com/jonsowman) 2015.  

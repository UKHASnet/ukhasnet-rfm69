# Name: UKHASNet RFM69 Library
# Project: AVR UKHASNet Project
# Author: Jon Sowman <jon@jonsowman.com> 
# Modified Auguust 2015

# Configure the following settings according to the device in use
DEVICE     = m168
CLOCK      = 1000000
SOURCES	   = $(wildcard *.c)

# Library name
RFMLIB = ukhasnet-rfm69

# End configuration
OBJECTS = $(SOURCES:.c=.o)

# Compiler flags. Optimise for code size. Allow C99 standards.
COMPILE = avr-gcc -Wall -Os -gdwarf-2 -std=gnu99 -DF_CPU=$(CLOCK) -mmcu=atmega168 -c -o $(OBJECTS)
AR = avr-ar -r lib$(RFMLIB).a

# symbolic targets:
all: 	$(OBJECTS)
	$(AR) $(OBJECTS)

.c.o:
	$(COMPILE) -c $< -o $@ -lrfm69

clean:
	rm -f lib$(RFMLIB).a $(OBJECTS)

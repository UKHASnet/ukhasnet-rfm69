/**
 * RFM69.c
 *
 * This file is part of the UKHASNet (ukhas.net) maintained RFM69 library for
 * use with all UKHASnet nodes, including Arduino, AVR and ARM.
 *
 * Ported to Arduino 2014 James Coxon
 * Ported, tidied and hardware abstracted by Jon Sowman, 2015
 *
 * Copyright (C) 2014 Phil Crump
 * Copyright (C) 2015 Jon Sowman <jon@jonsowman.com>
 *
 * Based on RF22 Copyright (C) 2011 Mike McCauley
 * Ported to mbed by Karl Zweimueller
 *
 * Based on RFM69 LowPowerLabs (https://github.com/LowPowerLab/RFM69/)
 */

#include <avr/io.h>
#include <util/delay.h>

#include "ukhasnet-rfm69.h"
#include "ukhasnet-rfm69-config.h"

/** Track the current mode of the radio */
static uint8_t _mode;

/**
 * Initialise the RFM69 device.
 * @returns 0 on failure, nonzero on success
 */
bool rf69_init(void)
{
    uint8_t i;

    // Set up device
    for(i = 0; CONFIG[i][0] != 255; i++)
        rf69_spiWrite(CONFIG[i][0], CONFIG[i][1]);
    
    /* Set initial mode */
    _mode = RFM69_MODE_RX;
    rf69_setMode(_mode);

    // Zero version number, RFM probably not connected/functioning
    if(!rf69_spiRead(RFM69_REG_10_VERSION))
        return false;

    return true;
}

/**
 * Read a single byte from a register in the RFM69. Transmit the (one byte)
 * address of the register to be read, then read the (one byte) response.
 * @param reg The register address to be read
 * @returns The value of the register
 */
uint8_t rf69_spiRead(const uint8_t reg)
{
    uint8_t data;

    spi_ss_assert();

    /* Transmit the reg we want to read from */
    data = spi_exchange_single(reg);

    /* Read the data back */
    data = spi_exchange_single(0xFF);

    spi_ss_deassert();

    return data;
}

/**
 * Write a single byte to a register in the RFM69. Transmit the register
 * address (one byte) with the write mask RFM_SPI_WRITE_MASK on, and then the
 * value of the register to be written.
 * @param reg The address of the register to write
 * @param val The value for the address
 */
void rf69_spiWrite(const uint8_t reg, const uint8_t val)
{
    spi_ss_assert();

    /* Transmit the reg address */
    spi_exchange_single(reg | RFM69_SPI_WRITE_MASK);

    /* Transmit the value for this address */
    spi_exchange_single(val);

    spi_ss_deassert();
}

/**
 * Read a given number of bytes from the given register address into a provided
 * buffer
 * @param reg The address of the register to start from
 * @param dest A pointer into the destination buffer
 * @param len The number of bytes to read
 */
void rf69_spiBurstRead(const uint8_t reg, uint8_t* dest, uint8_t len)
{
    spi_ss_assert();
    
    // Send the start address with the write mask off
    spi_exchange_single(reg & ~RFM69_SPI_WRITE_MASK);
    
    while(len--)
        *dest++ = spi_exchange_single(0xFF);

    spi_ss_deassert();
}

/**
 * Write a given number of bytes into the registers in the RFM69.
 * @param reg The first byte address into which to write
 * @param src A pointer into the source data buffer
 * @param len The number of bytes to write
 */
void rf69_spiBurstWrite(uint8_t reg, const uint8_t* src, uint8_t len)
{
    spi_ss_assert();
    
    // Send the start address with the write mask on
    spi_exchange_single(reg | RFM69_SPI_WRITE_MASK); 

    while(len--)
        spi_exchange_single(*src++);

    spi_ss_deassert();
}

/**
 * Write data into the FIFO on the RFM69
 * @param src The source data comes from this buffer
 * @param len Write this number of bytes from the buffer into the FIFO
 */
void rf69_spiFifoWrite(const uint8_t* src, uint8_t len)
{
    spi_ss_assert();
    
    // Send the start address with the write mask on
    spi_exchange_single(RFM69_REG_00_FIFO | RFM69_SPI_WRITE_MASK);
    
    // First byte is packet length
    spi_exchange_single(len);

    // Then write the packet
    while(len--)
        spi_exchange_single(*src++);

    spi_ss_deassert();
}

/**
 * Change the RFM69 operating mode to a new one.
 * @param newMode The value representing the new mode (see datasheet for
 * further information).
 */
void rf69_setMode(const uint8_t newMode)
{
    rf69_spiWrite(RFM69_REG_01_OPMODE, 
            (rf69_spiRead(RFM69_REG_01_OPMODE) & 0xE3) | newMode);
    _mode = newMode;
}

/**
 * Get data from the RFM69 receive buffer.
 * @param buf A pointer into the local buffer in which we would like the data.
 * @param len The length of the data
 * @param lastrssi The RSSI of the packet we're getting
 * @returns true on success, false is there's no packet waiting.
 */
bool rf69_receive(uint8_t* buf, uint8_t* len, int16_t* lastrssi)
{
    // Check IRQ register for payloadready flag (indicates RXed packet waiting in FIFO)
    if(rf69_spiRead(RFM69_REG_28_IRQ_FLAGS2) & RF_IRQFLAGS2_PAYLOADREADY) {
        // Get packet length from first byte of FIFO
        *len = rf69_spiRead(RFM69_REG_00_FIFO)+1;
        // Read FIFO into our Buffer
        rf69_spiBurstRead(RFM69_REG_00_FIFO, buf, RFM69_FIFO_SIZE);
        // Read RSSI register (should be of the packet? - TEST THIS)
        *lastrssi = -(rf69_spiRead(RFM69_REG_24_RSSI_VALUE)/2);
        // Clear the radio FIFO (found in HopeRF demo code)
        rf69_clearFifo();
        return true;
    }
    
    return false;
}

/**
 * Send a packet using the RFM69 radio.
 * @param data The data buffer that contains the string to transmit
 * @param len The number of bytes in the data packet (excluding preamble, sync
 * and checksum)
 * @param power The transmit power to be used in dBm
 */
void rf69_send(const uint8_t* data, uint8_t len, uint8_t power)
{
    uint8_t oldMode, paLevel;
    // power is TX Power in dBmW (valid values are 2dBmW-20dBmW)
    if(power < 2 || power > 20)
    {
        // Could be dangerous, so let's check this
        return;
    }

    oldMode = _mode;
    
    // Start Transmitter
    rf69_setMode(RFM69_MODE_TX);

    // Set up PA
    if(power <= 17)
    {
        // Set PA Level
        paLevel = power + 28;
	rf69_spiWrite(RFM69_REG_11_PA_LEVEL, RF_PALEVEL_PA0_ON | RF_PALEVEL_PA1_OFF | RF_PALEVEL_PA2_OFF | paLevel);        
    } else {
        // Disable Over Current Protection
        rf69_spiWrite(RFM69_REG_13_OCP, RF_OCP_OFF);
        // Enable High Power Registers
        rf69_spiWrite(RFM69_REG_5A_TEST_PA1, 0x5D);
        rf69_spiWrite(RFM69_REG_5C_TEST_PA2, 0x7C);
        // Set PA Level
        paLevel = power + 11;
	rf69_spiWrite(RFM69_REG_11_PA_LEVEL, RF_PALEVEL_PA0_OFF | RF_PALEVEL_PA1_ON | RF_PALEVEL_PA2_ON | paLevel);
    }

    // Wait for PA ramp-up
    while(!(rf69_spiRead(RFM69_REG_27_IRQ_FLAGS1) & RF_IRQFLAGS1_TXREADY));

    // Throw Buffer into FIFO, packet transmission will start automatically
    rf69_spiFifoWrite(data, len);

    // Wait for packet to be sent
    while(!(rf69_spiRead(RFM69_REG_28_IRQ_FLAGS2) & RF_IRQFLAGS2_PACKETSENT));

    // Return Transceiver to original mode
    rf69_setMode(oldMode);

    // If we were in high power, switch off High Power Registers
    if(power > 17)
    {
        // Disable High Power Registers
        rf69_spiWrite(RFM69_REG_5A_TEST_PA1, 0x55);
        rf69_spiWrite(RFM69_REG_5C_TEST_PA2, 0x70);
        // Enable Over Current Protection
        rf69_spiWrite(RFM69_REG_13_OCP, RF_OCP_ON | RF_OCP_TRIM_95);
    }
}

/*void RFM69::SetLnaMode(uint8_t lnaMode) {*/
    /*// RF_TESTLNA_NORMAL (default)*/
    /*// RF_TESTLNA_SENSITIVE*/
    /*spiWrite(RFM69_REG_58_TEST_LNA, lnaMode);*/
/*}*/

/**
 * Clear the FIFO in the RFM69. We do this by entering STBY mode and then
 * returing to RX mode.
 * @warning Must only be called in RX Mode
 * @note Apparently this works... found in HopeRF demo code
 */
void rf69_clearFifo(void)
{
    rf69_setMode(RFM69_MODE_STDBY);
    rf69_setMode(RFM69_MODE_RX);
}

/**
 * The RFM69 has an onboard temperature sensor, read its value
 * @warning RFM69 must be in one of the active modes for temp sensor to work.
 * @returns The temperature in degrees C or 255.0 for failure
 */
int8_t rf69_readTemp(void)
{
    // Store current transceiver mode
    uint8_t oldMode, rawTemp, timeout;
    
    oldMode = _mode;
    // Set mode into Standby (required for temperature measurement)
    rf69_setMode(RFM69_MODE_STDBY);

    // Trigger Temperature Measurement
    rf69_spiWrite(RFM69_REG_4E_TEMP1, RF_TEMP1_MEAS_START);

    // Check Temperature Measurement has started
    timeout = 0;
    while(!(RF_TEMP1_MEAS_RUNNING & rf69_spiRead(RFM69_REG_4E_TEMP1)))
    {
        _delay_ms(1);
        if(++timeout > 50)
            return -127.0;
        rf69_spiWrite(RFM69_REG_4E_TEMP1, RF_TEMP1_MEAS_START);
    }

    // Wait for Measurement to complete
    timeout = 0;
    while(RF_TEMP1_MEAS_RUNNING & rf69_spiRead(RFM69_REG_4E_TEMP1))
    {
        _delay_ms(1);
        if(++timeout > 10)
            return -127.0;
    }

    // Read raw ADC value
    rawTemp = rf69_spiRead(RFM69_REG_4F_TEMP2);
	
    // Set transceiver back to original mode
    rf69_setMode(oldMode);

    // Return processed temperature value
    return 161 - (int8_t)rawTemp;
}

/**
 * Get the last RSSI value from the RFM69
 * @warning Must only be called when the RFM69 is in rx mode
 * @returns The last RSSI in some units, or 0 for failure
 */
int16_t rf69_sampleRssi(void)
{
    int16_t lastRssi;

    // Must only be called in RX mode
    if(_mode != RFM69_MODE_RX)
        return 0;

    // Trigger RSSI Measurement
    rf69_spiWrite(RFM69_REG_23_RSSI_CONFIG, RF_RSSI_START);

    // Wait for Measurement to complete
    while(!(RF_RSSI_DONE & rf69_spiRead(RFM69_REG_23_RSSI_CONFIG)));

    // Read, store in _lastRssi and return RSSI Value
    lastRssi = -(rf69_spiRead(RFM69_REG_24_RSSI_VALUE)/2);

    return lastRssi;
}

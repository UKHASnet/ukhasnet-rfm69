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
static rfm_reg_t _mode;

/**
 * Initialise the RFM69 device.
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_init(void)
{
    uint8_t i;
    rfm_reg_t res;

    // Set up device
    for(i = 0; CONFIG[i][0] != 255; i++)
        rf69_spiWrite(CONFIG[i][0], CONFIG[i][1]);
    
    /* Set initial mode */
    _mode = RFM69_MODE_RX;
    rf69_setMode(_mode);

    // Zero version number, RFM probably not connected/functioning
    rf69_spiRead(RFM69_REG_10_VERSION, &res);
    if(!res)
        return RFM_FAIL;

    return RFM_OK;
}

/**
 * Read a single byte from a register in the RFM69. Transmit the (one byte)
 * address of the register to be read, then read the (one byte) response.
 * @param reg The register address to be read
 * @param result A pointer to where to put the result
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_spiRead(const rfm_reg_t reg, rfm_reg_t* result)
{
    rfm_reg_t data;

    spi_ss_assert();

    /* Transmit the reg we want to read from */
    spi_exchange_single(reg, &data);

    /* Read the data back */
    spi_exchange_single(0xFF, result);

    spi_ss_deassert();

    return RFM_OK;
}

/**
 * Write a single byte to a register in the RFM69. Transmit the register
 * address (one byte) with the write mask RFM_SPI_WRITE_MASK on, and then the
 * value of the register to be written.
 * @param reg The address of the register to write
 * @param val The value for the address
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_spiWrite(const rfm_reg_t reg, const rfm_reg_t val)
{
    rfm_reg_t dummy;

    spi_ss_assert();

    /* Transmit the reg address */
    spi_exchange_single(reg | RFM69_SPI_WRITE_MASK, &dummy);

    /* Transmit the value for this address */
    spi_exchange_single(val, &dummy);

    spi_ss_deassert();

    return RFM_OK;
}

/**
 * Read a given number of bytes from the given register address into a 
 * provided buffer
 * @param reg The address of the register to start from
 * @param dest A pointer into the destination buffer
 * @param len The number of bytes to read
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_spiBurstRead(const rfm_reg_t reg, rfm_reg_t* dest, 
        uint8_t len)
{
    rfm_reg_t dummy;

    spi_ss_assert();
    
    // Send the start address with the write mask off
    spi_exchange_single(reg & ~RFM69_SPI_WRITE_MASK, &dummy);
    
    while(len--)
    {
        spi_exchange_single(0xFF, dest);
        dest++;
    }

    spi_ss_deassert();

    return RFM_OK;
}

/**
 * Write a given number of bytes into the registers in the RFM69.
 * @param reg The first byte address into which to write
 * @param src A pointer into the source data buffer
 * @param len The number of bytes to write
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_spiBurstWrite(rfm_reg_t reg, const rfm_reg_t* src, 
        uint8_t len)
{
    rfm_reg_t dummy;

    spi_ss_assert();
    
    // Send the start address with the write mask on
    spi_exchange_single(reg | RFM69_SPI_WRITE_MASK, &dummy); 

    while(len--)
        spi_exchange_single(*src++, &dummy);

    spi_ss_deassert();

    return RFM_OK;
}

/**
 * Write data into the FIFO on the RFM69
 * @param src The source data comes from this buffer
 * @param len Write this number of bytes from the buffer into the FIFO
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_spiFifoWrite(const rfm_reg_t* src, uint8_t len)
{
    rfm_reg_t dummy;

    spi_ss_assert();
    
    // Send the start address with the write mask on
    spi_exchange_single(RFM69_REG_00_FIFO | RFM69_SPI_WRITE_MASK, &dummy);
    
    // First byte is packet length
    spi_exchange_single(len, &dummy);

    // Then write the packet
    while(len--)
        spi_exchange_single(*src++, &dummy);

    spi_ss_deassert();

    return RFM_OK;
}

/**
 * Change the RFM69 operating mode to a new one.
 * @param newMode The value representing the new mode (see datasheet for
 * further information). The MODE bits are masked in the register, i.e. only
 * bits 2-4 of newMode are ovewritten in the register.
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_setMode(const rfm_reg_t newMode)
{
    rfm_reg_t res;
    rf69_spiRead(RFM69_REG_01_OPMODE, &res);
    rf69_spiWrite(RFM69_REG_01_OPMODE, (res & 0xE3) | newMode);
    _mode = newMode;
    return RFM_OK;
}

/**
 * Get data from the RFM69 receive buffer.
 * @param buf A pointer into the local buffer in which we would like the data.
 * @param len The length of the data
 * @param lastrssi The RSSI of the packet we're getting
 * @param rfm_packet_waiting A boolean pointer which is true if a packet was
 * received and has been put into the buffer buf, false if there was no packet
 * to get from the RFM69.
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_receive(rfm_reg_t* buf, rfm_reg_t* len, int16_t* lastrssi,
        bool* rfm_packet_waiting)
{
    rfm_reg_t res;

    // Check IRQ register for payloadready flag 
    // (indicates RXed packet waiting in FIFO)
    rf69_spiRead(RFM69_REG_28_IRQ_FLAGS2, &res);
    if(res & RF_IRQFLAGS2_PAYLOADREADY) {
        // Get packet length from first byte of FIFO
        rf69_spiRead(RFM69_REG_00_FIFO, len);
        *len += 1;
        // Read FIFO into our Buffer
        rf69_spiBurstRead(RFM69_REG_00_FIFO, buf, RFM69_FIFO_SIZE);
        // Read RSSI register (should be of the packet? - TEST THIS)
        rf69_spiRead(RFM69_REG_24_RSSI_VALUE, &res);
        *lastrssi = -(res/2);
        // Clear the radio FIFO (found in HopeRF demo code)
        rf69_clearFifo();
        *rfm_packet_waiting = true;
        return RFM_OK;
    }
    else
    {
        *rfm_packet_waiting = false;
        return RFM_OK;
    }
    
    return RFM_FAIL;
}

/**
 * Send a packet using the RFM69 radio.
 * @param data The data buffer that contains the string to transmit
 * @param len The number of bytes in the data packet (excluding preamble, sync
 * and checksum)
 * @param power The transmit power to be used in dBm
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_send(const rfm_reg_t* data, uint8_t len, 
        const uint8_t power)
{
    rfm_reg_t oldMode, res;
    uint8_t paLevel;

    // power is TX Power in dBmW (valid values are 2dBmW-20dBmW)
    if(power < 2 || power > 20)
    {
        // Could be dangerous, so let's check this
        return RFM_FAIL;
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
    res = 0;
    while(!(res & RF_IRQFLAGS1_TXREADY))
        rf69_spiRead(RFM69_REG_27_IRQ_FLAGS1, &res);

    // Throw Buffer into FIFO, packet transmission will start automatically
    rf69_spiFifoWrite(data, len);

    // Wait for packet to be sent
    res = 0;
    while(!(res & RF_IRQFLAGS2_PACKETSENT))
        rf69_spiRead(RFM69_REG_28_IRQ_FLAGS2, &res);

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

    return RFM_OK;
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
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_clearFifo(void)
{
    rf69_setMode(RFM69_MODE_STDBY);
    rf69_setMode(RFM69_MODE_RX);
    return RFM_OK;
}

/**
 * The RFM69 has an onboard temperature sensor, read its value
 * @warning RFM69 must be in one of the active modes for temp sensor to work.
 * @param temp The temperature in degrees C
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_readTemp(int8_t* temperature)
{
    // Store current transceiver mode
    rfm_reg_t oldMode, temp;
    uint8_t timeout;
    
    oldMode = _mode;
    // Set mode into Standby (required for temperature measurement)
    rf69_setMode(RFM69_MODE_STDBY);

    // Trigger Temperature Measurement
    rf69_spiWrite(RFM69_REG_4E_TEMP1, RF_TEMP1_MEAS_START);

    // Check Temperature Measurement has started
    timeout = 0;
    temp = 0;
    while(!(RF_TEMP1_MEAS_RUNNING & temp))
    {
        rf69_spiRead(RFM69_REG_4E_TEMP1, &temp);
        _delay_ms(1);
        if(++timeout > 50)
        {
            *temperature = -127.0;
            return RFM_TIMEOUT;
        }
        rf69_spiWrite(RFM69_REG_4E_TEMP1, RF_TEMP1_MEAS_START);
    }

    // Wait for Measurement to complete
    timeout = 0;
    temp = 0;
    while(RF_TEMP1_MEAS_RUNNING & temp)
    {
        rf69_spiRead(RFM69_REG_4E_TEMP1, &temp);
        _delay_ms(1);
        if(++timeout > 10)
        {
            *temperature = -127.0;
            return RFM_TIMEOUT;
        }
    }

    // Read raw ADC value
    temp = 0;
    rf69_spiRead(RFM69_REG_4F_TEMP2, &temp);
	
    // Set transceiver back to original mode
    rf69_setMode(oldMode);

    // Return processed temperature value
    *temperature = 161 - (int8_t)temp;

    return RFM_OK;
}

/**
 * Get the last RSSI value from the RFM69
 * @warning Must only be called when the RFM69 is in rx mode
 * @param rssi A pointer to an int16_t where we will place the RSSI value
 * @returns RFM_OK for success, RFM_FAIL for failure.
 */
rfm_status_t rf69_sampleRssi(int16_t* rssi)
{
    rfm_reg_t res;

    // Must only be called in RX mode
    if(_mode != RFM69_MODE_RX)
        return 0;

    // Trigger RSSI Measurement
    rf69_spiWrite(RFM69_REG_23_RSSI_CONFIG, RF_RSSI_START);

    // Wait for Measurement to complete
    while(!(RF_RSSI_DONE & res))
        rf69_spiRead(RFM69_REG_23_RSSI_CONFIG, &res);

    // Read, store in _lastRssi and return RSSI Value
    res = 0;
    rf69_spiRead(RFM69_REG_24_RSSI_VALUE, &res);
    *rssi = -(res/2);

    return RFM_OK;
}

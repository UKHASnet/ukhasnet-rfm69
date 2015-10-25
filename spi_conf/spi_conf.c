/**
 * spi_conf.c
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

#include "ukhasnet-rfm69.h"
#include "spi_conf.h"

/**
 * User SPI setup function. Use this function to set up the SPI peripheral
 * on the microcontroller, such as to setup the IO, set the mode (0,0) for the
 * RFM69, and become a master.
 * @returns RFM_OK on success, RFM_FAIL or RFM_TIMEOUT on failure
 */
rfm_status_t spi_init(void)
{
    /* Set up the SPI peripheral */

    /*
     * You should return RFM_OK if everything went well, otherwise return
     * RFM_FAIL or RFM_TIMEOUT to signal that something went wrong.
     * */
    return RFM_OK;
}

/**
 * User function to exchange a single byte over the SPI interface
 * @warn This does not handle SS, since higher level functions might want to do
 * burst read and writes
 * @param out The byte to be sent
 * @param in A pointer into which we place the returned value
 * @returns RFM_OK on success, RFM_FAIL or RFM_TIMEOUT on failure
 */
rfm_status_t spi_exchange_single(const rfm_reg_t out, rfm_reg_t* in)
{
    /* Insert code to send a byte and receive a byte at the same time */

    /*
     * You should return RFM_OK if everything went well, otherwise return
     * RFM_FAIL or RFM_TIMEOUT to signal that something went wrong.
     * */
    return RFM_OK;
}

/**
 * User function to assert the slave select pin
 * @returns RFM_OK on success, RFM_FAIL or RFM_TIMEOUT on failure
 */
rfm_status_t spi_ss_assert(void)
{
    /* Insert code to assert the SS line */

    /*
     * You should return RFM_OK if everything went well, otherwise return
     * RFM_FAIL or RFM_TIMEOUT to signal that something went wrong.
     * */
    return RFM_OK;
}

/**
 * User function to deassert the slave select pin
 * @returns RFM_OK on success, RFM_FAIL or RFM_TIMEOUT on failure
 */
rfm_status_t spi_ss_deassert(void)
{
    /* Insert code to deassert the SS line */

    /*
     * You should return RFM_OK if everything went well, otherwise return
     * RFM_FAIL or RFM_TIMEOUT to signal that something went wrong.
     * */
    return RFM_OK;
}


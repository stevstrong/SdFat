/**
 * Copyright (c) 20011-2017 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#if defined(__STM32F1__) || defined(__STM32F4__)
#include "SdSpiDriver.h"
//------------------------------------------------------------------------------
#if BOARD_NR_SPI > 3
#error BOARD_NR_SPI too large
#endif
//------------------------------------------------------------------------------
/** Set SPI options for access to SD/SDHC cards.
 *
 * \param[in] divisor SCK clock divider relative to the APB1 or APB2 clock.
 */
void SdSpiAltDriver::activate() {
  m_spi->beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
/** Initialize the SPI bus.
 *
 * \param[in] chipSelectPin SD card chip select pin.
 */
void SdSpiAltDriver::begin(uint8_t csPin) {
  m_csPin = csPin;
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  m_spi->begin();
}
//------------------------------------------------------------------------------
/**
 * End SPI transaction.
 */
void SdSpiAltDriver::deactivate() {
  m_spi->endTransaction();
}
//------------------------------------------------------------------------------
/** Receive a byte.
 *
 * \return The byte.
 */
uint8_t SdSpiAltDriver::receive() {
  return m_spi->transfer(0XFF);
}
//------------------------------------------------------------------------------
/** Receive multiple bytes.
 *
 * \param[out] buf Buffer to receive the data.
 * \param[in] n Number of bytes to receive.
 *
 * \return Zero for no error or nonzero error code.
 */
uint8_t SdSpiAltDriver::receive(uint8_t* buf, size_t n) {
//	Serial.print("SdSpiAltDriver::receive "); Serial.print((uint32)buf, HEX); Serial.print("::"); Serial.println(n);
#if USE_STM32_DMA
if (n>250)
  m_spi->dmaTransfer((uint8_t)0x00, buf, n);
else
  m_spi->read(buf, n);
#else  // USE_STM32_DMA
  m_spi->read(buf, n);
#endif  // USE_STM32_DMA
  return 0;
}
//------------------------------------------------------------------------------
/** Send a byte.
 *
 * \param[in] b Byte to send
 */
void SdSpiAltDriver::send(uint8_t b) {
  m_spi->write(b);
}
//------------------------------------------------------------------------------
/** Send multiple bytes.
 *
 * \param[in] buf Buffer for data to be sent.
 * \param[in] n Number of bytes to send.
 */
void SdSpiAltDriver::send(const uint8_t* buf , size_t n) {
#if USE_STM32_DMA
if (n>250)
  m_spi->dmaSend(buf, n);
else
  m_spi->write(buf, n);
#else  // USE_STM32_DMA
  m_spi->write(buf, n);
#endif  // USE_STM32_DMA
}
//------------------------------------------------------------------------------
void SdSpiAltDriver::setPort(uint8_t portNumber) {

  if (portNumber > BOARD_NR_SPI) return;
  if (portNumber == 0) portNumber ++;
  m_spi = &SPI;
  SPI.setModule(portNumber);
}
#endif  // defined(__STM32F1__) || defined(__STM32F4__)

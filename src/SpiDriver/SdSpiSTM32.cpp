/**
 * Copyright (c) 2011-2020 Bill Greiman
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
#include "SdSpiDriver.h"

#if defined(SD_USE_CUSTOM_SPI)\
  && (defined(__STM32F1__) || defined(__STM32F4__))
#if defined(__STM32F1__)
#define USE_STM32_DMA 1
#elif defined(__STM32F4__)
#define USE_STM32_DMA 1
#else  // defined(__STM32F1__)
#error Unknown STM32 type
#endif  // defined(__STM32F1__)

//------------------------------------------------------------------------------
void SdSpiArduinoDriver::activate() {
  m_spi->beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::begin(SdSpiConfig spiConfig) {
  if (spiConfig.spiPort) {
    m_spi = spiConfig.spiPort;
  } else {
    m_spi = &SPI;
  }
  m_spi->begin();
}
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::deactivate() {
  m_spi->endTransaction();
}
//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive() {
  return m_spi->transfer(0XFF);
}
//------------------------------------------------------------------------------
uint8_t SdSpiArduinoDriver::receive(uint8_t* buf, size_t count) {
#if USE_STM32_DMA
if (n<250) // DMA does not work on addresses on stack (CCMRAM)
  m_spi->read(buf, n);
else
  m_spi->dmaTransfer((uint8_t)0xFF, buf, n);
#else  // USE_STM32_DMA
  m_spi->read(buf, n);
#endif  // USE_STM32_DMA
  return 0;
}
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(uint8_t data) {
  m_spi->write(data);
}
//------------------------------------------------------------------------------
void SdSpiArduinoDriver::send(const uint8_t* buf , size_t count) {
#if USE_STM32_DMA
if (n<250) // DMA does not work with addresses on stack (CCMRAM)
  m_spi->write(buf, n);
else
  m_spi->dmaSend(buf, n);
#else  // USE_STM32_DMA
  m_spi->write(buf, n);
#endif  // USE_STM32_DMA
}
#endif  // defined(SD_USE_CUSTOM_SPI) &&  defined(__STM32F1__)

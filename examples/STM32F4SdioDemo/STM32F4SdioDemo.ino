// Simple performance test for STM32 SDHC.
// Demonstrates yield() efficiency.
#include <SdioF4.h> //#include <SdFat.h>
#include <iostream/ArduinoStream.h>

#include "FreeStack.h"

// 32 KiB buffer.
const int32_t BUF_DIM = 32768;

// 8 MiB file.
const uint32_t FILE_SIZE = 256UL*BUF_DIM;

//SdioFat sd; // old SDIO usage
SdFat sd; // set USE_SDIO in config file

File file;

uint8_t buf[BUF_DIM];

// buffer as uint32_t
uint32_t* buf32 = (uint32_t*)buf;

// Total usec in read/write calls.
uint32_t totalMicros = 0;
// Time in yield() function.
uint32_t yieldMicros = 0;
// Number of yield calls.
uint32_t yieldCalls = 0;
// Max busy time for single yield call.
uint32_t yieldMaxUsec = 0;
//-----------------------------------------------------------------------------
bool sdBusy() {
  return sd.card()->isBusy();
}
//-----------------------------------------------------------------------------
void errorHalt(const char* msg) {
  sd.errorHalt(msg);
}
//------------------------------------------------------------------------------
// Replace "weak" system yield() function.
void yield() {
  // Only count cardBusy time.
  if (!sdBusy()) {
    return;
  }
  uint32_t m = micros();
  yieldCalls++;
  while (sdBusy()) {
    // Do something here.
  }
  m = micros() - m;
  if (m > yieldMaxUsec) {
    yieldMaxUsec = m;
  }
  yieldMicros += m;
}
//-----------------------------------------------------------------------------
void runTest() {
  // Zero Stats
  totalMicros = 0;
  yieldMicros = 0;
  yieldCalls = 0;
  yieldMaxUsec = 0; 
  if (!file.open("STM32Demo.bin", O_RDWR | O_CREAT)) {
    errorHalt("open failed");
  }
  Serial.println("\nsize, write, read");
  Serial.println("bytes, KB/sec, KB/sec");
  for (int nb = 512; nb <= BUF_DIM; nb *= 2) {
    file.truncate(0);
    uint32_t nRdWr = FILE_SIZE/nb;
    //Serial.print("writing blocks: "); Serial.println(nRdWr);
    Serial.print(nb); Serial.print(", ");
    uint32_t t = micros();
    for (uint32_t n = 0; n < nRdWr; n++) {
      // Set start and end of buffer.
      buf32[0] = n;
      buf32[nb/4 - 1] = n;
	  int32_t ret = file.write(buf, nb);
      if (nb != ret) {
		  Serial.print("file.write returned: "); Serial.println(ret);
        errorHalt("write failed");
      }
    }
    t = micros() - t;
    totalMicros += t;
    Serial.print(1000.0*FILE_SIZE/t);
    Serial.print(", ");
    file.rewind();
    t = micros();
    
    for (uint32_t n = 0; n < nRdWr; n++) {
      if ((int)nb != file.read(buf, nb)) {
        errorHalt("read failed");
      }
      // crude check of data.     
      if (buf32[0] != n || buf32[nb/4 - 1] != n) {
		  Serial.print("Check error: expected: "); Serial.print(n); Serial.print("read: "); Serial.print(buf32[0]); Serial.print(", ");  Serial.println(buf32[nb/4 - 1]);
        errorHalt("data check");
      }
    }
    t = micros() - t;
    totalMicros += t;   
    Serial.println(1000.0*FILE_SIZE/t);    
  }
  file.close();
  Serial.print("\ntotalMicros  ");
  Serial.println(totalMicros);
  Serial.print("yieldMicros  ");
  Serial.println(yieldMicros);
  Serial.print("yieldCalls   ");
  Serial.println(yieldCalls);
  Serial.print("yieldMaxUsec ");
  Serial.println(yieldMaxUsec); 
  Serial.println("Done");
}
//------------------------------------------------------------------------------
// serial output steam
ArduinoOutStream cout(Serial);
cid_t m_cid;
csd_t m_csd;
uint32_t m_eraseSize;
uint32_t m_ocr;

#define sdErrorMsg(msg) sd.errorPrint(F(msg));
//------------------------------------------------------------------------------
uint8_t cidDmp() {
  cid_t cid;
  if (!sd.card()->readCID(&cid)) {
    sdErrorMsg("readCID failed");
    return false;
  }
  cout << F("\nManufacturer ID: ");
  cout << hex << int(cid.mid) << dec << endl;
  cout << F("OEM ID: ") << cid.oid[0] << cid.oid[1] << endl;
  cout << F("Product: ");
  for (uint8_t i = 0; i < 5; i++) {
    cout << cid.pnm[i];
  }
  cout << F("\nVersion: ");
  cout << int(cid.prv_n) << '.' << int(cid.prv_m) << endl;
  cout << F("Serial number: ") << hex << cid.psn << dec << endl;
  cout << F("Manufacturing date: ");
  cout << int(cid.mdt_month) << '/';
  cout << (2000 + cid.mdt_year_low + 10 * cid.mdt_year_high) << endl;
  cout << endl;
  return true;
}
//------------------------------------------------------------------------------
uint8_t csdDmp() {
  bool eraseSingleBlock;
  if (m_csd.v1.csd_ver == 0) {
    eraseSingleBlock = m_csd.v1.erase_blk_en;
    m_eraseSize = (m_csd.v1.sector_size_high << 1) | m_csd.v1.sector_size_low;
  } else if (m_csd.v2.csd_ver == 1) {
    eraseSingleBlock = m_csd.v2.erase_blk_en;
    m_eraseSize = (m_csd.v2.sector_size_high << 1) | m_csd.v2.sector_size_low;
  } else {
    cout << F("m_csd version error\n");
    return false;
  }
  m_eraseSize++;
  cout << F("cardSize: ") << 0.000512 * sdCardCapacity(&m_csd);
  cout << F(" MB (MB = 1,000,000 bytes)\n");

  cout << F("flashEraseSize: ") << int(m_eraseSize) << F(" blocks\n");
  cout << F("eraseSingleBlock: ");
  if (eraseSingleBlock) {
    cout << F("true\n");
  } else {
    cout << F("false\n");
  }
  return true;
}
//------------------------------------------------------------------------------
void sd_info(void)
{
  uint32_t t = millis();

  if (!sd.begin(DMA_SDIO)) {
    sdErrorMsg("\ncardBegin failed");
    return;
  }
  t = millis() - t;

  cout << F("\ninit time: ") << t << " ms" << endl;
  cout << F("\nCard type: ");
  switch (sd.card()->type()) {
  case SD_CARD_TYPE_SD1:
    cout << F("SD1\n");
    break;

  case SD_CARD_TYPE_SD2:
    cout << F("SD2\n");
    break;

  case SD_CARD_TYPE_SDHC:
    if (sdCardCapacity(&m_csd) < 70000000) {
      cout << F("SDHC\n");
    } else {
      cout << F("SDXC\n");
    }
    break;

  default:
    cout << F("Unknown\n");
  }
  if (!cidDmp()) {
    return;
  }
  if (!csdDmp()) {
    return;
  }
  uint32_t ocr;
  if (!sd.card()->readOCR(&ocr)) {
    sdErrorMsg("\nreadOCR failed");
    return;
  }
  cout << F("OCR: ") << hex << ocr << dec << endl;
//  if (!partDmp()) {
//    return;
//  }
}
//------------------------------------------------------------------------------
void volDmp() {
  cout << F("\nScanning FAT, please wait.\n");
  uint32_t freeClusterCount = sd.freeClusterCount();
  if (sd.fatType() <= 32) {
    cout << F("\nVolume is FAT") << int(sd.fatType()) << endl;
  } else {
    cout << F("\nVolume is exFAT\n");
  }
  cout << F("sectorsPerCluster: ") << sd.sectorsPerCluster() << endl;
  cout << F("clusterCount:      ") << sd.clusterCount() << endl;
  cout << F("freeClusterCount:  ") << freeClusterCount << endl;
  cout << F("fatStartSector:    ") << sd.fatStartSector() << endl;
  cout << F("dataStartSector:   ") << sd.dataStartSector() << endl;
  if (sd.dataStartSector() % m_eraseSize) {
    cout << F("Data area is not aligned on flash erase boundary!\n");
    cout << F("Download and use formatter from www.sdcard.org!\n");
  }
}
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  while (!Serial); delay(10);

  cout << ("######################################################\n");
  cout << ("  Demo sketch of STM32F4 SDIO (DMA) implementation.\n"); 
  cout << ("######################################################\n");
  if (!sd.begin()) {
      sd.initErrorHalt("SdFatSdio begin() failed");
  }
  // make sd the current volume.
  sd.chvol();

  sd_info();

  volDmp();

  cout << F("\nFreeStack: ") << FreeStack() << endl;
  cout << ("######################################################\n");
}
//-----------------------------------------------------------------------------
void loop() {
  while (Serial.available()) Serial.read();

  Serial.println("\nType any character to start");
  while (!Serial.available()) {}
  Serial.println("Test started - please wait, it may take up to 3 minutes");

  runTest();
}
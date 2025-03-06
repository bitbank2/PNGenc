//
// PNG encoder example
// Demonstrates how to compress a RGB565 image as a truecolor (24-bit)
// The PNG spec doesn't support working directly with RGB565 pixels
//
// written by Larry Bank (bitbank@pobox.com)
//
#include <PNGenc.h>
#include "tulips_16bit.h" // a 240x240 RGB565 image

#define WRITE_TO_SD

#ifdef WRITE_TO_SD
#include <SD.h>
#include <SPI.h>
#endif

// SD card on the JC4827W543
#define SD_CS 10
#define SPI_MOSI 11
#define SPI_SCK 12
#define SPI_MISO 13

PNG png;
uint8_t *pBits, *pOut, *pSrc;
#define MAX_OUT_SIZE 65536
uint8_t ucTemp[240 * 3]; // will hold a line of converted RGB565->RGB888 pixels
//
// Convert a line of RGB565 pixels into RGB888
void ConvertLine(uint8_t *pSrc, uint8_t *pDest, int iWidth)
{
int i;
uint8_t r, g, b;
uint16_t *pu16, u16;

  pu16 = (uint16_t *)pSrc;
  for (i=0; i<iWidth; i++) {
      u16 = *pu16++;
      r = ((u16 & 0xf800) >> 8) | (u16 >> 13);
      g = ((u16 & 0x7e0) >> 3) | ((u16 >> 9) & 3);
      b = ((u16 & 0x1f) << 3) | ((u16 & 0x1c) >> 2);
      pDest[0] = r; pDest[1] = g; pDest[2] = b;
      pDest += 3;
  }
} /* ConvertLine() */

void setup()
{
  int y, rc, iDataSize;
  long l;
  int iPitch;

  Serial.begin(115200);
  delay(3000); // allow time for CDC-Serial to start
  Serial.println("Starting PNG RGB565 to RGB888 example");
  pOut = (uint8_t *)malloc(MAX_OUT_SIZE);
  if (!pOut) {
    Serial.println("Not enough RAM!");
    while (1) {};
  }
  pBits = (uint8_t *)tulips_16bit; // get pointer to start of image data
  pBits += pBits[10] + (pBits[11] * 256); // start of pixels
  iPitch = 240*2; // bytes per line
  if (tulips_16bit[25] == 0) { // height is positive so bitmap is "bottom-up"
      pBits += 239 * iPitch;
      iPitch = -iPitch;
  } 
  rc = png.open(pOut, MAX_OUT_SIZE);
  if (rc == PNG_SUCCESS) {
      Serial.println("Successfully opened PNG encoder");
      l = millis();
      rc = png.encodeBegin(240, 240, PNG_PIXEL_TRUECOLOR, 24, NULL, 9);
      if (rc == PNG_SUCCESS) {
          pSrc = pBits;
          for (y=0; y<240 && rc == PNG_SUCCESS; y++) {
              // convert a line of image from RGB565 to RGB888
              ConvertLine(pSrc, ucTemp, 240);
              rc = png.addLine(ucTemp);
              pSrc += iPitch;
          } // for y
      } else {
          Serial.printf("Error starting PNG encoding = %d\n", png.getLastError());
      }
      iDataSize = png.close();
      l = millis() - l;
      Serial.printf("240x240 PNG image compressed in %d ms to %d bytes\n", (int)l, iDataSize);
#ifdef WRITE_TO_SD
File myfile;
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
      while (!SD.begin(SD_CS, SPI, 20000000)) {
          Serial.println("Unable to access SD Card");
          delay(1000);
      }
      Serial.println("SD Card opened successfully");
      myfile = SD.open("/tulips.png", FILE_WRITE);
      if (myfile) {
         myfile.write(pOut, iDataSize);
         myfile.close();
         Serial.println("Successfully created the file");
      } else {
        Serial.println("Error creating file");
      }
#endif // WRITE_TO_SD
  } else {// if not opened successfully
      Serial.printf("Error opening memory PNG = %d\n", png.getLastError());
  }
} /* setup() */
void loop()
{
}

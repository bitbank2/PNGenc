//
// This example shows how to save a screenshot of a RGB Panel LCD
// (framebuffer comes from the ESP32-S3 PSRAM)
// In this case, the Makerfabs ESP32-S3 Touch 4" 480x480 
//
#include <bb_spi_lcd.h>
#include <PNGenc.h>
#include <SPI.h>
#include <SD.h>
BB_SPI_LCD lcd;
PNG png;
SPIClass SD_SPI;
// File instance
static File pngfile;
// SD connections for the Makerfabs ESP32-S3 4" 480x480
#define SD_CS 10
#define SD_MOSI 11
#define SD_SCK 12
#define SD_MISO 13

// Callback functions to access a file on the SD card

void *myOpen(const char *filename) {
 // Serial.printf("Attempting to open %s\n", filename);
  pngfile = SD.open(filename, "w+r"); // This option is needed to both read and write
  return &pngfile;
}

void myClose(PNGFILE *handle) {
  File *f = (File *)handle->fHandle;
  f->flush();
  f->close();
}
int32_t myRead(PNGFILE *pFile, uint8_t *buffer, int32_t length) {
//  File *f = (File *)pFile->fHandle;
//  return f->read(buffer, length);
    int32_t iBytesRead;
    iBytesRead = length;
    File *f = static_cast<File *>(pFile->fHandle);
//    Serial.printf("Read: requested size = %d\n", length);
    iBytesRead = (int32_t)f->read(buffer, iBytesRead);
    pFile->iPos = f->position();
//    Serial.printf("Read %d bytes\n", iBytesRead);
//    Serial.printf("New Pos: %d\n", pFile->iPos);
    return iBytesRead;
}

int32_t myWrite(PNGFILE *handle, uint8_t *buffer, int32_t length) {
  File *f = (File *)handle->fHandle;
  handle->iPos += length;
//  Serial.printf("Write %d bytes, new pos = %d\n", length, handle->iPos);
  return f->write(buffer, length);
}
int32_t mySeek(PNGFILE *handle, int32_t position) {
  File *f = (File *)handle->fHandle;
  f->seek(position);
  handle->iPos = (int)f->position();
//  Serial.printf("Seek: %d\n", (int)position);
  return handle->iPos;
}

// Write out a PNG file
bool saveScreenshot(const char *filename)
{
int rc;
uint8_t *pTemp;

  // Delete any previous file of same name
//  SD.remove(PNG_FILENAME);

  // Open the PNG file mechanism and install callbacks
  rc = png.open(filename, myOpen, myClose, myRead, myWrite, mySeek);
  if (rc != PNG_SUCCESS) {
    lcd.println("png.open failed");
    return false;
  }
  rc = png.encodeBegin(lcd.width(), lcd.height(), PNG_PIXEL_TRUECOLOR, 24, NULL, 9);
  if (rc != PNG_SUCCESS) {
    lcd.println("png.encodeBegin failed");
    png.close();
    return false;
  }

  // Get a pointer to the buffer containing the fractal image
  uint16_t *pFramebuffer;
  pTemp = (uint8_t *)malloc(lcd.width() * 3);
  pFramebuffer = (uint16_t *)lcd.getBuffer();
   for (int y = 0; y < lcd.height(); y++) {
    rc = png.addRGB565Line(&pFramebuffer[y * lcd.width()], pTemp, true);
    if (rc != PNG_SUCCESS) {
      lcd.printf("addRGB565Line failed; rc = %d\n", rc);
      png.close();
      return false;
    }
  }

  int32_t bytesWritten = png.close();
  lcd.printf("Bytes written to file: %d\n", bytesWritten);
  return true;
} /* saveScreenshot() */

void setup() {
//  Serial.begin(115200);
  lcd.begin(DISPLAY_CYD_4848);
  lcd.fillScreen(TFT_BLACK);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.setFont(FONT_12x16);
  lcd.println("PNG screenshot example");
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
  if (!SD.begin(SD_CS, SD_SPI, 20000000)) { // Faster than 10MHz seems to fail on the CYDs
    lcd.println("Card Mount Failed");
    while (1) {
      vTaskDelay(1);
    }
  } else {
    lcd.println("Card Mount Succeeded");
  }
  lcd.fillRect(100, 100, 200, 200, TFT_BLUE);
  lcd.fillCircle(300, 300, 80, TFT_RED);
//  delay(2000);
} /* setup() */

void loop() {
//  Serial.println("Saving screenshot");
  saveScreenshot("/shot.png");
//  Serial.println("Finished");
  while (1) {
    vTaskDelay(1);
  }
}

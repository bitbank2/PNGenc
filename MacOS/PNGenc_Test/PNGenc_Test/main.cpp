//
//  main.cpp
//  PNGenc_Test
//
//  Created by Laurence Bank on 3/3/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include "../../../src/PNGenc.cpp"
#include "../../../src/zutil.c"
#include "../../../src/trees.c"
#include "../../../src/deflate.c"
#include "../../../src/crc32.c"
#include "../../../src/adler32.c"
#include "tulips_16bit.h" // WinBMP test image
PNG png;
uint8_t *s, *d, *pOut, ucLine[1024];
uint16_t *s16;
int iFilePos;
//
// Simple logging print
//
void PNGLOG(int line, char *string, const char *result)
{
    printf("Line: %d: msg: %s%s\n", line, string, result);
} /* PNGLOG() */

void * myOpen(const char *filename) {
    iFilePos = 0;
    return (void *)1; // fake file system
}
void myClose(PNGFILE *handle) {
}
int32_t myRead(PNGFILE *handle, uint8_t *buffer, int32_t length) {
    memcpy(buffer, &pOut[iFilePos], length);
    iFilePos += length;
    return length;
}
int32_t myWrite(PNGFILE *handle, uint8_t *buffer, int32_t length) {
    memcpy(&pOut[iFilePos], buffer, length);
    iFilePos += length;
    return length;
}
int32_t mySeek(PNGFILE *handle, int32_t position) {
    iFilePos = position;
    return iFilePos;
}

int main(int argc, const char * argv[]) {
    int i, y, rc, iTotal;
    char *szTestName;
    uint32_t u32;
    int iTotalPass, iTotalFail;
    const char *szStart = " - START";
    int iDataSize;
    iTotalPass = iTotalFail = iTotal = 0;

    // Test 1 - Test creating a pure color 32-bit PNG in RAM
    iTotal++;
    szTestName = (char *)"Test creating a pure color 32-bit PNG in RAM";
    PNGLOG(__LINE__, szTestName, szStart);
    pOut = (uint8_t *)malloc(16384); // big enough
    rc = png.open(pOut, 16384);
    if (rc == PNG_SUCCESS) {
        rc = png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR_ALPHA, 8, NULL, 9);
        //png.setAlphaPalette(ucAlphaPal);
        if (rc == PNG_SUCCESS) {
            uint32_t *d32 = (uint32_t *)ucLine;
            for (i=0; i<100; i++) { // prepare line of pixels
                d32[i] = 0xff00ff00; // fully opaque green
            }
            for (y=0; y<100 && rc == PNG_SUCCESS; y++) {
                rc = png.addLine(ucLine);
            } // for y
            iDataSize = png.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 127 && u32 == 0x474e5089) { // correct length for highly compressed data and the PNG header magic bytes
                iTotalPass++;
                PNGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                PNGLOG(__LINE__, szTestName, " - FAILED");
            }
        }
    } else {
        PNGLOG(__LINE__, szTestName, "Error creating PNG file.");
    }
    free(pOut);
    // Test 2 - Test creating a pure color 32-bit PNG in a file
    iTotal++;
    szTestName = (char *)"Test creating a pure color 32-bit PNG in a file";
    PNGLOG(__LINE__, szTestName, szStart);
    pOut = (uint8_t *)calloc(16384, 1); // big enough
    rc = png.open("/testimg.png", myOpen, myClose, myRead, myWrite, mySeek);
    if (rc == PNG_SUCCESS) {
        rc = png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR_ALPHA, 8, NULL, 9);
        //png.setAlphaPalette(ucAlphaPal);
        if (rc == PNG_SUCCESS) {
            uint32_t *d32 = (uint32_t *)ucLine;
            for (i=0; i<100; i++) { // prepare line of pixels
                d32[i] = 0xff00ff00; // fully opaque green
            }
            for (y=0; y<100 && rc == PNG_SUCCESS; y++) {
                rc = png.addLine(ucLine);
            } // for y
            iDataSize = png.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 127 && u32 == 0x474e5089) { // correct length for highly compressed data and the PNG header magic bytes
                iTotalPass++;
                PNGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                PNGLOG(__LINE__, szTestName, " - FAILED");
            }
        }
    } else {
        PNGLOG(__LINE__, szTestName, "Error creating PNG file.");
    }
    free(pOut);
    // Test 3 - Test creating an 8-bit grayscale PNG in RAM
    iTotal++;
    szTestName = (char *)"Test creating an 8-bit grayscale PNG in RAM";
    PNGLOG(__LINE__, szTestName, szStart);
    pOut = (uint8_t *)malloc(16384); // big enough
    rc = png.open(pOut, 16384);
    if (rc == PNG_SUCCESS) {
        rc = png.encodeBegin(100, 100, PNG_PIXEL_GRAYSCALE, 8, NULL, 9);
        //png.setAlphaPalette(ucAlphaPal);
        if (rc == PNG_SUCCESS) {
            for (i=0; i<100; i++) { // prepare line of pixels
                ucLine[i] = 0x1f;
            }
            for (y=0; y<100 && rc == PNG_SUCCESS; y++) {
                rc = png.addLine(ucLine);
            } // for y
            iDataSize = png.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 128 && u32 == 0x474e5089) { // correct length for highly compressed data and the PNG header magic bytes
                iTotalPass++;
                PNGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                PNGLOG(__LINE__, szTestName, " - FAILED");
            }
        }
    } else {
        PNGLOG(__LINE__, szTestName, "Error creating PNG file.");
    }
    free(pOut);
    // Test 4 - Graceful failure of insufficient output buffer size
    iTotal++;
    szTestName = (char *)"Graceful failure of insufficient output buffer size";
    PNGLOG(__LINE__, szTestName, szStart);
    pOut = (uint8_t *)malloc(16384); // big enough
    rc = png.open(pOut, 100);
    if (rc == PNG_SUCCESS) {
        rc = png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR_ALPHA, 8, NULL, 9);
        //png.setAlphaPalette(ucAlphaPal);
        if (rc == PNG_SUCCESS) {
            uint32_t *d32 = (uint32_t *)ucLine;
            for (i=0; i<100; i++) { // prepare line of pixels
                d32[i] = 0xff00ff00; // fully opaque green
            }
            for (y=0; y<100 && rc == PNG_SUCCESS; y++) {
                rc = png.addLine(ucLine);
            } // for y
            if (rc == PNG_MEM_ERROR) { // error telling us the output buffer is too small
                iTotalPass++;
                PNGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                PNGLOG(__LINE__, szTestName, " - FAILED");
            }
        }
    } else {
        PNGLOG(__LINE__, szTestName, "Error creating PNG file.");
    }
    free(pOut);
    // Test 5 - Invalid parameters test 1
    iTotal++;
    szTestName = (char *)"Invalid parameters test 1";
    PNGLOG(__LINE__, szTestName, szStart);
    rc = png.open("/testimg.png", NULL, NULL, NULL, NULL, NULL);
    if (rc == PNG_INVALID_PARAMETER) { // file access requires all of the callback functions
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 6 - Invalid parameters test 2
    iTotal++;
    szTestName = (char *)"Invalid parameters test 2";
    PNGLOG(__LINE__, szTestName, szStart);
    rc = png.open(NULL, 0);
    if (rc == PNG_INVALID_PARAMETER) { // memory access requires a valid buffer and length
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 7 - Invalid parameters test 3
    iTotal++;
    szTestName = (char *)"Invalid parameters test 3";
    PNGLOG(__LINE__, szTestName, szStart);
    rc = png.open("/testimg.png", myOpen, NULL, myRead, myWrite, mySeek); // missing the close callback
    if (rc == PNG_INVALID_PARAMETER) { // file access requires all of the callback functions
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 8 - Invalid parameters test 4
    iTotal++;
    memset(&png, 0, sizeof(png));
    szTestName = (char *)"Invalid parameters test 4";
    PNGLOG(__LINE__, szTestName, szStart);
    png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR_ALPHA, 8, NULL, 9);
    rc = png.addLine(ucLine);
    if (rc == PNG_NOT_INITIALIZED) { // the encoder hasn't been properly initialized
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 9 - Invalid parameters test 5
    iTotal++;
    memset(&png, 0, sizeof(png));
    szTestName = (char *)"Invalid parameters test 5";
    PNGLOG(__LINE__, szTestName, szStart);
    rc = png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR_ALPHA + 10, 8, NULL, 9); // invalid pixel type
    if (rc == PNG_INVALID_PARAMETER) {
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 10 - Invalid parameters test 6
    iTotal++;
    memset(&png, 0, sizeof(png));
    szTestName = (char *)"Invalid parameters test 6";
    PNGLOG(__LINE__, szTestName, szStart);
    rc = png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR_ALPHA, 5, NULL, 9); // unsupported bit depth
    if (rc == PNG_INVALID_PARAMETER) {
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 11 - Invalid parameters test 7
    iTotal++;
    memset(&png, 0, sizeof(png));
    szTestName = (char *)"Invalid parameters test 7";
    PNGLOG(__LINE__, szTestName, szStart);
    rc = png.encodeBegin(100, 100, PNG_PIXEL_TRUECOLOR, 24, NULL, 10); // unsupported compression effort
    if (rc == PNG_INVALID_PARAMETER) {
        iTotalPass++;
        PNGLOG(__LINE__, szTestName, " - PASSED");
    } else {
        iTotalFail++;
        PNGLOG(__LINE__, szTestName, " - FAILED");
    }
    // Test 12 - Test creating a 24-bit truecolor image from RGB565
    iTotal++;
    szTestName = (char *)"Test creating a 24-bit color PNG in RAM";
    PNGLOG(__LINE__, szTestName, szStart);
    pOut = (uint8_t *)malloc(65536); // big enough
    rc = png.open(pOut, 65536);
    s = (uint8_t *)tulips_16bit;
    s += s[10]; // point to pixels
    s += 240*2*239; // bottom up bitmap
    s16 = (uint16_t *)s;
    if (rc == PNG_SUCCESS) {
        rc = png.encodeBegin(240, 240, PNG_PIXEL_TRUECOLOR, 24, NULL, 9);
        if (rc == PNG_SUCCESS) {
            for (y=0; y<240 && rc == PNG_SUCCESS; y++) {
                for (i=0; i<240; i++) { // prepare line of pixels
                    uint8_t r, g, b;
                    uint16_t u16;
                    u16 = s16[i];
                    r = ((u16 & 0xf800) >> 8) | (u16 >> 13);
                    g = ((u16 & 0x7e0) >> 3) | ((u16 >> 9) & 3);
                    b = ((u16 & 0x1f) << 3) | ((u16 & 0x1c) >> 2);
                    ucLine[i*3] = r; ucLine[(i*3)+1] = g; ucLine[(i*3)+2] = b;
                }
                s16 -= 240;
                rc = png.addLine(ucLine);
            } // for y
            iDataSize = png.close();
            u32 = *(uint32_t *)pOut;
            if (iDataSize == 59869 && u32 == 0x474e5089) { // correct length for highly compressed data and the PNG header magic bytes
                iTotalPass++;
                PNGLOG(__LINE__, szTestName, " - PASSED");
            } else {
                iTotalFail++;
                PNGLOG(__LINE__, szTestName, " - FAILED");
            }
        }
    } else {
        PNGLOG(__LINE__, szTestName, "Error creating PNG file.");
    }
    free(pOut);

    printf("Total tests: %d, %d passed, %d failed\n", iTotal, iTotalPass, iTotalFail);
    return 0;
}

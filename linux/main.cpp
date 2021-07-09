//
//  main.cpp
//  pngenc_test
//
//  Created by Larry Bank on 6/27/21.
//
#include "../src/PNGenc.h"

PNG png; // static instance of class

uint8_t localPalette[] = {0,0,0,0,255,0};
uint8_t ucAlphaPalette[] = {0,255}; // first color is transparent

// Disable this macro to use a memory buffer to 'catch' the PNG output
// otherwise it will write the data incrementally to the output file
#define USE_FILES
//
// Read a 24-bpp BMP file into memory
//
uint8_t * ReadBMP(const char *fname, int *width, int *height, int *bpp, unsigned char *pPal)
{
    int y, w, h, bits, offset;
    uint8_t *s, *d, *pTemp, *pBitmap;
    int pitch, bytewidth;
    int iSize, iDelta;
    FILE *infile;
    
    infile = fopen(fname, "r+b");
    if (infile == NULL) {
        printf("Error opening input file %s\n", fname);
        return NULL;
    }
    // Read the bitmap into RAM
    fseek(infile, 0, SEEK_END);
    iSize = (int)ftell(infile);
    fseek(infile, 0, SEEK_SET);
    pBitmap = (uint8_t *)malloc(iSize);
    pTemp = (uint8_t *)malloc(iSize);
    fread(pTemp, 1, iSize, infile);
    fclose(infile);
    
    if (pTemp[0] != 'B' || pTemp[1] != 'M' || pTemp[14] < 0x28) {
        free(pBitmap);
        free(pTemp);
        printf("Not a Windows BMP file!\n");
        return NULL;
    }
    w = *(int32_t *)&pTemp[18];
    h = *(int32_t *)&pTemp[22];
    bits = *(int16_t *)&pTemp[26] * *(int16_t *)&pTemp[28];
    if (bits <= 8) { // it has a palette, copy it
        uint8_t *p = pPal;
        for (int i=0; i<(1<<bits); i++)
        {
           *p++ = pTemp[54+i*4];
           *p++ = pTemp[55+i*4];
           *p++ = pTemp[56+i*4];
        }
    }
    offset = *(int32_t *)&pTemp[10]; // offset to bits
    bytewidth = (w * bits) >> 3;
    pitch = (bytewidth + 3) & 0xfffc; // DWORD aligned
// move up the pixels
    d = pBitmap;
    s = &pTemp[offset];
    iDelta = pitch;
    if (h > 0) {
        iDelta = -pitch;
        s = &pTemp[offset + (h-1) * pitch];
    } else {
        h = -h;
    }
    for (y=0; y<h; y++) {
        if (bits == 32) {// need to swap red and blue
            for (int i=0; i<bytewidth; i+=4) {
                d[i] = s[i+2];
                d[i+1] = s[i+1];
                d[i+2] = s[i];
                d[i+3] = s[i+3];
            }
        } else {
            memcpy(d, s, bytewidth);
        }
        d += bytewidth;
        s += iDelta;
    }
    *width = w;
    *height = h;
    *bpp = bits;
    free(pTemp);
    return pBitmap;
    
} /* ReadBMP() */

#define ITERATION_COUNT 1

int32_t myWrite(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    FILE *ohandle = (FILE *)pFile->fHandle;
    return (int32_t)fwrite(pBuf, 1, iLen, ohandle);
} /* myWrite() */

int32_t myRead(PNGFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    FILE *ohandle = (FILE *)pFile->fHandle;
    return (int32_t)fread(pBuf, 1, iLen, ohandle);
} /* myRead() */

int32_t mySeek(PNGFILE *pFile, int32_t iPosition)
{
    FILE *f = (FILE *)pFile->fHandle;
    fseek(f, iPosition, SEEK_SET);
    return(int32_t)ftell(f);

} /* mySeek() */

void * myOpen(const char *szFilename)
{
    return fopen(szFilename, "w+b");
} /* myOpen() */

void myClose(PNGFILE *pHandle)
{
    FILE *f = (FILE *)pHandle->fHandle;
    fclose(f);
} /* myClose() */

int main(int argc, const char * argv[]) {
    int rc;
    int iDataSize, iBufferSize;
#ifndef USE_FILES
    FILE *ohandle;
    uint8_t *pOutput;
#endif
    int iWidth, iHeight, iBpp, iPitch;
    uint8_t *pBitmap;
    uint8_t ucPalette[1024];
    
    if (argc != 3) {
       printf("Usage: png_demo <infile.bmp> <outfile.png>\n");
       return 0;
    }
    printf("size of png class/struct = %d\n", (int)sizeof(png));
    
    pBitmap = ReadBMP(argv[1], &iWidth, &iHeight, &iBpp, ucPalette);
    if (pBitmap == NULL)
    {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        return -1; // bad filename passed?
    }
    
    for (int j=0; j<ITERATION_COUNT; j++) {
        uint8_t ucBitSize = 8, ucPixelType=0, *pPalette=NULL;
        iBufferSize = iWidth * iHeight;
#ifndef USE_FILES
        pOutput = (uint8_t *)malloc(iBufferSize);
#endif
        iPitch = iWidth;
        switch (iBpp) {
            case 8:
            case 4:
            case 2:
            case 1:
                ucPixelType = PNG_PIXEL_INDEXED;
                iPitch = (iWidth * iBpp) >> 3;
                pPalette = ucPalette;
                ucBitSize = (uint8_t)iBpp;
                break;
            case 24:
                ucPixelType = PNG_PIXEL_TRUECOLOR;
                iPitch = iWidth * 3;
                ucBitSize = 24;
                break;
            case 32:
                ucPixelType = PNG_PIXEL_TRUECOLOR_ALPHA;
                iPitch = iWidth * 4;
                ucBitSize = 32;
                break;
        } // switch on pixel type
        
#ifdef USE_FILES
        rc = png.open(argv[2], myOpen, myClose, myRead, myWrite, mySeek);
#else
        rc = png.open(pOutput, iBufferSize);
#endif
        if (rc != PNG_SUCCESS) {
            printf("Error opening output file %s, exiting...\n", argv[2]);
            return -1;
        }
        rc = png.encodeBegin(128, 128, PNG_PIXEL_INDEXED, 8, localPalette, 9);
        png.setAlphaPalette(ucAlphaPalette);
//        rc = png.encodeBegin(iWidth, iHeight, ucPixelType, ucBitSize, pPalette, 9);
        if (rc == PNG_SUCCESS) {
            for (int y=0; y<128/*iHeight*/ && rc == PNG_SUCCESS; y++) {
                uint8_t ucLine[128];
                if (y==0 || y == 127) {
                  memset(ucLine, 1, 128); // top+bottom red lines
                } else {
                  memset(ucLine, 0, 128);
                  ucLine[0] = ucLine[127] = 1; // left/right border
                  ucLine[y] = ucLine[127-y] = 1; // X in the middle
                }
                rc = png.addLine(ucLine);
               // rc = png.addLine(&pBitmap[iPitch * y]);
            }
            iDataSize = png.close();
            printf("%d bytes of compressed data written to file\n", iDataSize);
#ifndef USE_FILES
            if (rc == PNG_SUCCESS) { // good output, write it to a file
                ohandle = fopen(argv[2], "w+b");
                if (ohandle != NULL) {
                    fwrite(pOutput, 1, iDataSize, ohandle);
                    fclose(ohandle);
                }
            }
            free(pOutput);
#endif
        }
    } // for j
    free(pBitmap);
    return 0;
}

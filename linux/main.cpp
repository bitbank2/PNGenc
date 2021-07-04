//
//  main.cpp
//  pngenc_test
//
//  Created by Larry Bank on 6/27/21.
//

#include "../src/PNGenc.h"

PNG png; // static instance of class

//
// Read a 24-bpp BMP file into memory
//
uint8_t * ReadBMP(const char *fname, int *width, int *height)
{
    int y, w, h, offset;
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
    offset = *(int32_t *)&pTemp[10]; // offset to bits
    bytewidth = w * 3;
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
        memcpy(d, s, bytewidth);
        d += bytewidth;
        s += iDelta;
    }
    *width = w;
    *height = h;
    free(pTemp);
    return pBitmap;
    
} /* ReadBMP() */

#define ITERATION_COUNT 1

int main(int argc, const char * argv[]) {
    int rc;
    int iDataSize, iBufferSize;
    FILE *ohandle;
    int iWidth, iHeight, iPitch;
    uint8_t *pBitmap, *pOutput;
    
    if (argc != 3) {
       printf("Usage: png_demo <infile.bmp> <outfile.png>\n");
       return 0;
    }
    pBitmap = ReadBMP(argv[1], &iWidth, &iHeight);
    if (pBitmap == NULL)
    {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        return -1; // bad filename passed?
    }
    
    for (int j=0; j<ITERATION_COUNT; j++) {
        iBufferSize = iWidth * iHeight;
        pOutput = (uint8_t *)malloc(iBufferSize);
        iPitch = iWidth * 3;
        png.open(pOutput, iBufferSize);
        rc = png.encodeBegin(iWidth, iHeight, PNG_PIXEL_TRUECOLOR, NULL, 9);
        if (rc == PNG_SUCCESS) {
            for (int y=0; y<iHeight && rc == PNG_SUCCESS; y++) {
                rc = png.addLine(&pBitmap[iPitch * y]);
            }
            iDataSize = png.close();
            ohandle = fopen(argv[2], "w+b");
            if (ohandle != NULL) {
                fwrite(pOutput, 1, iDataSize, ohandle);
                fclose(ohandle);
            }
            free(pOutput);
        }
    } // for j
    free(pBitmap);
    return 0;
}

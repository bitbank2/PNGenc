//
//  main.cpp
//  pngenc_test
//
//  Created by Larry Bank on 6/27/21.
//

#include "../src/PNGenc.h"

PNG png; // static instance of class

/* Windows BMP header info (54 bytes) */
uint8_t winbmphdr[54] =
        {0x42,0x4d,
         0,0,0,0,         /* File size */
         0,0,0,0,0x36,4,0,0,0x28,0,0,0,
         0,0,0,0, /* Xsize */
         0,0,0,0, /* Ysize */
         1,0,8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* number of planes, bits per pel */
         0,0,0,0};

int main(int argc, const char * argv[]) {
#ifdef FUTURE
    int i, rc;
    uint8_t *pData;
    int iDataSize;
    FILE *ihandle;
    uint8_t *pPalette;
    
    if (argc != 3) {
       printf("Usage: png_demo <infile.png> <outfile.bmp>\n");
       return 0;
    }
    ihandle = fopen(argv[1],"rb"); // open input file
    if (ihandle == NULL)
    {
        fprintf(stderr, "Unable to open file: %s\n", argv[1]);
        return -1; // bad filename passed
    }
    fseek(ihandle, 0L, SEEK_END); // get the file size
    iDataSize = (int)ftell(ihandle);
    fseek(ihandle, 0, SEEK_SET);
    pData = (uint8_t *)malloc(iDataSize);
    fread(pData, 1, iDataSize, ihandle);
    fclose(ihandle);
    
//    for (int j=0; j<10000; j++) {
        rc = png.openRAM(pData, iDataSize, NULL); //PNGDraw);
    if (rc == PNG_SUCCESS) {
        printf("image specs: (%d x %d), %d bpp, pixel type: %d\n", png.getWidth(), png.getHeight(), png.getBpp(), png.getPixelType());
        png.setBuffer((uint8_t *)malloc(png.getBufferSize()));
        rc = png.decode(NULL, 0); //PNG_CHECK_CRC);
        i = 1;
        pPalette = NULL;
        switch (png.getPixelType()) {
            case PNG_PIXEL_INDEXED:
                pPalette = png.getPalette();
                i = 1;
                break;
            case PNG_PIXEL_TRUECOLOR:
                i = 3;
                break;
            case PNG_PIXEL_TRUECOLOR_ALPHA:
                i = 4;
                break;
        }
        SaveBMP((char *)argv[2], png.getBuffer(), pPalette, png.getWidth(), png.getHeight(), i*png.getBpp());
        png.close();
        free(png.getBuffer());
//    } // for j
    }
#endif // FUTURE
    return 0;
}

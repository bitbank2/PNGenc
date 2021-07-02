//
// PNG Encoder
//
// Copyright (c) 2000-2021 BitBank Software, Inc.
// Written by Larry Bank
// Project started 12/9/2000
//
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "zlib.h"

#define WRITEMOTO32(p, o, val) {uint32_t l = val; p[o] = (unsigned char)(l >> 24); p[o+1] = (unsigned char)(l >> 16); p[o+2] = (unsigned char)(l >> 8); p[o+3] = (unsigned char)l;}

//extern int PILAdjustColors(PIL_PAGE *inpage, int iDestBpp, int iOptions, PILBOOL bAllocate);
//extern int PILReadBMP(PIL_PAGE *pInPage, PIL_PAGE *pOutPage);
//extern void PILTIFFHoriz(PIL_PAGE *InPage, PILBOOL bDecode);
unsigned char PILPNGFindFilter(uint8_t *pCurr, uint8_t *pPrev, int iPitch, int iStride);
void PNGFilter(uint8_t ucFilter, uint8_t *pOut, uint8_t *pCurr, uint8_t *pPrev, int iStride, int iPitch);
uint32_t PNGCalcCRC(unsigned char *buf, int len);

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PNGCalcCRC()                                               *
 *                                                                          *
 *  PURPOSE    : Calculate the PNG-style CRC value for a block of data.     *
 *                                                                          *
 ****************************************************************************/
uint32_t PNGCalcCRC(unsigned char *buf, int len)
{
/* Table of CRCs of all 8-bit messages. */
   static uint32_t crc_table[256];
   static int crc_table_computed = 0;
   uint32_t crc = 0xffffffff;
   int n;

   /* Make the table for a fast CRC. */
   if (crc_table_computed == 0)
   {
     uint32_t c;
     int n, k;
     
     for (n = 0; n < 256; n++) {
       c = (uint32_t) n;
       for (k = 0; k < 8; k++) {
         if (c & 1)
           c = 0xedb88320L ^ (c >> 1);
         else
           c = c >> 1;
       }
       crc_table[n] = c;
     }
     crc_table_computed = 1;
   }

   /* Update a running CRC with the bytes buf[0..len-1]--the CRC
      should be initialized to all 1's, and the transmitted value
      is the 1's complement of the final running CRC (see the
      crc() routine below)). */

     for (n = 0; n < len; n++)
     {
         crc = crc_table[(crc ^ buf[n]) & 0xff] ^ (crc >> 8);
     }

  return crc ^ 0xffffffffL;

} /* PNGCalcCRC() */

static unsigned char PILPAETH(unsigned char a, unsigned char b, unsigned char c)
{
    int pa, pb, pc;
#ifdef SLOW_WAY
    int p;
#endif // SLOW_WAY
    
#ifndef SLOW_WAY
    pc = c;
    pa = b - pc;
    pb = a - pc;
    pc = pa + pb;
    if (pa < 0) pa = -pa;
    if (pb < 0) pb = -pb;
    if (pc < 0) pc = -pc;
#else
    p = a + b - c; // initial estimate
    pa = abs(p - a); // distances to a, b, c
    pb = abs(p - b);
    pc = abs(p - c);
#endif
    // return nearest of a,b,c,
    // breaking ties in order a,b,c.
    if (pa <= pb && pa <= pc)
        return a;
    else if (pb <= pc)
        return b;
    else return c;
    
} /* PILPAETH() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : CompressPNG()                                           *
 *                                                                          *
 *  PURPOSE    : Compress a block of image data into an IDAT chunk.         *
 *                                                                          *
 ****************************************************************************/
int PNGAddLine(PNGIMAGE *pImage, uint8_t *pSrc, int y)
{
    unsigned char ucFilter; // filter type
    unsigned char *pOut;
    int iStride;
    int err;
    int iPitch;
    
    iStride = pImage->ucBpp >> 3; // bytes per pixel
    iPitch = (pImage->iWidth * pImage->ucBpp) >> 3;
    if (iStride < 1)
        iStride = 1; // 1,4 bpp
    pOut = pImage->ucFileBuf;
    memcpy(pImage->ucPrevLine2, pSrc, iPitch); // hold onto unfiltered pixels
// PILPNGFindFilter(uint8_t *pCurr, uint8_t *pPrev, int iPitch, int iStride)
    ucFilter = PILPNGFindFilter(pSrc, (y == 0) ? NULL : pImage->ucPrevLine, iPitch, iStride); // find best filter
    *pOut++ = ucFilter; // store filter byte
    PNGFilter(ucFilter, pOut, pSrc, pImage->ucPrevLine, iStride, iPitch); // filter the current line of image data and store
    memcpy(pImage->ucPrevLine, pImage->ucPrevLine2, iPitch);
    // Compress the filtered image data
    if (y == 0) // first block, initialize zlib
    {
        memset(&pImage->c_stream, 0, sizeof(z_stream));
        // ZLIB compression levels: 1 = fastest, 9 = most compressed (slowest)
        err = deflateInit(&pImage->c_stream, pImage->ucCompLevel); // might as well use max compression
        pImage->c_stream.next_out = pImage->pOutput; // pImage->ucFileBuf; // DEBUG
        pImage->c_stream.total_out = 0;
        pImage->c_stream.avail_out = pImage->iBufferSize; // DEBUG
    }
    pImage->c_stream.next_in  = (Bytef*)pImage->ucFileBuf;
    pImage->c_stream.total_in = 0;
    pImage->c_stream.avail_in = iPitch+1; // compress entire buffer in 1 shot
//    while (pImage->c_stream.total_in != 0 && pImage->c_stream.total_out < (unsigned)pImage->iBufferSize)
    {
        //      c_stream.avail_in = c_stream.avail_out = 1; /* force small buffers */
        err = deflate(&pImage->c_stream, Z_NO_FLUSH);
        //         CHECK_ERR(err, "deflate");
    }
//    err = deflate(&pImage->c_stream, Z_SYNC_FLUSH);
    /* Finish the stream, still forcing small buffers: */
    if (err < 0) // unable to compress the data, use it raw
    {
//        memcpy(pComp, pUnComp, iLen);
//        c_stream.total_out = iLen; // compressed length = uncompressed length
    }
    if (y == pImage->iHeight - 1) // last line, clean up
    {
        err = deflate(&pImage->c_stream, Z_FINISH);
        err = deflateEnd(&pImage->c_stream);
        pImage->iDataSize = (int)pImage->c_stream.total_out;
    }
    
    return PNG_SUCCESS; // DEBUG
    
} /* PNGAddLine() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PILMakePNG(PIL_PAGE *, PIL_PAGE *, int)                    *
 *                                                                          *
 *  PURPOSE    : Convert an image into PNG FLATE format.                    *
 *                                                                          *
 ****************************************************************************/
int PILMakePNG(PNGIMAGE *pInPage)
{
    int iError = PNG_SUCCESS;
    unsigned char *p;
    int iSize, i, iLen, iCompressedSize;
    uint32_t ulCRC;
    int iStart, iBlock, iCount, iBlockCount;
    unsigned char *pOut = NULL;
    unsigned char *pComp = NULL; // compressed data
    unsigned char *pUnComp = NULL; // uncompressed, filtered data
    //unsigned char *pBW; // current black and white line
        
    // Break up the image into 8 blocks so that we don't have to allocate another buffer
    // the same size as the uncompressed image.  This can also help the decoder reduce
    // the memory it needs to decode the image.
//    if (pInPage->iHeight > 200 && iOptions == 0)
//    {
//        iBlockCount = 8;
//        iBlock = (pInPage->iHeight+7) / 8; // do it in 8 chunks
//    }
//    else
//    {
//        iBlockCount = 1;
//        iBlock = pInPage->iHeight;
//    }
    p = pOut; // DEBUG
    iSize = 0; // output data size
    WRITEMOTO32(p, iSize, 0x89504e47); // PNG File header
    iSize += 4;
    WRITEMOTO32(p, iSize, 0x0d0a1a0a);
    iSize += 4;
    // IHDR contains 13 data bytes
    WRITEMOTO32(p, iSize, 0x0000000d); // IHDR length
    iSize += 4;
    WRITEMOTO32(p, iSize, 0x49484452); // IHDR marker
    iSize += 4;
    WRITEMOTO32(p, iSize, pInPage->iWidth); // Image Width
    iSize += 4;
    WRITEMOTO32(p, iSize, pInPage->iHeight); // Image Height
    iSize += 4;
    if (pInPage->ucBpp == 8) // grayscale
    {
           p[iSize++] = pInPage->ucBpp; // Bit depth
           p[iSize++] = PNG_PIXEL_GRAYSCALE;
    }
    else // full color image
    {
           p[iSize++] = 8; // Bit depth
           p[iSize++] = (pInPage->ucBpp <= 24) ? 2:6; // RGB triplets or with alpha
    }
    p[iSize++] = 0; // compression method 0
    p[iSize++] = 0; // filter type 0
    p[iSize++] = 0; // interlace = no
    ulCRC = PNGCalcCRC(&p[iSize-17], 17); // store CRC for IHDR chunk
    WRITEMOTO32(p, iSize, ulCRC);
    iSize += 4;
#ifdef FUTURE
    if (pInPage->ucBpp == 8 || pInPage->ucBpp == 4)
	   {
           // Write the palette
           iLen = (1 << pInPage->ucBpp); // palette length
           WRITEMOTO32(p, iSize, iLen*3); // 3 bytes per entry
           iSize += 4;
           WRITEMOTO32(p, iSize, 0x504c5445/*'PLTE'*/);
           iSize += 4;
           for (i=0; i<iLen; i++)
           {
               p[iSize++] = pInPage->pPalette[i*3+2]; // red
               p[iSize++] = pInPage->pPalette[i*3+1]; // green
               p[iSize++] = pInPage->pPalette[i*3+0]; // blue
           }
           ulCRC = PNGCalcCRC(&p[iSize-(iLen*3)-4], 4+(iLen*3)); // store CRC for PLTE chunk
           WRITEMOTO32(p, iSize, ulCRC);
           iSize += 4;
           if (pInPage->iTransparent >= 0 && pInPage->iTransparent < (1 << pInPage->cBitsperpixel)) // add transparency chunk
           {
               iLen = (1 << pInPage->cBitsperpixel); // palette length
               WRITEMOTO32(p, iSize, iLen); // 1 byte per palette alpha entry
               iSize += 4;
               WRITEMOTO32(p, iSize, 0x74524e53 /*'tRNS'*/);
               iSize += 4;
               for (i = 0; i<iLen; i++) // write n alpha values to accompany the palette
               {
                   if (i == pInPage->iTransparent)
                       p[iSize++] = 0; // alpha 0 = fully transparent
                   else
                       p[iSize++] = 255; // fully opaque
               }
               ulCRC = PNGCalcCRC(&p[iSize - iLen - 4], 4 + iLen); // store CRC for tRNS chunk
               WRITEMOTO32(p, iSize, ulCRC);
               iSize += 4;
           }
       }
#endif // FUTURE - palette not needed for a camera
    // Compress the blocks of data and save each one as an
    // IDAT chunk.
    iStart = 0;
    for (i=0; i<iBlockCount; i++)
    {
        iCount = iBlock;
        if (iCount + iStart > pInPage->iHeight)
            iCount = pInPage->iHeight - iStart;
        // DEBUG
//        iCompressedSize = CompressPNG(pInPage, pUnComp, i);
//        if (iCompressedSize < 0) // error
//        {
//            iError = iCompressedSize;
//            goto makepng_exit;
//        }
        iStart += iCount;
        // IDAT
        WRITEMOTO32(p, iSize, iCompressedSize); // IDAT length
        iSize += 4;
        WRITEMOTO32(p, iSize, 0x49444154); // IDAT marker
        iSize += 4;
        memcpy(&p[iSize], pComp, iCompressedSize);
        iSize += iCompressedSize;
        ulCRC = PNGCalcCRC(&p[iSize-iCompressedSize-4], iCompressedSize+4); // store CRC for IDAT chunk
        WRITEMOTO32(p, iSize, ulCRC);
        iSize += 4;
    }
    // Write the IEND chunk
    WRITEMOTO32(p, iSize, 0);
    iSize += 4;
    WRITEMOTO32(p, iSize, 0x49454e44/*'IEND'*/);
    iSize += 4;
    WRITEMOTO32(p, iSize, 0xae426082); // same CRC every time
    iSize += 4;
//    pOutPage->iDataSize = iSize;
    
makepng_exit:
    return iError;
    
} /* PILMakePNG() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PNGFindFilter()                                            *
 *                                                                          *
 *  PURPOSE    : Find the best filter method for this scanline. Use SAD     *
 *               to compare all 5 filter options.                           *
 *                                                                          *
 ****************************************************************************/
unsigned char PILPNGFindFilter(uint8_t *pCurr, uint8_t *pPrev, int iPitch, int iStride)
{
int i;
unsigned char a, b, c, ucDiff, ucFilter;
uint32_t ulSum[5]  = {0,0,0,0,0}; // individual sums for the 4 types of filters
uint32_t ulMin;

    ucFilter = 0;
    for (i=0; i<iPitch; i++)
    {
       ucDiff = pCurr[i]; // no filter
       ulSum[0] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       // Sub
       if (i >= iStride)
       {
          ucDiff = pCurr[i]-pCurr[i-iStride];
          ulSum[1] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       else
       {
           ucDiff = pCurr[i];
           ulSum[1] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       // Up
       if (pPrev)
       {
          ucDiff = pCurr[i]-pPrev[i];
          ulSum[2] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       else // not available
       {
           ucDiff = pCurr[i];
           ulSum[2] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       }
       // Average
       if (!pPrev || i < iStride)
       {
          if (!pPrev)
          {
             if (i < iStride)
                a = 0;
             else
                a = pCurr[i-iStride]>>1;
          }
          else
             a = pPrev[i]>>1;
       }
       else
       {
          a = (pCurr[i-iStride] + pPrev[i])>>1;
       }
       ucDiff = pCurr[i] - a;
       ulSum[3] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       // Paeth
       if (i < iStride)
          a = 0;
       else
          a = pCurr[i-iStride]; // left
       if (pPrev == NULL)
          b = 0; // above doesn't exist
       else
          b = pPrev[i];
       if (!pPrev || i < iStride)
          c = 0;
       else
          c = pPrev[i-iStride]; // above left
       ucDiff = pCurr[i] - PILPAETH(a,b,c);
       ulSum[4] += (ucDiff < 128) ? ucDiff: 256 - ucDiff;
       } // for i
       // Pick the best filter (or NONE if they're all bad)
       ulMin = iPitch * 255; // max value
       for (a=0; a<5; a++)
       {
          if (ulSum[a] < ulMin)
          {
             ulMin = ulSum[a];
             ucFilter = a; // current winner
          }
       } // for
       return ucFilter;

} /* PILPNGFindFilter() */

/****************************************************************************
 *                                                                          *
 *  FUNCTION   : PNGFilter()                                                *
 *                                                                          *
 *  PURPOSE    : Filter raw image data to make it compress better.          *
 *                                                                          *
 ****************************************************************************/
void PNGFilter(uint8_t ucFilter, uint8_t *pOut, uint8_t *pCurr, uint8_t *pPrev, int iStride, int iPitch)
{
int j;

   switch (ucFilter)
      {
      case 0: // no filter, just copy
         memcpy(pOut, pCurr, iPitch);
         break;
      case 1: // sub
         j = 0;
         while (j < iStride)
         {
             pOut[j] = pCurr[j];
             j++;
         }
         while (j < (int)iPitch)
         {
            pOut[j] = pCurr[j]-pPrev[j];
            j++;
         }
         break;
      case 2: // up
         if (pPrev)
         {
            for (j=0;j<iPitch;j++)
            {
               pOut[j] = pCurr[j]-pPrev[j];
            }
         }
         else
            memcpy(pOut, pCurr, iPitch);
         break;
      case 3: // average
         for (j=0; j<iPitch; j++)
        {
            int a;
            if (!pPrev || j < iStride)
            {
               if (!pPrev)
               {
                  if (j < iStride)
                     a = 0;
                  else
                     a = pCurr[j-iStride]>>1;
               }
               else
                  a = pPrev[j]>>1;
            }
            else
            {
               a = (pCurr[j-iStride] + pPrev[j])>>1;
            }
            pOut[j] = (uint8_t)(pCurr[j] - a);
         }
         break;
      case 4: // paeth
         for (j=0; j<iPitch; j++)
         {
            uint8_t a,b,c;
            if (j < iStride)
               a = 0;
            else
               a = pCurr[j-iStride]; // left
            if (!pPrev)
               b = 0; // above doesn't exist
            else
               b = pPrev[j]; // above
            if (!pPrev || j < iStride)
               c = 0;
            else
               c = pPrev[j-iStride]; // above left
            pOut[j] = pCurr[j] - PILPAETH(a,b,c);
         }
         break;
      } // switch
} /* PNGFilter() */

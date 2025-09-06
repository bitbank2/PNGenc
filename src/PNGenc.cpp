//
// PNG Decoder
//
// written by Larry Bank
// bitbank@pobox.com
// Arduino port started 5/3/2021
// Original PNG code written 20+ years ago :)
// The goal of this code is to decode PNG images on embedded systems
//
// Copyright 2021 BitBank Software, Inc. All Rights Reserved.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//    http://www.apache.org/licenses/LICENSE-2.0
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//===========================================================================
//
#include "PNGenc.h"

// Include the C code which does the actual work
#include "png.inl"

//
// File (SD/MMC) based initialization
//
int PNGENC::open(const char *szFilename, PNGENC_OPEN_CALLBACK *pfnOpen, PNGENC_CLOSE_CALLBACK *pfnClose, PNGENC_READ_CALLBACK *pfnRead, PNGENC_WRITE_CALLBACK *pfnWrite, PNGENC_SEEK_CALLBACK *pfnSeek)
{
    // For file access, all callback functions MUST be defined
    if (!pfnOpen || !pfnClose || !pfnRead || !pfnWrite || !pfnSeek) return PNG_INVALID_PARAMETER;
    memset(&_png, 0, sizeof(PNGENCIMAGE));
    _png.iTransparent = -1;
    _png.pfnRead = pfnRead;
    _png.pfnWrite = pfnWrite;
    _png.pfnSeek = pfnSeek;
    _png.pfnOpen = pfnOpen;
    _png.pfnClose = pfnClose;
    _png.PNGFile.fHandle = (*pfnOpen)(szFilename);
    if (_png.PNGFile.fHandle == NULL) {
        _png.iError = PNG_INVALID_FILE;
       return PNG_INVALID_FILE;
    }
    return PNG_SUCCESS;

} /* open() */

int PNGENC::open(uint8_t *pOutput, int iBufferSize)
{
    if (!pOutput || iBufferSize < 32) return PNG_INVALID_PARAMETER; // must have a valid buffer and minimum size
    memset(&_png, 0, sizeof(PNGENCIMAGE));
    _png.iTransparent = -1;
    _png.pOutput = pOutput;
    _png.iBufferSize = iBufferSize;
    return PNG_SUCCESS;
} /* open() */

//
// return the last error (if any)
//
int PNGENC::getLastError()
{
    return _png.iError;
} /* getLastError() */
//
// Close the file - not needed when decoding from memory
//
int PNGENC::close()
{
    if (_png.pfnClose)
        (*_png.pfnClose)(&_png.PNGFile);
    return _png.iDataSize;
} /* close() */

int PNGENC::encodeBegin(int iWidth, int iHeight, uint8_t ucPixelType, uint8_t ucBpp, uint8_t *pPalette, uint8_t ucCompLevel)
{
    // Check for valid parameters
    if (iWidth < 1 || iWidth > 32767 || iHeight < 1 || iHeight > 32767) return PNG_INVALID_PARAMETER;
    if (ucPixelType != PNG_PIXEL_GRAYSCALE && ucPixelType != PNG_PIXEL_TRUECOLOR && ucPixelType != PNG_PIXEL_INDEXED &&
        ucPixelType != PNG_PIXEL_GRAY_ALPHA && ucPixelType != PNG_PIXEL_TRUECOLOR_ALPHA) return PNG_INVALID_PARAMETER;
    if (ucBpp != 1 && ucBpp != 2 && ucBpp != 4 && ucBpp != 8 && ucBpp != 24 && ucBpp != 32) return PNG_INVALID_PARAMETER;
    if (ucCompLevel > 9) return PNG_INVALID_PARAMETER;
    
    _png.iWidth = iWidth;
    _png.iHeight = iHeight;
    _png.ucPixelType = ucPixelType;
    _png.ucBpp = ucBpp;
    if (pPalette != NULL)
        memcpy(_png.ucPalette, pPalette, 768); // save 256 color entries
    _png.ucCompLevel = ucCompLevel;
    _png.y = 0;
    return PNG_SUCCESS;
} /* encodeBegin() */

int PNGENC::addLine(uint8_t *pPixels)
{
    int rc;
    if ((_png.pOutput && _png.iBufferSize) || (_png.pfnOpen && _png.pfnClose && _png.pfnRead && _png.pfnSeek && _png.pfnWrite)) {
        rc = PNG_addLine(&_png, pPixels, _png.y);
        _png.y++;
        return rc;
    } else { // the encoder was never initialized
        return PNG_NOT_INITIALIZED;
    }
} /* addLine() */

int PNGENC::addRGB565Line(uint16_t *pPixels, void *pTempLine, bool bBigEndian)
{
    int rc;
    rc = PNG_addRGB565Line(&_png, pPixels, pTempLine, _png.y, (int)bBigEndian);
    _png.y++;
    return rc;
} /* addRGB565Line() */

int PNGENC::setTransparentColor(uint32_t u32Color)
{
    if (_png.ucPixelType == PNG_PIXEL_GRAYSCALE || _png.ucPixelType == PNG_PIXEL_TRUECOLOR) {
        _png.iTransparent = u32Color;
        return PNG_SUCCESS;
    }
    else
        return PNG_INVALID_PARAMETER; // indexed image must have palette alpha values
} /* setTransparentColor() */

int PNGENC::setAlphaPalette(uint8_t *pPalette)
{
    if (pPalette != NULL && _png.ucPixelType == PNG_PIXEL_INDEXED) {
        _png.ucHasAlphaPalette = 1;
        memcpy(&_png.ucPalette[768], pPalette, 256); // capture up to 256 alpha values
        return PNG_SUCCESS;
    }
    return PNG_INVALID_PARAMETER;
} /* setAlphaPalette() */

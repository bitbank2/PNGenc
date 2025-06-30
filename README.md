## PNGenc

Copyright (c) 2021 BitBank Software, Inc.<br>
Written by Larry Bank<br>
bitbank@pobox.com<br>

## What is it?

An 'embedded-friendly' (aka Arduino) PNG image encoding library<br>
<br>

## Why did you write it?

Starting in the late 80's I wrote my own imaging codecs for the existing standards (CCITT G3/G4 was the first). I soon added GIF, JPEG and not long after that, the PNG specification was ratified. All of this code was "clean room" - written just from the specification. I used my imaging library in many projects and products over the years and recently decided that some of my codecs could get a new lease on life as open source, embedded-friendly libraries for microcontrollers.<br>
<br>

## What's special about it?

The PNG image specification was written at a time when computers had megabytes of RAM and conserving memory wasn't a big priority. The memory allocated for encoding the compressed data (zlib) and for holding the uncompressed image can be quite a bit more than is available on modern microcontrollers (usually measured in K bytes). Three goals for this project are: easy to compile+use on all embedded systems, use a minimal amount of RAM and be self-contained. One of the dependencies I like to remove when working on embedded software is malloc/free. When compiling on a system with a tiny amount of RAM, heap memory management might not even exist.<br>

## Feature summary

- Runs on any MCU with at least 45K of free RAM<br>
- No external dependencies (including malloc/free)<br>
- Encode an image line by line<br>
- Allows encoding pixels of type grayscale, indexed, truecolor or truecolor+alpha
- Supports up to 8-bits per color stimulus<br>
- Allows defining a transparent color or alpha palette values<br>
- Allows control of the zlib compression effort amount (1-9)
- Arduino-style C++ library class with simple API<br>
- Can by built as straight C as well<br>
<br>

## How fast is it?

The examples folder contains a sketch to measure the performance of encoding a 128x128 image generated dynamically.<br>

## Documentation

Detailed information about the API is in the Wiki<br>
See the examples folder for easy starting points<br>
<br>

## Funding

This project received funding through [NGI Zero Core](https://nlnet.nl/core), a fund established by [NLnet](https://nlnet.nl) with financial support from the European Commission's [Next Generation Internet](https://ngi.eu) program. Learn more at the [NLnet project page](https://nlnet.nl/project/ImageCodes-Optimised).

[<img src="https://nlnet.nl/logo/banner.png" alt="NLnet foundation logo" width="20%" />](https://nlnet.nl)
[<img src="https://nlnet.nl/image/logos/NGI0_tag.svg" alt="NGI Zero Logo" width="20%" />](https://nlnet.nl/core)

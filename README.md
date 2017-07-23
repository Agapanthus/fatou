# fatou
[![Codacy Badge](https://api.codacy.com/project/badge/Grade/bc720fe91294494984d5dafe1d447ef0)](https://www.codacy.com/app/Agapanthus/fatou?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Agapanthus/fatou&amp;utm_campaign=Badge_Grade)
[![Coverity Scan](https://img.shields.io/coverity/scan/13251.svg)](https://scan.coverity.com/projects/agapanthus-fatou)

Simple viewer for convergence conduct of polynomials solved using Newton's method

# Build

Simply use CMake.
You can build both 32 bit and 64 bit for MSVC.
When using MSVC, use "MinSizeRelease" for a faster Release build. "Release" usually takes much longer (because of the /GL compiler option)!
#!/bin/sh

cp WM2014_01.bin ../../bin/
cd ../../bin/
pwd
WM_CreateImg.exe WM2014_01.bin WM2014_01.img 62161900
TL6_CreateImg.exe WM2014_01.bin WM2014_01.run 4 0x400 
TL6_CreateImg.exe WM2014_01.bin WM2014_01.fls 4 0x400 0x3000
convertToHedImage.exe WM2014_01.bin WM2014_01_uart firm 0x400 0x400
cd -

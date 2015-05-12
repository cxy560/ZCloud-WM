copy WM_SDK.map ..\..\..\Bin
cd ..\..\..\Bin
WM_CreateImg.exe WM_SDK.bin WM_SDK.img 62161900
TL6_CreateImg.exe WM_SDK.bin WM_SDK.run 4 0x400 
TL6_CreateImg.exe WM_SDK.bin WM_SDK.fls 4 0x400 0x3000
convertToHedImage.exe WM_SDK.bin WM_SDK_UART firm 0x400 0x400


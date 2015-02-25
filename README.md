iccflow
=======
*Command-line batch ICC color conversion of JPEG files*  
by Enric Tordera (etordera at gmail dot com)

Description
-----------
*iccflow* application performs ICC color conversion on a set of JPEG files. It is 
a command-line tool that you can integrate in your ICC color management workflow. It takes a folder as input parameter,
and processes all JPEG images found inside this folder. Converted images will be saved to another folder.

Requirements
------------
To compile *iccflow* you will need these libraries:

+  LittleCMS version 2 or higher (lcms2)
+  libjpeg 6b (jpeg)


Build
-----
To build the *iccflow* application just type the usual *make* command:

    make
Binary executable will be output to the *bin* folder.

Usage
-----
**iccflow -i inputFolder -o outputFolder [options]**

**Mandatory parameters:**  
`-i inputFolder` Source folder containing the original JPEG images.

`-o outputFolder` Destination folder where converted images will be saved.

**Optional parameters:**  
`-p outputProfile` Output profile for the color transformation (path to .icc/.icm file). Defaults to sRGB

`-prgb rgbProfile` Default RGB input profile when none is found in the source JPEG file (path to .icc/.icm file). Defaults to sRGB

`-pcmyk cmykProfile` Default CMYK input profile when none is found in the source JPEG file (path to .icc/.icm file). Defaults to FOGRA27

`-pgray grayProfile` Default Grayscale input profile when none is found in the source JPEG file (path to .icc/.icm file). Defaults to D50 Gamma-2.2 Grayscale

`-c intentCode` Rendering intent to be used during color transformation. Possible values:
> 0: Perceptual
> 1: Relative colorimetric (DEFAULT)  
> 2: Saturation  
> 3: Absolute colorimetric

`-q jpegQuality` JPEG quality level for output compression (0-100, defaults to 85)

`-npbc` Disable black point compensation (enabled by default)

`-v` Enable verbose output. Displays percentage progress during processing.

License
-------
You are free to use, modify and distribute this software as you please. 
If you feel *iccflow* is useful for you, giving me credit would be great, but not mandatory. 

Disclaimer
----------
This software is provided "as is", without any warranty of any kind. If you decide to use it, you do it under your only
responsibility. In no way will I be liable for any damage caused by this code.

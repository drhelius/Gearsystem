[Preset]
Name=Sharp Bilinear + Scanlines
Passes=1

[Pass0]
Path=sharp_bilinear_scanlines.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
ScanlineBaseBrightness=0.60
ScanlineHorizontalModulation=0.0
ScanlineVerticalModulation=0.75

[Parameter.ScanlineBaseBrightness]
Label=Base Brightness
Min=0.0
Max=1.0
Step=0.01

[Parameter.ScanlineHorizontalModulation]
Label=Horizontal Modulation
Min=0.0
Max=2.0
Step=0.01

[Parameter.ScanlineVerticalModulation]
Label=Vertical Modulation
Min=0.0
Max=2.0
Step=0.01

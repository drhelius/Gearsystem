; CRT Consumer + NTSC preset for Geargrafx.
; Adds a color-only NTSC YIQ pass before the CRT Consumer pass.

[Preset]
Name=CRT Consumer + NTSC
Passes=2

[Pass0]
Path=ntsc_color.glsl
ScaleType=Source
Filter=Nearest

[Pass1]
Path=crt_consumer.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
NtscArtifacts=1.00
NtscSaturation=0.85
NtscContrast=1.05
NtscShimmer=0.75
PRE_SCALE=1.5
blurx=0.25
blury=-0.1
warpx=0.03
warpy=0.04
corner=0.03
smoothness=125.0
inter=1.0
Downscale=2.0
scanlow=6.0
scanhigh=8.0
beamlow=1.7
beamhigh=1.05
preserve=0.98
brightboost1=1.25
brightboost2=1.0
glow=3.0
quality=1.0
glow_str=0.35
nois=10.0
postbr=1.0
GAMMA_OUT=2.25
sat=1.0
contrast=1.0
WP=0.0
vignette=1.0
vpower=0.1
vstr=40.0

[Parameter.NtscArtifacts]
Label=NTSC Artifacts
Min=0.0
Max=1.0
Step=0.05

[Parameter.NtscSaturation]
Label=NTSC Saturation
Min=0.0
Max=2.0
Step=0.05

[Parameter.NtscContrast]
Label=NTSC Contrast
Min=0.5
Max=1.5
Step=0.05

[Parameter.NtscShimmer]
Label=NTSC Shimmer
Min=0.0
Max=1.0
Step=0.05

[Parameter.PRE_SCALE]
Label=Pre-Scale Sharpening
Min=1.0
Max=4.0
Step=0.1

[Parameter.blurx]
Label=Convergence X
Min=-4.0
Max=4.0
Step=0.05

[Parameter.blury]
Label=Convergence Y
Min=-4.0
Max=4.0
Step=0.05

[Parameter.warpx]
Label=Curvature X
Min=0.0
Max=0.12
Step=0.01

[Parameter.warpy]
Label=Curvature Y
Min=0.0
Max=0.12
Step=0.01

[Parameter.corner]
Label=Corner Size
Min=0.0
Max=0.10
Step=0.01

[Parameter.smoothness]
Label=Border Smoothness
Min=100.0
Max=600.0
Step=5.0

[Parameter.inter]
Label=Interlacing
Min=0.0
Max=1.0
Step=1.0

[Parameter.Downscale]
Label=Interlace Downscale
Min=1.0
Max=8.0
Step=1.0

[Parameter.scanlow]
Label=Beam Low
Min=1.0
Max=15.0
Step=1.0

[Parameter.scanhigh]
Label=Beam High
Min=1.0
Max=15.0
Step=1.0

[Parameter.beamlow]
Label=Scanlines Dark
Min=0.5
Max=2.5
Step=0.05

[Parameter.beamhigh]
Label=Scanlines Bright
Min=0.5
Max=2.5
Step=0.05

[Parameter.preserve]
Label=Protect White
Min=0.0
Max=1.0
Step=0.01

[Parameter.brightboost1]
Label=Dark Pixel Boost
Min=0.0
Max=3.0
Step=0.05

[Parameter.brightboost2]
Label=Bright Pixel Boost
Min=0.0
Max=3.0
Step=0.05

[Parameter.glow]
Label=Glow Pixels
Min=1.0
Max=6.0
Step=1.0

[Parameter.quality]
Label=Glow Quality
Min=0.25
Max=4.0
Step=0.05

[Parameter.glow_str]
Label=Glow Intensity
Min=0.0001
Max=2.0
Step=0.05

[Parameter.nois]
Label=Noise
Min=0.0
Max=32.0
Step=1.0

[Parameter.postbr]
Label=Post Brightness
Min=0.0
Max=2.5
Step=0.02

[Parameter.GAMMA_OUT]
Label=Gamma Out
Min=0.0
Max=4.0
Step=0.05

[Parameter.sat]
Label=Saturation
Min=0.0
Max=2.0
Step=0.05

[Parameter.contrast]
Label=Contrast
Min=0.0
Max=2.0
Step=0.05

[Parameter.WP]
Label=Color Temperature
Min=-100.0
Max=100.0
Step=5.0

[Parameter.vignette]
Label=Vignette
Min=0.0
Max=1.0
Step=1.0

[Parameter.vpower]
Label=Vignette Power
Min=0.0
Max=1.0
Step=0.01

[Parameter.vstr]
Label=Vignette Strength
Min=0.0
Max=50.0
Step=1.0
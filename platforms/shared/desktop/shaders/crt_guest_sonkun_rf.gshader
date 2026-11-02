; CRT Guest Sonkun RF preset for Gearsystem.
; Ports crt-guest-advanced-hd 2026-07-23-r1 by guest(r) with Dogway's grade, from libretro slang-shaders.
; Parameter values from Sonkun's CRT Guest Advanced HD presets (samples 2026-09-24).
; Original shaders licensed GPL v2 or later.

[Preset]
Name=CRT Guest Sonkun RF
Group=CRT Guest Sonkun
Passes=15
Variants=9

[Pass0]
Path=present.glsl
Alias=StockPass
Filter=Nearest
ScaleType=Previous

[Pass1]
Path=crt_guest_afterglow.glsl
Alias=AfterglowPass
Filter=Linear
ScaleType=Previous

[Pass2]
Path=crt_guest_grade.glsl
Alias=PrePass0
Filter=Linear
ScaleType=Previous

[Pass3]
Path=crt_guest_ntsc_pass1.glsl
Alias=NPass1
Filter=Nearest
FloatFramebuffer=true
ScaleTypeX=Previous
ScaleTypeY=Previous
ScaleX=2.0

[Pass4]
Path=crt_guest_ntsc_pass2.glsl
Filter=Linear
FloatFramebuffer=true
ScaleTypeX=Previous
ScaleTypeY=Previous
ScaleX=0.5

[Pass5]
Path=crt_guest_ntsc_pass3.glsl
Filter=Nearest
ScaleType=Previous

[Pass6]
Path=crt_guest_ntsc_sharpen.glsl
Alias=PrePass
Filter=Nearest
ScaleType=Previous

[Pass7]
Path=crt_guest_linearize.glsl
Alias=LinearizePass
Filter=Linear
FloatFramebuffer=true
ScaleType=Previous

[Pass8]
Path=crt_guest_pass1.glsl
Alias=Pass1
Filter=Linear
FloatFramebuffer=true
ScaleTypeX=Viewport
ScaleTypeY=Previous

[Pass9]
Path=crt_guest_gaussian_horizontal.glsl
Filter=Linear
FloatFramebuffer=true
ScaleTypeX=Absolute
ScaleTypeY=Previous
AbsoluteWidth=800

[Pass10]
Path=crt_guest_gaussian_vertical.glsl
Alias=GlowPass
Filter=Linear
FloatFramebuffer=true
ScaleTypeX=Absolute
ScaleTypeY=Absolute
AbsoluteWidth=800
AbsoluteHeight=600

[Pass11]
Path=crt_guest_bloom_horizontal.glsl
Filter=Linear
FloatFramebuffer=true
ScaleTypeX=Absolute
ScaleTypeY=Previous
AbsoluteWidth=800

[Pass12]
Path=crt_guest_bloom_vertical.glsl
Alias=BloomPass
Filter=Linear
FloatFramebuffer=true
ScaleTypeX=Previous
ScaleTypeY=Absolute
AbsoluteHeight=600

[Pass13]
Path=crt_guest_pass2.glsl
Filter=Linear
FloatFramebuffer=true
ScaleType=Viewport

[Pass14]
Path=crt_guest_deconvergence.glsl
Filter=Linear
ScaleType=Viewport

[Parameters]
internal_res=1.0
auto_res=0.0
esrc=1.0
bth=4.0
PR=0.32
PG=0.32
PB=0.32
AS=0.2
asat=0.33
g_signal_type=1.0
g_crtgamut=-1.0
g_space_out=-1.0
g_Dark_to_Dim=1.0
g_GCompress=1.0
wp_temperature=7104.0
g_CRT_l=2.4
g_CRT_b=50.0
g_CRT_c=50.0
g_hue_degrees=0.0
g_U_SHIFT=0.0
g_V_SHIFT=0.0
g_U_MUL=1.08
g_V_MUL=1.04
g_CRT_br=0.38
g_CRT_bg=0.4
g_CRT_bb=0.3
g_CRT_rf=5.0
g_CRT_sl=0.0
g_vignette=0.0
g_vstr=50.0
g_vpower=0.5
g_lum_fix=0.0
g_MD_Pal=0.0
g_SMS_bl=0.0
g_lum=0.5
g_cntrst=0.5
g_mid=0.59
g_sat=1.0
g_vibr=0.0
g_satr=-0.11
g_satg=0.1
g_satb=0.1
g_lift=6.0
blr=0.0
blg=0.0
blb=0.0
wlr=0.46
wlg=0.67
wlb=0.58
rg=0.08
rb=0.08
gr=0.315
gb=0.26
br=0.1
bg=0.06
speedup=1.0
cust_artifacting=2.0
cust_fringing=2.0
ntsc_fields=-1.0
ntsc_phase=1.0
ntsc_scale=1.0
nscale=1.0
ntsc_sat=1.0
ntsc_bright=1.0
ntsc_gamma=1.0
ntsc_rainbow1=1.0
ntsc_taps=32.0
ntsc_charp=0.0
ntsc_charp3=0.0
ntsc_cscale=1.0
ntsc_cscale1=1.0
ntsc_ring=1.0
ntsc_sharp=10.0
ntsc_fonts=0.75
ntsc_shape=1.0
RFNOISE=0.3
RFNOISE1=0.01
RFNOISE2=0.03
CSHARPEN=5.0
CCONTR=0.0
CDETAILS=1.0
NDEBLUR=1.5
DSMART=0.0
DEDGE=1.0
DESHARP=0.0
GAMMA_INPUT=2.4
gamma_out=2.4
inter=375.0
interm=5.0
iscan=0.2
intres=0.0
iscans=0.25
vga_mode=0.0
hiscan=0.0
HSHARPNESS=1.15
SIGMA_HOR=1.0
S_SHARP=1.3
MAXS=0.03
HARNG=0.0
VSHARPNESS=1.0
SIGMA_VER=0.5
spike=1.0
m_glow=2.0
m_glow_cutoff=0.03
m_glow_low=7.0
m_glow_high=7.0
m_glow_dist=1.0
m_glow_mask=0.75
FINE_GLOW=1.0
SIZEH=1.0
SIGMA_H=1.2
SIZEV=1.0
SIGMA_V=1.2
SIZEVB=1.0
SIGMA_VB=1.0
FINE_BLOOM=1.0
SIZEHB=1.0
SIGMA_HB=1.0
IOS=0.0
warpX=0.0
warpY=0.0
c_shape=0.25
cbulge=0.25
overscanX=0.0
overscanY=0.0
VShift=0.0
csize=0.0
bsize1=0.0
sborder=0.75
barspeed=50.0
barintensity=0.0
bardir=0.0
glow=0.6
bloom=-1.5
mask_bloom=0.0
bloom_dist=2.5
halation=-0.05
bmask1=0.55
hmask1=0.0
gamma_c=2.0
gamma_c2=1.2
brightboost=0.25
brightboost1=0.25
clips=0.0
bloomsamp=0.0
gsl=2.0
scanline1=20.0
scanline2=20.0
beam_min=0.25
beam_max=2.5
tds=0.0
beam_size=0.0
scans=0.0
scan_falloff=0.1
ssharp=0.5
scangamma=16.0
rolling_scan=0.0
no_scanlines=0.0
shadowMask=6.0
maskstr=1.0
mcut=1.1
maskboost=1.0
masksize=1.0
mask_zoom=0.0
mzoom_sh=0.0
mshift=0.0
mask_layout=0.0
laysh=0.0
maskDark=0.5
maskLight=1.5
mask_gamma=1.0
slotmask=1.0
slotmask1=1.0
slotwidth=0.0
double_slot=2.0
slotms=1.0
smoothmask=0.0
smask_mit=0.5
bmask=0.03
mclip=0.0
pr_scan=0.1
edgemask=1.0
dctypex=0.0
dctypey=0.0
deconrr=0.0
deconrg=0.0
deconrb=0.0
deconrry=2.5
deconrgy=2.5
deconrby=2.5
decons=0.8
addnoised=-0.26
noiseresd=1.0
noisetype=1.0
post_br=1.1
oimage=0.0

[Variant0]
Name=Slot Mask 1080p

[Variant1]
Name=Slot Mask 1440p
shadowMask=10.0
masksize=2.0
mask_zoom=-5.0
double_slot=1.0
slotms=2.0

[Variant2]
Name=Slot Mask 4K
shadowMask=12.0
mcut=0.85
masksize=2.0
mask_zoom=-8.0
double_slot=1.0
slotms=3.0

[Variant3]
Name=Shadow Mask 1080p
masksize=2.0
mask_zoom=-2.0
mzoom_sh=0.2
mshift=1.0
mask_gamma=1.2
slotmask=0.0
slotmask1=0.0
smask_mit=0.0

[Variant4]
Name=Shadow Mask 1440p
mcut=0.85
mask_zoom=1.0
mshift=0.5
mask_gamma=1.2
slotmask=0.0
slotmask1=0.0
smask_mit=0.0

[Variant5]
Name=Shadow Mask 4K
shadowMask=10.0
mcut=0.85
masksize=3.0
mask_zoom=-4.0
mshift=1.0
mask_gamma=1.2
slotmask=0.0
slotmask1=0.0
smask_mit=0.0

[Variant6]
Name=Aperture Grille 1080p
mask_gamma=1.2
slotmask=0.0
slotmask1=0.0
smask_mit=0.0
addnoised=-0.22

[Variant7]
Name=Aperture Grille 1440p
shadowMask=10.0
masksize=2.0
mask_zoom=-5.0
mask_gamma=1.2
slotmask=0.0
slotmask1=0.0
smask_mit=0.0

[Variant8]
Name=Aperture Grille 4K
shadowMask=12.0
mcut=0.85
masksize=2.0
mask_zoom=-8.0
mask_gamma=1.2
slotmask=0.0
slotmask1=0.0
smask_mit=0.0

[Parameter.internal_res]
Label=Internal Resolution
Group=Internal Resolution (Quick Setup)
Min=0.5
Max=8.0
Step=0.1

[Parameter.auto_res]
Label=SNES/Amiga Hi-Res Auto Mode
Group=Internal Resolution (Quick Setup)
Min=0.0
Max=1.0
Step=1.0

[Parameter.esrc]
Label=Afterglow Effect Source: OriginalHistory | Source
Group=Afterglow Settings
Min=1.0
Max=2.0
Step=1.0

[Parameter.bth]
Label=Afterglow Effect Threshold
Group=Afterglow Settings
Min=1.0
Max=125.0
Step=1.0

[Parameter.PR]
Label=Persistence Red
Group=Afterglow Settings
Min=0.0
Max=0.5
Step=0.01

[Parameter.PG]
Label=Persistence Green
Group=Afterglow Settings
Min=0.0
Max=0.5
Step=0.01

[Parameter.PB]
Label=Persistence Blue
Group=Afterglow Settings
Min=0.0
Max=0.5
Step=0.01

[Parameter.AS]
Label=Afterglow Strength
Group=Grade
Min=0.0
Max=0.6
Step=0.01

[Parameter.asat]
Label=Afterglow saturation
Group=Grade
Min=0.0
Max=1.0
Step=0.01

[Parameter.g_signal_type]
Label=Signal Type (0:RGB 1:Composite)
Group=Grade
Min=0.0
Max=1.0
Step=1.0

[Parameter.g_crtgamut]
Label=Phosphor (-2:CRT-95s -1:P22-80s 1:P22-90s 2:NTSC-J 3:PAL)
Group=Grade
Min=-3.0
Max=3.0
Step=1.0

[Parameter.g_space_out]
Label=Diplay Color Space (-1:709 0:sRGB 1:P3-D65 2:2020 3:Adobe)
Group=Grade
Min=-1.0
Max=3.0
Step=1.0

[Parameter.g_Dark_to_Dim]
Label=Dark to Dim adaptation
Group=Grade
Min=0.0
Max=1.0
Step=1.0

[Parameter.g_GCompress]
Label=Gamut Compression
Group=Grade
Min=0.0
Max=1.0
Step=1.0

[Parameter.wp_temperature]
Label=White Point
Group=Grade Analog Controls
Min=5004.0
Max=12004.0
Step=100.0

[Parameter.g_CRT_l]
Label=CRT Gamma
Group=Grade Analog Controls
Min=2.3
Max=2.6
Step=0.01

[Parameter.g_CRT_b]
Label=CRT Brightness
Group=Grade Analog Controls
Min=0.0
Max=100.0
Step=1.0

[Parameter.g_CRT_c]
Label=CRT Contrast
Group=Grade Analog Controls
Min=0.0
Max=100.0
Step=1.0

[Parameter.g_hue_degrees]
Label=CRT Hue
Group=Grade Analog Controls
Min=-180.0
Max=180.0
Step=1.0

[Parameter.g_U_SHIFT]
Label=CRT U Shift
Group=Grade Analog Controls
Min=-0.2
Max=0.2
Step=0.01

[Parameter.g_V_SHIFT]
Label=CRT V Shift
Group=Grade Analog Controls
Min=-0.2
Max=0.2
Step=0.01

[Parameter.g_U_MUL]
Label=CRT U Multiplier
Group=Grade Analog Controls
Min=0.0
Max=2.0
Step=0.01

[Parameter.g_V_MUL]
Label=CRT V Multiplier
Group=Grade Analog Controls
Min=0.0
Max=2.0
Step=0.01

[Parameter.g_CRT_br]
Label=CRT Beam Red
Group=Grade Analog Controls
Min=0.0
Max=1.2
Step=0.01

[Parameter.g_CRT_bg]
Label=CRT Beam Green
Group=Grade Analog Controls
Min=0.0
Max=1.2
Step=0.01

[Parameter.g_CRT_bb]
Label=CRT Beam Blue
Group=Grade Analog Controls
Min=0.0
Max=1.2
Step=0.01

[Parameter.g_CRT_rf]
Label=CRT Lambert Refl. in %
Group=Grade Analog Controls
Min=2.0
Max=5.0
Step=0.1

[Parameter.g_CRT_sl]
Label=Surround Luminance -nits-
Group=Grade Analog Controls
Min=0.0
Max=100.0
Step=1.0

[Parameter.g_vignette]
Label=Vignette Toggle
Group=Grade Analog Controls
Min=0.0
Max=1.0
Step=1.0

[Parameter.g_vstr]
Label=Vignette Strength
Group=Grade Analog Controls
Min=0.0
Max=50.0
Step=1.0

[Parameter.g_vpower]
Label=Vignette Power
Group=Grade Analog Controls
Min=0.0
Max=0.5
Step=0.01

[Parameter.g_lum_fix]
Label=Sega Luma Fix
Group=Grade Sega Fixes
Min=0.0
Max=1.0
Step=1.0

[Parameter.g_MD_Pal]
Label=MD Palette
Group=Grade Sega Fixes
Min=0.0
Max=1.0
Step=1.0

[Parameter.g_SMS_bl]
Label=SMS Blue Lift
Group=Grade Sega Fixes
Min=0.0
Max=1.0
Step=1.0

[Parameter.g_lum]
Label=Brightness
Group=Grade Digital Controls
Min=-0.5
Max=1.0
Step=0.01

[Parameter.g_cntrst]
Label=Contrast
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.05

[Parameter.g_mid]
Label=Contrast Pivot
Group=Grade Digital Controls
Min=0.0
Max=1.0
Step=0.01

[Parameter.g_sat]
Label=Saturation
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.01

[Parameter.g_vibr]
Label=Dullness/Vibrance
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.05

[Parameter.g_satr]
Label=Hue vs Sat Red
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.01

[Parameter.g_satg]
Label=Hue vs Sat Green
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.01

[Parameter.g_satb]
Label=Hue vs Sat Blue
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.01

[Parameter.g_lift]
Label=Black Level
Group=Grade Digital Controls
Min=-60.0
Max=20.0
Step=1.0

[Parameter.blr]
Label=Black-Red Tint
Group=Grade Digital Controls
Min=0.0
Max=1.0
Step=0.01

[Parameter.blg]
Label=Black-Green Tint
Group=Grade Digital Controls
Min=0.0
Max=1.0
Step=0.01

[Parameter.blb]
Label=Black-Blue Tint
Group=Grade Digital Controls
Min=0.0
Max=1.0
Step=0.01

[Parameter.wlr]
Label=White-Red Tint
Group=Grade Digital Controls
Min=0.0
Max=2.0
Step=0.01

[Parameter.wlg]
Label=White-Green Tint
Group=Grade Digital Controls
Min=0.0
Max=2.0
Step=0.01

[Parameter.wlb]
Label=White-Blue Tint
Group=Grade Digital Controls
Min=0.0
Max=2.0
Step=0.01

[Parameter.rg]
Label=Red-Green Tint
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.005

[Parameter.rb]
Label=Red-Blue Tint
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.005

[Parameter.gr]
Label=Green-Red Tint
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.005

[Parameter.gb]
Label=Green-Blue Tint
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.005

[Parameter.br]
Label=Blue-Red Tint
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.005

[Parameter.bg]
Label=Blue-Green Tint
Group=Grade Digital Controls
Min=-1.0
Max=1.0
Step=0.005

[Parameter.speedup]
Label=Speedup w. higher Internal Res.
Group=NTSC
Min=1.0
Max=4.0
Step=1.0

[Parameter.cust_artifacting]
Label=NTSC Artifacting Value
Group=NTSC
Min=0.0
Max=5.0
Step=0.1

[Parameter.cust_fringing]
Label=NTSC Fringing Value
Group=NTSC
Min=0.0
Max=5.0
Step=0.1

[Parameter.ntsc_fields]
Label=NTSC Merge Fields: Auto | NO | YES
Group=NTSC
Min=-1.0
Max=1.0
Step=1.0

[Parameter.ntsc_phase]
Label=NTSC Phase: Auto | 2 phase | 3 phase | Mixed | PCE
Group=NTSC
Min=1.0
Max=5.0
Step=1.0

[Parameter.ntsc_scale]
Label=NTSC Resolution Scaling
Group=NTSC
Min=0.2
Max=2.5
Step=0.025

[Parameter.nscale]
Label=NTSC Filter Scaling
Group=NTSC
Min=0.2
Max=2.5
Step=0.025

[Parameter.ntsc_sat]
Label=NTSC Color Saturation
Group=NTSC
Min=0.0
Max=2.0
Step=0.01

[Parameter.ntsc_bright]
Label=NTSC Brightness
Group=NTSC
Min=0.0
Max=1.5
Step=0.01

[Parameter.ntsc_gamma]
Label=NTSC Filtering Gamma Correction
Group=NTSC
Min=0.25
Max=2.5
Step=0.025

[Parameter.ntsc_rainbow1]
Label=NTSC Coloring/Rainbow Effect (2-phase)
Group=NTSC
Min=0.0
Max=3.0
Step=1.0

[Parameter.ntsc_taps]
Label=NTSC # of Taps (Filter Width)
Group=NTSC
Min=6.0
Max=48.0
Step=1.0

[Parameter.ntsc_charp]
Label=NTSC Preserve 'Edge' Colors 2-phase
Group=NTSC
Min=0.0
Max=10.0
Step=0.5

[Parameter.ntsc_charp3]
Label=NTSC Preserve 'Edge' Colors 3-phase
Group=NTSC
Min=0.0
Max=10.0
Step=0.5

[Parameter.ntsc_cscale]
Label=NTSC Chroma Scaling / Bleeding (2-phase)
Group=NTSC
Min=0.5
Max=4.0
Step=0.05

[Parameter.ntsc_cscale1]
Label=NTSC Chroma Scaling / Bleeding (3-phase)
Group=NTSC
Min=0.2
Max=2.25
Step=0.05

[Parameter.ntsc_ring]
Label=NTSC Anti-Ringing
Group=NTSC
Min=0.0
Max=1.0
Step=0.1

[Parameter.ntsc_sharp]
Label=NTSC Sharpness (Adaptive)
Group=NTSC
Min=-10.0
Max=10.0
Step=0.5

[Parameter.ntsc_fonts]
Label=NTSC Preserve Fonts (0.25 = OFF)
Group=NTSC
Min=0.25
Max=0.75
Step=0.01

[Parameter.ntsc_shape]
Label=NTSC Sharpness Shape
Group=NTSC
Min=0.5
Max=1.0
Step=0.025

[Parameter.RFNOISE]
Label=NTSC RF Noise Frequency
Group=NTSC
Min=0.0
Max=1.0
Step=0.01

[Parameter.RFNOISE1]
Label=NTSC RF Noise Luma+
Group=NTSC
Min=0.0
Max=0.7
Step=0.01

[Parameter.RFNOISE2]
Label=NTSC RF Noise Chroma+
Group=NTSC
Min=0.0
Max=0.7
Step=0.01

[Parameter.CSHARPEN]
Label=FSharpen - Sharpen strength
Group=NTSC Sharpen
Min=0.0
Max=5.0
Step=0.1

[Parameter.CCONTR]
Label=FSharpen - Sharpen(+Deb) Contrast
Group=NTSC Sharpen
Min=0.0
Max=0.25
Step=0.01

[Parameter.CDETAILS]
Label=FSharpen - Details sharpened
Group=NTSC Sharpen
Min=0.0
Max=1.0
Step=0.05

[Parameter.NDEBLUR]
Label=FSharpen - Deblur Strength
Group=NTSC Sharpen
Min=1.0
Max=8.0
Step=0.25

[Parameter.DSMART]
Label=FSharpen - Smart 'Dithering' Deblur
Group=NTSC Sharpen
Min=0.0
Max=1.0
Step=1.0

[Parameter.DEDGE]
Label=FSharpen - Deblur Edge Falloff
Group=NTSC Sharpen
Min=0.7
Max=1.0
Step=0.01

[Parameter.DESHARP]
Label=FSharpen - Deblur Extra Sharpen
Group=NTSC Sharpen
Min=0.0
Max=4.0
Step=0.2

[Parameter.GAMMA_INPUT]
Label=Gamma Input
Group=Gamma Options
Min=1.0
Max=5.0
Step=0.05

[Parameter.gamma_out]
Label=Gamma out
Group=Gamma Options
Min=1.0
Max=5.0
Step=0.05

[Parameter.inter]
Label=Interlace Trigger Resolution/VGA Trigger:
Group=Interlacing Options
Min=0.0
Max=800.0
Step=25.0

[Parameter.interm]
Label=Interlace Mode: OFF, Normal 1-3,5; Interpolation 4
Group=Interlacing Options
Min=0.0
Max=5.0
Step=1.0

[Parameter.iscan]
Label=Interlacing Scanline Effect ('Laced brightness)
Group=Interlacing Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.intres]
Label=Internal Resolution Y: 0.5...y-dowsample divider
Group=Interlacing Options
Min=0.0
Max=10.0
Step=0.5

[Parameter.iscans]
Label=Interlacing (Scanline) Saturation
Group=Interlacing Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.vga_mode]
Label=VGA Single/Double Scan mode
Group=Interlacing Options
Min=0.0
Max=1.0
Step=1.0

[Parameter.hiscan]
Label=High Resolution Scanlines (prepend a scaler...)
Group=Interlacing Options
Min=0.0
Max=1.0
Step=1.0

[Parameter.HSHARPNESS]
Label=Horizontal Filter Range
Group=Filtering Options
Min=1.0
Max=8.0
Step=0.05

[Parameter.SIGMA_HOR]
Label=Horizontal Blur Sigma
Group=Filtering Options
Min=0.1
Max=1.0
Step=0.01

[Parameter.S_SHARP]
Label=Substractive Sharpness
Group=Filtering Options
Min=0.0
Max=2.0
Step=0.1

[Parameter.MAXS]
Label=Maximum Sharpness
Group=Filtering Options
Min=0.0
Max=0.3
Step=0.01

[Parameter.HARNG]
Label=Substractive Sharpness Ringing
Group=Filtering Options
Min=0.0
Max=4.0
Step=0.1

[Parameter.VSHARPNESS]
Label=Vertical Filter Range
Group=Vertical/Interlacing Filtering Options
Min=1.0
Max=8.0
Step=0.05

[Parameter.SIGMA_VER]
Label=Vertical Blur Sigma
Group=Vertical/Interlacing Filtering Options
Min=0.1
Max=1.0
Step=0.01

[Parameter.spike]
Label=Scanline Spike Removal
Group=Vertical/Interlacing Filtering Options
Min=0.0
Max=2.0
Step=0.1

[Parameter.m_glow]
Label=Ordinary Glow / Magic Glow
Group=Magic Glow Settings
Min=0.0
Max=2.0
Step=1.0

[Parameter.m_glow_cutoff]
Label=Magic Glow Cutoff
Group=Magic Glow Settings
Min=0.0
Max=0.4
Step=0.01

[Parameter.m_glow_low]
Label=Magic Glow Low Strength
Group=Magic Glow Settings
Min=0.0
Max=7.0
Step=0.05

[Parameter.m_glow_high]
Label=Magic Glow High Strength
Group=Magic Glow Settings
Min=0.0
Max=7.0
Step=0.1

[Parameter.m_glow_dist]
Label=Magic Glow Distribution
Group=Magic Glow Settings
Min=0.2
Max=4.0
Step=0.05

[Parameter.m_glow_mask]
Label=Magic Glow Mask Strength
Group=Magic Glow Settings
Min=0.0
Max=2.0
Step=0.025

[Parameter.FINE_GLOW]
Label=Fine Glow/M.Glow Sampling
Group=Glow Pass Settings
Min=-1.0
Max=5.0
Step=1.0

[Parameter.SIZEH]
Label=Horizontal Glow Radius
Group=Glow Pass Settings
Min=1.0
Max=50.0
Step=1.0

[Parameter.SIGMA_H]
Label=Horizontal Glow Sigma
Group=Glow Pass Settings
Min=0.2
Max=3.0
Step=0.01

[Parameter.SIZEV]
Label=Vertical Glow Radius
Min=1.0
Max=50.0
Step=1.0

[Parameter.SIGMA_V]
Label=Vertical Glow Sigma
Min=0.2
Max=3.0
Step=0.01

[Parameter.SIZEVB]
Label=Vertical Bloom/Halation Radius
Min=1.0
Max=50.0
Step=1.0

[Parameter.SIGMA_VB]
Label=Vertical Bloom/Halation Sigma
Min=0.25
Max=3.0
Step=0.01

[Parameter.FINE_BLOOM]
Label=Fine Bloom/Halation Sampling
Group=Bloom/Halation/(Glow) Pass Settings
Min=-1.0
Max=5.0
Step=1.0

[Parameter.SIZEHB]
Label=Horizontal Bloom/Halation Radius
Group=Bloom/Halation/(Glow) Pass Settings
Min=1.0
Max=50.0
Step=1.0

[Parameter.SIGMA_HB]
Label=Horizontal Bloom/Halation Sigma
Group=Bloom/Halation/(Glow) Pass Settings
Min=0.25
Max=3.0
Step=0.01

[Parameter.IOS]
Label=Integer Scaling: Odd:Y, Even:'X'+Y
Group=Screen Options
Min=0.0
Max=4.0
Step=1.0

[Parameter.warpX]
Label=CurvatureX (default 0.03)
Group=Screen Options
Min=0.0
Max=0.25
Step=0.01

[Parameter.warpY]
Label=CurvatureY (default 0.04)
Group=Screen Options
Min=0.0
Max=0.25
Step=0.01

[Parameter.c_shape]
Label=Curvature Shape
Group=Screen Options
Min=0.05
Max=0.6
Step=0.05

[Parameter.cbulge]
Label=Curvature 3D Effect
Group=Screen Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.overscanX]
Label=Overscan X original pixels
Group=Screen Options
Min=-200.0
Max=200.0
Step=1.0

[Parameter.overscanY]
Label=Overscan Y original pixels
Group=Screen Options
Min=-200.0
Max=200.0
Step=1.0

[Parameter.VShift]
Label=Vertical shift Y original pixels
Group=Screen Options
Min=-200.0
Max=200.0
Step=1.0

[Parameter.csize]
Label=Corner Size
Group=Screen Options
Min=0.0
Max=0.35
Step=0.01

[Parameter.bsize1]
Label=Border Size
Group=Screen Options
Min=0.0
Max=2.0
Step=0.01

[Parameter.sborder]
Label=Border Intensity
Group=Screen Options
Min=0.25
Max=2.0
Step=0.05

[Parameter.barspeed]
Label=Hum Bar Speed
Group=Screen Options
Min=5.0
Max=200.0
Step=1.0

[Parameter.barintensity]
Label=Hum Bar Intensity
Group=Screen Options
Min=-1.0
Max=1.0
Step=0.01

[Parameter.bardir]
Label=Hum Bar Direction
Group=Screen Options
Min=0.0
Max=1.0
Step=1.0

[Parameter.glow]
Label=(Magic) Glow Strength
Group=Brightness Settings
Min=-2.0
Max=2.0
Step=0.01

[Parameter.bloom]
Label=Bloom Strength
Group=Brightness Settings
Min=-2.0
Max=2.0
Step=0.05

[Parameter.mask_bloom]
Label=Mask Bloom
Group=Brightness Settings
Min=-2.0
Max=2.0
Step=0.05

[Parameter.bloom_dist]
Label=Bloom Distribution
Group=Brightness Settings
Min=-2.0
Max=3.0
Step=0.05

[Parameter.halation]
Label=Halation Strength
Group=Brightness Settings
Min=-2.0
Max=2.0
Step=0.025

[Parameter.bmask1]
Label=Bloom Mask Strength
Group=Brightness Settings
Min=-1.0
Max=1.0
Step=0.025

[Parameter.hmask1]
Label=Halation Mask Strength
Group=Brightness Settings
Min=-1.0
Max=1.0
Step=0.025

[Parameter.gamma_c]
Label=Gamma correct
Group=Brightness Settings
Min=0.5
Max=2.0
Step=0.025

[Parameter.gamma_c2]
Label=Complementary Gamma correct
Group=Brightness Settings
Min=1.0
Max=2.0
Step=0.025

[Parameter.brightboost]
Label=Bright Boost Dark Pixels
Group=Brightness Settings
Min=0.25
Max=10.0
Step=0.05

[Parameter.brightboost1]
Label=Bright Boost Bright Pixels
Group=Brightness Settings
Min=0.25
Max=3.0
Step=0.025

[Parameter.clips]
Label=Clip Saturated Color Beams
Group=Brightness Settings
Min=-1.0
Max=1.0
Step=0.05

[Parameter.bloomsamp]
Label=Bloom Pixel Sampling
Group=Brightness Settings
Min=0.0
Max=2.0
Step=1.0

[Parameter.gsl]
Label=Scanline Type
Group=Scanline Options
Min=-1.0
Max=2.0
Step=1.0

[Parameter.scanline1]
Label=Scanline Beam Shape Center
Group=Scanline Options
Min=-20.0
Max=40.0
Step=0.5

[Parameter.scanline2]
Label=Scanline Beam Shape Edges
Group=Scanline Options
Min=0.0
Max=70.0
Step=1.0

[Parameter.beam_min]
Label=Scanline Shape Dark Pixels
Group=Scanline Options
Min=0.25
Max=10.0
Step=0.05

[Parameter.beam_max]
Label=Scanline Shape Bright Pixels
Group=Scanline Options
Min=0.2
Max=3.5
Step=0.025

[Parameter.tds]
Label=Thinner Dark Scanlines
Group=Scanline Options
Min=0.0
Max=1.0
Step=1.0

[Parameter.beam_size]
Label=Increased Bright Scanline Beam
Group=Scanline Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.scans]
Label=Scanline Saturation / Mask Falloff
Group=Scanline Options
Min=0.0
Max=6.0
Step=0.1

[Parameter.scan_falloff]
Label=Scanline Falloff
Group=Scanline Options
Min=0.1
Max=2.0
Step=0.025

[Parameter.ssharp]
Label=Smart Sharpen Scanlines
Group=Scanline Options
Min=0.0
Max=0.5
Step=0.01

[Parameter.scangamma]
Label=Scanline Gamma
Group=Scanline Options
Min=0.5
Max=16.0
Step=0.05

[Parameter.rolling_scan]
Label=Rolling Scanlines
Group=Scanline Options
Min=-1.0
Max=1.0
Step=0.01

[Parameter.no_scanlines]
Label=No-scanline mode
Group=Scanline Options
Min=0.0
Max=1.5
Step=0.05

[Parameter.shadowMask]
Label=CRT Mask: 0:CGWG, 1-4:Lottes, 5-14:'Trinitron'
Group=CRT Mask Options
Min=-1.0
Max=14.0
Step=1.0

[Parameter.maskstr]
Label=Mask Strength (0, 5-14)
Group=CRT Mask Options
Min=-0.5
Max=1.0
Step=0.025

[Parameter.mcut]
Label=Mask 5-14 Low Strength
Group=CRT Mask Options
Min=0.0
Max=2.0
Step=0.05

[Parameter.maskboost]
Label=CRT Mask Boost
Group=CRT Mask Options
Min=1.0
Max=3.0
Step=0.05

[Parameter.masksize]
Label=CRT Mask Size
Group=CRT Mask Options
Min=1.0
Max=4.0
Step=1.0

[Parameter.mask_zoom]
Label=CRT Mask Zoom (+ mask width)
Group=CRT Mask Options
Min=-10.0
Max=6.0
Step=1.0

[Parameter.mzoom_sh]
Label=CRT Mask Zoom Sharpen (needs Mask Zoom)
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.mshift]
Label=(Transform to) Shadow Mask
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.5

[Parameter.mask_layout]
Label=Mask Layout Sequence: RGB or RBG (check LCD panel)
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=1.0

[Parameter.laysh]
Label=Mask Layout Shift: GBR, RGB, BRG | GRB, RBG, BGR
Group=CRT Mask Options
Min=-1.0
Max=1.0
Step=1.0

[Parameter.maskDark]
Label=Lottes maskDark
Group=CRT Mask Options
Min=0.0
Max=2.0
Step=0.05

[Parameter.maskLight]
Label=Lottes maskLight
Group=CRT Mask Options
Min=0.0
Max=2.0
Step=0.05

[Parameter.mask_gamma]
Label=Mask gamma
Group=CRT Mask Options
Min=1.0
Max=5.0
Step=0.05

[Parameter.slotmask]
Label=Slot Mask Strength Bright Pixels
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.slotmask1]
Label=Slot Mask Strength Dark Pixels
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.slotwidth]
Label=Slot Mask Width (0:Auto)
Group=CRT Mask Options
Min=0.0
Max=16.0
Step=1.0

[Parameter.double_slot]
Label=Slot Mask Height: 2x1 or 4x1...
Group=CRT Mask Options
Min=1.0
Max=4.0
Step=1.0

[Parameter.slotms]
Label=Slot Mask Thickness
Group=CRT Mask Options
Min=1.0
Max=4.0
Step=1.0

[Parameter.smoothmask]
Label=Smooth Masks in bright scanlines
Group=CRT Mask Options
Min=0.0
Max=2.0
Step=0.25

[Parameter.smask_mit]
Label=Mitigate Slotmask Interaction
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.05

[Parameter.bmask]
Label=Base (black) Mask strength
Group=CRT Mask Options
Min=0.0
Max=0.25
Step=0.01

[Parameter.mclip]
Label=Preserve Mask Strength
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.025

[Parameter.pr_scan]
Label=Preserve Scanline Properties
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.025

[Parameter.edgemask]
Label=Mitigate Mask on Edges
Group=CRT Mask Options
Min=0.0
Max=1.0
Step=0.1

[Parameter.dctypex]
Label=Deconvergence type X : 0.0 - static, other - dynamic
Group=Horizontal/Vertical Deconvergence
Min=0.0
Max=0.75
Step=0.05

[Parameter.dctypey]
Label=Deconvergence type Y : 0.0 - static, other - dynamic
Group=Horizontal/Vertical Deconvergence
Min=0.0
Max=0.75
Step=0.05

[Parameter.deconrr]
Label=Horizontal Deconvergence Red Range
Group=Horizontal/Vertical Deconvergence
Min=-15.0
Max=15.0
Step=0.25

[Parameter.deconrg]
Label=Horizontal Deconvergence Green Range
Group=Horizontal/Vertical Deconvergence
Min=-15.0
Max=15.0
Step=0.25

[Parameter.deconrb]
Label=Horizontal Deconvergence Blue Range
Group=Horizontal/Vertical Deconvergence
Min=-15.0
Max=15.0
Step=0.25

[Parameter.deconrry]
Label=Vertical Deconvergence Red Range
Group=Horizontal/Vertical Deconvergence
Min=-15.0
Max=15.0
Step=0.25

[Parameter.deconrgy]
Label=Vertical Deconvergence Green Range
Group=Horizontal/Vertical Deconvergence
Min=-15.0
Max=15.0
Step=0.25

[Parameter.deconrby]
Label=Vertical Deconvergence Blue Range
Group=Horizontal/Vertical Deconvergence
Min=-15.0
Max=15.0
Step=0.25

[Parameter.decons]
Label=Deconvergence Strength
Group=Horizontal/Vertical Deconvergence
Min=0.0
Max=3.0
Step=0.1

[Parameter.addnoised]
Label=Add Noise
Group=Horizontal/Vertical Deconvergence
Min=-1.0
Max=1.0
Step=0.02

[Parameter.noiseresd]
Label=Noise Resolution
Group=Horizontal/Vertical Deconvergence
Min=1.0
Max=10.0
Step=1.0

[Parameter.noisetype]
Label=Noise Type: Colored, Luma
Group=Horizontal/Vertical Deconvergence
Min=0.0
Max=1.0
Step=1.0

[Parameter.post_br]
Label=Post Brightness
Group=Horizontal/Vertical Deconvergence
Min=0.25
Max=5.0
Step=0.01

[Parameter.oimage]
Label=Show Original Image
Group=Horizontal/Vertical Deconvergence
Min=0.0
Max=1.0
Step=0.25

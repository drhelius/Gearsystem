// Gearsystem port of bloom_horizontal.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

uniform vec4 LinearizePassSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float SIZEHB;
uniform float SIGMA_HB;
uniform float FINE_BLOOM;
uniform float auto_res;
uniform float GLOW_MAX;

/*
   Gaussian blur - horizontal pass, dynamic range, resizable
   
   Copyright (C) 2020 - 2026 guest(r)

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
   
*/ 

float auto_rez = (OriginalSize.y < 375.0) ? mix(1.0, 2.0, clamp(auto_res * round(OriginalSize.x/300.0)-1.0, 0.0,1.0)) : 1.0;
float FINE_BLOOM_ = (FINE_BLOOM > 0.5) ? FINE_BLOOM : mix(0.75, 0.5, -FINE_BLOOM);
#define SIZEHB SIZEHB * auto_rez
float SIGMA_HB_ = SIGMA_HB * SIZEHB / 3.0;

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D LinearizePass;

#define COMPAT_TEXTURE(c,d) texture(c,d)

float invsqrsigma = 1.0/(2.0*SIGMA_HB_*SIGMA_HB_);

float gaussian(float x)
{
	return exp(-x*x*invsqrsigma);
}

void main()
{
	vec4 SourceSize1 = OriginalSize * vec4(FINE_BLOOM_, FINE_BLOOM_, 1.0/FINE_BLOOM_, 1.0/FINE_BLOOM_);
	float f = fract(SourceSize1.x * vTexCoord.x);
	f = 0.5 - f;
	vec2 tex = floor(SourceSize1.xy * vTexCoord)*SourceSize1.zw + 0.5*SourceSize1.zw;
	vec4 color = vec4(0.0);
	vec2 dx  = vec2(SourceSize1.z, 0.0);

	float w;
	float wsum = 0.0;
	vec4 pixel;
	float n = -SIZEHB;

	do
	{
		pixel  = COMPAT_TEXTURE(LinearizePass, tex + n*dx);
		w = gaussian(n+f);
		pixel.a = max(max(pixel.r, pixel.g),pixel.b);
		pixel.a*=pixel.a*pixel.a;
		color  = color + w * pixel;
		wsum   = wsum + w;
		n = n + 1.0;
		
	} while (n <= SIZEHB);

	color = color / wsum;
	
	FragColor = vec4(color.rgb, pow(color.a, 0.333333));
}

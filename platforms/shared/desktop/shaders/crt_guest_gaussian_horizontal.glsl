// Gearsystem port of gaussian_horizontal.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

uniform vec4 LinearizePassSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float SIZEH;
uniform float SIGMA_H;
uniform float FINE_GLOW;
uniform float m_glow;
uniform float m_glow_cutoff;
uniform float m_glow_low;
uniform float m_glow_high;
uniform float m_glow_dist;
uniform float m_glow_mask;
uniform float auto_res;

/*
   Gaussian blur - horizontal pass, dynamic range, resizable
   
   Copyright (C) 2020-2026 guest(r)

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
float FINE_GLOW_ = (FINE_GLOW > 0.5) ? FINE_GLOW : mix(0.75, 0.5, -FINE_GLOW);
#define SIZEH SIZEH * auto_rez
float SIGMA_H_ = SIGMA_H * SIZEH / 6.0;

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D LinearizePass;

#define COMPAT_TEXTURE(c,d) texture(c,d)

float invsqrsigma = 1.0/(2.0*SIGMA_H_*SIGMA_H_);

float gaussian(float x)
{
	return exp(-x*x*invsqrsigma);
}

vec3 plant (vec3 tar, float r)
{
	float t = max(max(tar.r,tar.g),tar.b) + 0.00001;
	return tar * r / t;
}

void main()
{
	vec4 SourceSize1 = OriginalSize*vec4(FINE_GLOW_, FINE_GLOW_, 1.0/FINE_GLOW_, 1.0/FINE_GLOW_);
	float f = fract(SourceSize1.x * vTexCoord.x);
	f = 0.5 - f;
	vec2 tex = floor(SourceSize1.xy * vTexCoord)*SourceSize1.zw + 0.5*SourceSize1.zw;
	vec3 color = vec3(0.0);
	vec2 dx  = vec2(SourceSize1.z, 0.0);

	float w;
	float wsum = 0.0;
	vec3 pixel;
	float n = -SIZEH;

	do
	{
		pixel  = COMPAT_TEXTURE(LinearizePass, tex + n*dx).rgb;
		if (m_glow > 0.5)
		{
			pixel = max(pixel-m_glow_cutoff, 0.0);
			pixel = plant(pixel, max(max(max(pixel.r,pixel.g),pixel.b)-m_glow_cutoff,0.0));
		}
		w      = gaussian(n+f);
		color  = color + w * pixel;
		wsum   = wsum + w;
		n = n + 1.0;
		
	} while (n <= SIZEH);

	color = color / wsum;
	
	FragColor = vec4(color, 1.0);
}

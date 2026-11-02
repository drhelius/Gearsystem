// Gearsystem port of gaussian_vertical.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

uniform vec4 SourceSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float SIZEV;
uniform float SIGMA_V;
uniform float FINE_GLOW;

/*
   Gaussian blur - vertical pass, dynamic range, resizable
   
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
float SIGMA_V_ = SIGMA_V * SIZEV / 6.0;
float FINE_GLOW_ = (FINE_GLOW > 0.5) ? FINE_GLOW : mix(0.75, 0.5, -FINE_GLOW);

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D Source;

#define COMPAT_TEXTURE(c,d) texture(c,d)

float invsqrsigma = 1.0/(2.0*SIGMA_V_*SIGMA_V_);

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
	vec4 SourceSize1 = vec4(SourceSize.x, OriginalSize.y, SourceSize.z, OriginalSize.w)*vec4(FINE_GLOW_, FINE_GLOW_, 1.0/FINE_GLOW_, 1.0/FINE_GLOW_);
	float f = fract(SourceSize1.y * vTexCoord.y);
	f = 0.5 - f;
	vec2 tex = floor(SourceSize1.xy * vTexCoord)*SourceSize1.zw + 0.5*SourceSize1.zw;
	vec3 color = vec3(0.0);
	vec2 dy  = vec2(0.0, SourceSize1.w);

	float w;
	float wsum = 0.0;
	vec3 pixel;
	float n = -SIZEV;

	do
	{
		pixel  = COMPAT_TEXTURE(Source, tex + n*dy).rgb;
		w      = gaussian(n+f);
		color  = color + w * pixel;
		wsum   = wsum + w;
		n = n + 1.0;
		
	} while (n <= SIZEV);

	color = color / wsum;
	
	FragColor = vec4(color, 1.0);
}

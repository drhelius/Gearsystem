// Gearsystem port of crt-guest-advanced-hd-pass1.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

/*
   CRT - Guest - Advanced - HD - Pass1
   
   Copyright (C) 2018-2025 guest(r)

   Incorporates many good ideas and suggestions from Dr. Venom.
   I would also like give thanks to many Libretro forums members for continuous feedback, suggestions and using the shader.
   
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

uniform vec4 SourceSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float SIGMA_HOR;
uniform float HSHARPNESS;
uniform float S_SHARP;
uniform float HARNG;
uniform float spike;
uniform float SIGMA_VER;
uniform float VSHARPNESS;
uniform float internal_res;
uniform float auto_res;
uniform float MAXS;

float auto_rez = (OriginalSize.y < 375.0) ? mix(1.0, 2.0, clamp(auto_res * round(OriginalSize.x/300.0)-1.0, 0.0,1.0)) : 1.0;

float internal_res_ = internal_res;
float HSHARPNESS_ = HSHARPNESS * auto_rez;
float SIGMA_HOR_ = SIGMA_HOR * HSHARPNESS_;
#define SIGMA_VER SIGMA_VER*VSHARPNESS
#define COMPAT_TEXTURE(c,d) texture(c,d)
#define TEX0 vTexCoord
#define gl_FragCoord (vTexCoord * OutputSize.xy)

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D LinearizePass;

float invsqrsigma = 1.0/(2.0*SIGMA_HOR_*SIGMA_HOR_*internal_res_*internal_res_);

float gaussian(float x)
{
	return exp(-x*x*invsqrsigma);
}

void main()
{	
	vec2 prescalex = vec2(textureSize(LinearizePass, 0)) / OriginalSize.xy;

	vec4 SourceSize_ = OriginalSize * vec4(prescalex.x, prescalex.y, 1.0/prescalex.x, 1.0/prescalex.y);
	
	float f = fract(SourceSize_.x * vTexCoord.x);
	f = 0.5 - f;
	vec2 tex = floor(SourceSize_.xy * vTexCoord)*SourceSize_.zw + 0.5*SourceSize_.zw;
	vec3 color = vec3(0.0);
	float scolor = 0.0;
	vec2 dx  = vec2(SourceSize_.z, 0.0);

	float w = 0.0;
	float swsum = 0.0;
	float wsum = 0.0;
	vec3 pixel;

	float hsharpness = HSHARPNESS_ * internal_res_;
	vec3 cmax = vec3(0.0);
	vec3 cmin = vec3(1.0);
	float sharp = gaussian(hsharpness) * S_SHARP;
	float ksharp = -2.0*hsharpness*invsqrsigma*sharp;
	float maxsharp = MAXS;
	float FPR = hsharpness;
	float FPRi = 1.0/hsharpness;
	float fpx = 0.0;
	float sp = 0.0;
	float sw = 0.0;

	float ts = 0.025;
	vec3 luma = vec3(0.2126, 0.7152, 0.0722); 
	float LOOPSIZE = ceil(2.0*FPR);
	
	float n = -LOOPSIZE;
	
	do
	{
		pixel  = COMPAT_TEXTURE(LinearizePass, tex + n*dx).rgb;
		
		w = gaussian(n+f) - sharp;
		fpx = clamp((abs(n+f)-FPR)*FPRi, 0.0, 1.0);
		if (w < 0.0) w = max(ksharp*fpx, maxsharp * ksharp * (1.0-fpx*fpx));
		else
		{
			cmax = max(cmax, pixel); cmin = min(cmin, pixel);
			sw = w * (dot(pixel,luma) + ts); 
			sp = max(max(pixel.r,pixel.g),pixel.b);
			scolor = scolor + sw * sp;
			swsum = swsum + sw;			
		}
		color = color + w * pixel;
		wsum  = wsum + w;

		n = n + 1.0;
			
	} while (n <= LOOPSIZE);

	color = color / wsum;
	scolor = scolor / swsum;
	
	color = clamp(mix(clamp(color, cmin, cmax), color, HARNG), 0.0, 1.0);
	
	scolor = clamp(mix(max(max(color.r, color.g),color.b), scolor, spike), 0.0, 1.0);
	
	FragColor = vec4(color, scolor);
}

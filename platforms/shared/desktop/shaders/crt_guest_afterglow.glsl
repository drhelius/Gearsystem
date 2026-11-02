// Gearsystem port of afterglow0.slang from libretro slang-shaders (crt-guest-advanced 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

/*
   Phosphor Afterglow Shader pass 0
   
   Copyright (C) 2020 - 2025 guest(r)

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
uniform float PR;
uniform float PG;
uniform float PB;
uniform float esrc;
uniform float bth;

#define COMPAT_TEXTURE(c,d) texture(c,d)

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D Source;
uniform sampler2D Original;
uniform sampler2D AfterglowPassFeedback;

#define TEX0 vTexCoord

void main()
{
	vec2 dx = vec2(OriginalSize.z, 0.0);
	vec2 dy = vec2(0.0, OriginalSize.w);
	
	vec3 color0  = COMPAT_TEXTURE(Original, TEX0.xy).rgb;
	vec3 color1  = COMPAT_TEXTURE(Original, TEX0.xy - dx).rgb;
	vec3 color2  = COMPAT_TEXTURE(Original, TEX0.xy + dx).rgb;
	vec3 color3  = COMPAT_TEXTURE(Original, TEX0.xy - dy).rgb;
	vec3 color4  = COMPAT_TEXTURE(Original, TEX0.xy + dy).rgb;	

if (esrc > 1.5)
{
	color0  = COMPAT_TEXTURE(Source, TEX0.xy).rgb;
	color1  = COMPAT_TEXTURE(Source, TEX0.xy - dx).rgb;
	color2  = COMPAT_TEXTURE(Source, TEX0.xy + dx).rgb;
	color3  = COMPAT_TEXTURE(Source, TEX0.xy - dy).rgb;
	color4  = COMPAT_TEXTURE(Source, TEX0.xy + dy).rgb;	
}

	vec3 color = (2.5 * color0 + color1 + color2 + color3 + color4)/6.5;
	
	vec3 accumulate = COMPAT_TEXTURE(AfterglowPassFeedback, TEX0.xy).rgb;

	float w = 1.0;
	float b = bth/255.0;
	float c = max(max(color0.r,color0.g),color0.b);
	w = smoothstep(b, 2.0*b, c);

	vec3 result = mix( max(mix(color, accumulate, 0.49 + vec3(PR, PG, PB))- 0.5/255.0, 0.0), color, w);

	FragColor = vec4(result, w);
}

// Gearsystem port of linearize-hd.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

uniform vec4 OriginalSize;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float GAMMA_INPUT;
uniform float inter;
uniform float interm;
uniform float iscan;
uniform float intres;
uniform float iscans;
uniform float gamma_out;
uniform float vga_mode;
uniform float hiscan;

/*
   Interlacing
   
   Copyright (C) 2020-2025 guest(r)

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
in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D Source;

#define COMPAT_TEXTURE(c,d) texture(c,d)

vec3 plant (vec3 tar, float r)
{
	float t = max(max(tar.r,tar.g),tar.b) + 0.00001;
	return tar * r / t;
}

void main()
{
	vec3 c1 = COMPAT_TEXTURE(Source, vTexCoord).rgb;
	vec3 c2 = COMPAT_TEXTURE(Source, vTexCoord + vec2(0.0, SourceSize.w)).rgb;

	vec3  c  = c1;

	float intera = 1.0;
	float gamma_in = GAMMA_INPUT;

	float m1 = max(max(c1.r,c1.g),c1.b);
	float m2 = max(max(c2.r,c2.g),c2.b);
	vec3 df = abs(c1-c2);
		
	float d = max(max(df.r,df.g),df.b);
	if (interm == 2.0) d = mix(0.1*d,10.0*d, step(m1/(m2+0.0001),m2/(m1+0.0001)));

	float r = m1;

	float yres_div = 1.0; if (intres > 1.25) yres_div = intres;
	
	bool hscan = (hiscan > 0.5);
	
	if (inter <= OriginalSize.y/yres_div && interm > 0.5 && intres != 1.0  && intres != 0.5 && vga_mode < 0.5 || hscan)
	{
		intera = 0.25;
		float line_no  = floor(mod(OriginalSize.y*vTexCoord.y, 2.0));
		float frame_no = floor(mod(float(FrameCount),2.0));
		float ii = abs(line_no-frame_no);
		
		if (interm < 3.5 || interm > 4.5)
		{
			if (interm == 5.0) c = mix(c2, c1, ii); else
			{
			c2 = plant(mix(c2, c2*c2, iscans), max(max(c2.r,c2.g),c2.b));
			r = max(m1*ii, (1.0-iscan)*min(m1,m2));
			c = plant( mix(mix(c1,c2, min(mix(m1, 1.0-m2, min(m1,1.0-m1))/(d+0.00001),1.0)), c1, ii), r);
			if (interm == 3.0) c = (1.0-0.5*iscan)*mix(c2, c1, ii);
			}
		}
		if (interm == 4.0) { c = plant(mix(c, c*c, 0.5*iscans), max(max(c.r,c.g),c.b)) * (1.0-0.5*iscan); }
	
		if (hscan) c = c1;
	}
	
	if (vga_mode > 0.5)
	{
		c = c1;
		if (inter <= OriginalSize.y) intera = 0.75; else intera = 0.5;
	}	
	
	c = pow(c, vec3(gamma_in));
	
	if (vTexCoord.x > 0.5) gamma_in = intera; else gamma_in = 1.0/gamma_in;
	
	FragColor = vec4(c, gamma_in);
}

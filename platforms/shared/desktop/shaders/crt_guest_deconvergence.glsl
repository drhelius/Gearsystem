// Gearsystem port of deconvergence-hd.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

/*
   CRT - Guest - Advanced
   
   Copyright (C) 2018-2026 guest(r)

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

uniform float IOS;
uniform float brightboost;
uniform float brightboost1;
uniform float csize;
uniform float bsize1;
uniform float warpX;
uniform float warpY;
uniform float cbulge;
uniform float glow;
uniform float shadowMask;
uniform float masksize;
uniform float slotmask;
uniform float slotmask1;
uniform float slotwidth;
uniform float double_slot;
uniform float mcut;
uniform float maskDark;
uniform float maskLight;
uniform float maskstr;
uniform float mshift;
uniform float mask_layout;
uniform float mask_bloom;
uniform float hiscan;
uniform float pr_scan;
uniform vec4 SourceSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float bloom;
uniform float halation;
uniform float slotms;
uniform float mask_gamma;
uniform float gamma_out;
uniform float overscanX;
uniform float overscanY;
uniform float VShift;
uniform float intres;
uniform float prescalex;
uniform float c_shape;
uniform float barspeed;
uniform float barintensity;
uniform float bardir;
uniform float sborder;
uniform float bloom_dist;
uniform float deconr;
uniform float decons;
uniform float addnoised;
uniform float noiseresd;
uniform float noisetype;
uniform float deconrr;
uniform float deconrg;
uniform float deconrb;
uniform float deconrry;
uniform float deconrgy;
uniform float deconrby;
uniform float dctypex;
uniform float dctypey;
uniform float post_br;
uniform float maskboost;
uniform float smoothmask;
uniform float gamma_c;
uniform float gamma_c2;
uniform float m_glow;
uniform float m_glow_low;
uniform float m_glow_high;
uniform float m_glow_dist;
uniform float m_glow_mask;
uniform float smask_mit;
uniform float mask_zoom;
uniform float no_scanlines;
uniform float bmask;
uniform float bmask1;
uniform float hmask1;
uniform float mzoom_sh;
uniform float mclip;
uniform float edgemask;
uniform float oimage;
uniform float interm;
uniform float laysh;

#define COMPAT_TEXTURE(c,d) texture(c,d)
#define TEX0 vTexCoord
#define gl_FragCoord (vTexCoord * OutputSize.xy)

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D LinearizePass;
uniform sampler2D BloomPass;
uniform sampler2D PrePass;
uniform sampler2D Source;
uniform sampler2D GlowPass;
uniform sampler2D StockPass;

#define eps 1e-10 

// Shadow mask (1-4 from PD CRT Lottes shader).

vec3 Mask(vec2 pos, float mx, float mb)
{
	vec3 mask = vec3(maskDark, maskDark, maskDark);
	vec3 one = vec3(1.0);
	
	if (shadowMask == 0.0)
	{
		float mc = 1.0 - max(maskstr, 0.0);	
		pos.x = fract(pos.x*0.5);
		if (pos.x < 0.49) { mask.r = 1.0; mask.g = mc; mask.b = 1.0; }
		else { mask.r = mc; mask.g = 1.0; mask.b = mc; }
	}    
   
	// Very compressed TV style shadow mask.
	else if (shadowMask == 1.0)
	{
		float line = maskLight;
		float odd  = 0.0;

		if (fract(pos.x/6.0) < 0.49)
			odd = 1.0;
		if (fract((pos.y + odd)/2.0) < 0.49)
			line = maskDark;

		pos.x = floor(mod(pos.x,3.0));
    
		if      (pos.x < 0.5) mask.r = maskLight;
		else if (pos.x < 1.5) mask.g = maskLight;
		else                  mask.b = maskLight;
		
		mask*=line;
	} 

	// Aperture-grille.
	else if (shadowMask == 2.0)
	{
		pos.x = floor(mod(pos.x,3.0));

		if      (pos.x < 0.5) mask.r = maskLight;
		else if (pos.x < 1.5) mask.g = maskLight;
		else                  mask.b = maskLight;
	} 

	// Stretched VGA style shadow mask (same as prior shaders).
	else if (shadowMask == 3.0)
	{
		pos.x += pos.y*3.0;
		pos.x  = fract(pos.x/6.0);

		if      (pos.x < 0.3) mask.r = maskLight;
		else if (pos.x < 0.6) mask.g = maskLight;
		else                  mask.b = maskLight;
	}

	// VGA style shadow mask.
	else if (shadowMask == 4.0)
	{
		pos.xy = floor(pos.xy*vec2(1.0, 0.5));
		pos.x += pos.y*3.0;
		pos.x  = fract(pos.x/6.0);

		if      (pos.x < 0.3) mask.r = maskLight;
		else if (pos.x < 0.6) mask.g = maskLight;
		else                  mask.b = maskLight;
	}
	
	// Trinitron mask 5
	else if (shadowMask == 5.0)
	{
		mask = vec3(0.0);		
		pos.x = fract(pos.x/2.0);
		if  (pos.x < 0.49)
		{	mask.r  = 1.0;
			mask.b  = 1.0;
		}
		else     mask.g = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}    

	// Trinitron mask 6
	else if (shadowMask == 6.0)
	{
		mask = vec3(0.0);
		pos.x = floor(mod(pos.x,3.0));
		if      (pos.x < 0.5) mask.r = 1.0;
		else if (pos.x < 1.5) mask.g = 1.0;
		else                    mask.b = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}
	
	// BW Trinitron mask 7
	else if (shadowMask == 7.0)
	{
		mask = vec3(0.0);		
		pos.x = fract(pos.x/2.0);
		if  (pos.x < 0.49)
		{	mask  = vec3(0.0);
		}
		else     mask = vec3(1.0);
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}    

	// BW Trinitron mask 8
	else if (shadowMask == 8.0)
	{
		mask = vec3(0.0);
		pos.x = fract(pos.x/3.0);
		if      (pos.x < 0.3) mask = vec3(0.0);
		else if (pos.x < 0.6) mask = vec3(1.0);
		else                  mask = vec3(1.0);
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}    

	// Magenta - Green - Black mask
	else if (shadowMask == 9.0)
	{
		mask = vec3(0.0);
		pos.x = fract(pos.x/3.0);
		if      (pos.x < 0.3) mask    = vec3(0.0);
		else if (pos.x < 0.6) mask.rb = vec2(1.0);
		else                  mask.g  = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);	
	}  
	
	// RGBX
	else if (shadowMask == 10.0)
	{
		mask = vec3(0.0);
		pos.x = fract(pos.x * 0.25);
		if      (pos.x < 0.2)  mask  = vec3(0.0);
		else if (pos.x < 0.4)  mask.r = 1.0;
		else if (pos.x < 0.7)  mask.g = 1.0;	
		else                   mask.b = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}  

	// 4k mask
	else if (shadowMask == 11.0)
	{
		mask = vec3(0.0);
		pos.x = fract(pos.x * 0.25);
		if      (pos.x < 0.2)  mask.r  = 1.0;
		else if (pos.x < 0.4)  mask.rg = vec2(1.0);
		else if (pos.x < 0.7)  mask.gb = vec2(1.0);	
		else                   mask.b  = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}     

	// RRGGBBX mask
	else if (shadowMask == 12.0)
	{
		mask = vec3(0.0);
		pos.x = floor(mod(pos.x,7.0));
		if      (pos.x < 0.5)  mask   = vec3(0.0);
		else if (pos.x < 2.5)  mask.r = 1.0;
		else if (pos.x < 4.5)  mask.g = 1.0;	
		else                   mask.b = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}

	// 4k mask
	else if (shadowMask == 13.0)
	{
		mask = vec3(0.0);
		pos.x = floor(mod(pos.x,6.0));
		if      (pos.x < 0.5)  mask   = vec3(0.0);
		else if (pos.x < 1.5)  mask.r = 1.0;
		else if (pos.x < 2.5)  mask.rg = vec2(1.0);
		else if (pos.x < 3.5)  mask.rgb = vec3(1.0);
		else if (pos.x < 4.5)  mask.gb = vec2(1.0);		
		else                   mask.b = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	}

	// 4k mask
	else
	{
		mask = vec3(0.0);
		pos.x = floor(mod(pos.x, 5.0));
		if      (pos.x < 0.5)  mask    = vec3(0.0);
		else if (pos.x < 1.5)  mask.r  = 1.0;
		else if (pos.x < 2.5)  mask.rg = vec2(1.0);
		else if (pos.x < 3.5)  mask.gb = vec2(1.0);	
		else                   mask.b  = 1.0;
		mask = clamp(mix( mix(one, mask, mcut), mix(one, mask, maskstr), mx), 0.0, 1.0);
	} 
	
	if (mask_layout > 0.5) mask = mask.rbg;
	float maskmin = min(min(mask.r,mask.g),mask.b);
	return (mask - maskmin) * (1.0 + (maskboost-1.0)*mb) + maskmin;
}

float SlotMask(vec2 pos, float m, float swidth)
{
	if ((slotmask + slotmask1) == 0.0) return 1.0;
	else
	{
	pos.y = floor(pos.y/slotms);
	float mlen = swidth*2.0;
	float px = floor(mod(pos.x, 0.99999*mlen));
	float py = floor(fract(pos.y/(2.0*double_slot))*2.0*double_slot);
	float slot_dark = mix(1.0-slotmask1, 1.0-slotmask, m);
	float slot = 1.0;
	if (py == 0.0 && px < swidth) slot = slot_dark; else
	if (py == double_slot && px >= swidth) slot = slot_dark;		
	
	return slot;
	}
}
 
vec2 Warp(vec2 pos)
{
	pos  = pos*2.0-1.0;
	pos = mix(pos, sign(pos)*pow(abs(pos), 1.0 + min(15.0*vec2(warpY,warpX)*vec2(cbulge), vec2(cbulge))), 0.075 + 0.125*(abs(pos)+pos*pos));
	pos = mix(pos, vec2(pos.x*inversesqrt(1.0-c_shape*pos.y*pos.y), pos.y*inversesqrt(1.0-c_shape*pos.x*pos.x)), vec2(warpX, warpY)/c_shape);
	return pos*0.5 + 0.5;
}

vec2 Overscan(vec2 pos, float dx, float dy){
	pos=pos*2.0-1.0;    
	pos*=vec2(dx,dy);
	return pos*0.5+0.5;
} 

float humbar(float pos)
{
	if (barintensity == 0.0) return 1.0; else
	{
		pos = (barintensity >= 0.0) ? pos : (1.0-pos);
		pos = fract(pos + mod(float(FrameCount),barspeed)/(barspeed-1.0));
		pos = (barintensity <  0.0) ? pos : (1.0-pos);	
		return (1.0-barintensity) + barintensity*pos;
	}	
}

float corner(vec2 pos) {
	pos = abs(2.0*(pos - 0.5));
	vec2 aspect = vec2(1.0, OutputSize.x/OutputSize.y);	
	float b = bsize1 * 0.05 + 0.0005; pos.y = pos.y + b*(aspect.y - 1.0);	
	vec2 crn = max(vec2(csize),2.0*b+0.0015);
	vec2 cp = max(pos-(1.0-crn*aspect),0.0)/aspect; float cd = sqrt(dot(cp,cp));
	pos = max(pos, 1.0-crn+cd);
	float res = mix(1.0, 0.0, smoothstep(1.0-b, 1.0, sqrt(max(pos.x,pos.y))));
	return pow(res, sborder);	
}

vec3 plant (vec3 tar, float r)
{
	float t = max(max(tar.r,tar.g),tar.b) + 0.00001;
	return tar * r / t;
}

vec3 declip(vec3 c, float b)
{
	float m = max(max(c.r,c.g),c.b);
	if (m > b) c = c*b/m;
	return c;
}

float igc(float mc)
{
	return pow(mc, gamma_c);
}

vec3 gc2(vec3 c, float w3)
{
	float mc = max(max(c.r,c.g),c.b);
	float gp = 1.0/(1.0 + (gamma_c2 - 1.0)*mix(0.375, 1.0, w3));
	float mg = pow(mc, gp);
	return c * mg/(mc + eps);  
}

// noise function:
// Dedicated to the public domain.
// If you want a real license, you may consider this MIT/BSD/CC0/WTFPL-licensed (take your pick).
// Adapted from ChuckNorris - shadertoy: https://www.shadertoy.com/view/XtK3Dz

vec3 noise(vec3 v){
    if (addnoised < 0.0) v.z = -addnoised; else v.z = mod(v.z,6001.0)/1753.0;
	// ensure reasonable range
    v = fract(v) + fract(v*1e4) + fract(v*1e-4);
    // seed
    v += vec3(0.12345, 0.6789, 0.314159);
    // more iterations => more random
    v = fract(v*dot(v, v)*123.456);
    v = fract(v*dot(v, v)*123.456);
	v = fract(v*dot(v, v)*123.456);
	v = fract(v*dot(v, v)*123.456);	
    return v;
} 

void fetch_pixel (inout vec3 c, inout vec3 b, inout vec3 g, vec2 coord, vec2 bcoord)
{
		float stepx = OutputSize.z;
		float stepy = OutputSize.w;
		
		float ds = decons;
				
		vec2 dx = vec2(stepx, 0.0);
		vec2 dy = vec2(0.0, stepy);		
		
		float posx = 2.0*coord.x - 1.0;
		float posy = 2.0*coord.y - 1.0;
		
		if (dctypex > 0.025)
		{
			posx = sign(posx)*pow(abs(posx), 1.05-dctypex);
			dx = posx * dx;
		}

		if (dctypey > 0.025)
		{

			posy = sign(posy)*pow(abs(posy), 1.05-dctypey);
			dy = posy * dy;
		}

		vec2 rc = deconrr * dx + deconrry*dy;
		vec2 gc = deconrg * dx + deconrgy*dy;
		vec2 bc = deconrb * dx + deconrby*dy;		

		float r1 = COMPAT_TEXTURE(Source, coord + rc).r;
		float g1 = COMPAT_TEXTURE(Source, coord + gc).g;
		float b1 = COMPAT_TEXTURE(Source, coord + bc).b;

		vec3 d = vec3(r1, g1, b1);
		c = clamp(mix(c, d, ds), 0.0, 1.0);
		
		r1 = COMPAT_TEXTURE(BloomPass, bcoord + rc).r;
		g1 = COMPAT_TEXTURE(BloomPass, bcoord + gc).g;
		b1 = COMPAT_TEXTURE(BloomPass, bcoord + bc).b;

		d = vec3(r1, g1, b1);
		b = g = mix(b, d, min(ds,1.0));
		
		r1 = COMPAT_TEXTURE(GlowPass, bcoord + rc).r;
		g1 = COMPAT_TEXTURE(GlowPass, bcoord + gc).g;
		b1 = COMPAT_TEXTURE(GlowPass, bcoord + bc).b;

		d = vec3(r1, g1, b1);
		g = mix(g, d, min(ds,1.0));
}

void main()
{
	vec4 SourceSize_ = OriginalSize;
	
	float gamma_in = 1.0/COMPAT_TEXTURE(LinearizePass, vec2(0.25,0.25)).a;
	float intera = COMPAT_TEXTURE(LinearizePass, vec2(0.75,0.25)).a;
	bool inter6 = (intera < 0.35 && interm == 5.0);
	bool interb  = (((intera < 0.35 && !inter6) || no_scanlines > 0.025) && hiscan < 0.5);
	
	// Calculating texel coordinates
   
	vec2 texcoord = TEX0.xy;

	if (IOS > 0.0 && !interb){
		vec2 ofactor = OutputSize.xy/OriginalSize.xy;
		vec2 intfactor = (IOS < 2.5) ? floor(ofactor) : ceil(ofactor);
		vec2 diff = ofactor/intfactor;
		float scan = diff.y;
		texcoord = Overscan(texcoord, scan, scan);
		if (IOS == 1.0 || IOS == 3.0) texcoord = vec2(TEX0.x, texcoord.y);
	} 

	texcoord.y = texcoord.y - VShift*OriginalSize.w;
	texcoord = Overscan(texcoord, (OriginalSize.x - overscanX)/OriginalSize.x, (OriginalSize.y - overscanY)/OriginalSize.y);

	vec2 pos1 = TEX0.xy;
	vec2 pos  = Warp(texcoord);
	vec2 pos0 = Warp(TEX0.xy);
	vec2 posb = 2.0*(pos0-0.5);
	posb = max(abs(posb), abs(2.0*(pos-0.5)))*sign(posb);
	posb = 0.5*posb + 0.5;
	
	// color and bloom fetching
	vec3 color = COMPAT_TEXTURE(Source,pos1).rgb;
	vec3  Bloom = COMPAT_TEXTURE(BloomPass, pos).rgb;
	vec3  Glow = COMPAT_TEXTURE(GlowPass, pos).rgb;

if ((abs(deconrr) + abs(deconrry) + abs(deconrg) + abs(deconrgy) + abs(deconrb) + abs(deconrby)) > 0.2)
	fetch_pixel(color, Bloom, Glow, pos1, pos); // deconvergence
	
	float cm = igc(max(max(color.r,color.g),color.b));
	float mx1 = COMPAT_TEXTURE(Source, pos1     ).a;
	float colmx = max(mx1, cm);
	float w3 = min((max((cm - 0.0005)*1.0005 ,0.0) + 0.0001) / (colmx + 0.0005), 1.0); if(interb) w3 = 1.0;
	
	vec2 dx = vec2(0.001, 0.0);
	float mx0 = COMPAT_TEXTURE(Source, pos1 - dx).a;
	float mx2 = COMPAT_TEXTURE(Source, pos1 + dx).a;
	float mxg = max(max(mx0,mx1),max(mx2,cm));
	float mx = pow(mxg, 1.40/gamma_in);
	float cx = pow(colmx, 1.4/gamma_in);
	
	vec3 one = vec3(1.0);

	// mask boost tweak
	
	dx = vec2(OriginalSize.z, 0.0)*0.25;
	mx0 = COMPAT_TEXTURE(Source, pos1 - dx).a;
	mx2 = COMPAT_TEXTURE(Source, pos1 + dx).a;
	float mb = (1.0 - min(abs(mx0-mx2)/(0.5+mx1), 1.0));
	
	// Apply Mask
	
	vec3 orig1 = color;
	vec3 cmask = one;
	vec3 cmask1 = one;
	vec3 cmask2 = one;

	// mask widths and mask dark compensate (fractional part) values
	float mwidths[15] = float[15] (2.0, 3.0, 3.0, 6.0, 6.0, 2.4, 3.4, 2.4, 3.25, 3.4, 4.4, 4.25, 7.4, 6.25, 5.25); 
	
	float mwidth = mwidths[int(shadowMask)];
	float mask_compensate = fract(mwidth);
	float mwidth1 = mwidth;
	
if (shadowMask > -0.5)
{	
	vec2 maskcoord = gl_FragCoord.xy + vec2(laysh*masksize, 0.0);
	vec2 scoord = maskcoord;
	
	mwidth = floor(mwidth) * masksize;
	float swidth = mwidth;
	bool zoomed = (abs(mask_zoom) > 0.75);
	float mscale = 1.0;
	vec2 maskcoord0 = maskcoord;
	maskcoord.y = floor(maskcoord.y/masksize);
	mwidth1 = max(mwidth + mask_zoom, 2.0);

if ( mshift > 0.25 )
{	
	float stagg_lvl = 1.0; if (fract(mshift) > 0.25) stagg_lvl = 2.0;
	float next_line = float(floor(mod(maskcoord.y, 2.0*stagg_lvl)) < stagg_lvl);
	maskcoord0.x = maskcoord0.x + next_line * 0.5 * mwidth1;
}		
	maskcoord = maskcoord0/masksize;

if ( !zoomed )	
	cmask = Mask(floor(maskcoord), mx, mb);
else{
	mscale  = mwidth1/mwidth;
	float mlerp = fract(maskcoord.x/mscale); if (mzoom_sh > 0.025) mlerp = clamp((1.0 + mzoom_sh)*mlerp - 0.5*mzoom_sh, 0.0, 1.0);
	float mcoord = floor(maskcoord.x/mscale);
	if (shadowMask == 12.0 && mask_zoom == -2.0) mcoord = ceil(maskcoord.x/mscale);
	cmask = mix(Mask(vec2(mcoord,maskcoord.y), mx, mb), Mask(vec2(mcoord + 1.0, maskcoord.y), mx, mb), mlerp);
}

	float sm_offset = 0.0; bool bsm_offset = (shadowMask == 0.0 || shadowMask == 2.0 || shadowMask == 5.0 || shadowMask == 6.0 || shadowMask == 8.0 || shadowMask == 11.0);
	if( zoomed ) { if (mask_layout < 0.5 && bsm_offset) sm_offset = 1.0; else if (bsm_offset) sm_offset = -1.0; }
	
	swidth = round(mwidth1); if (slotwidth > 0.5) swidth = slotwidth; float smask = 1.0;
	smask = SlotMask(scoord + vec2(sm_offset,0.0), mx, swidth);

	smask = clamp(smask + mix(smask_mit, 0.0, w3*pow(colmx, 0.3)), 0.0, 1.0);
	
	cmask2 = cmask;
	cmask*=smask;
	cmask1 = cmask;

	if (abs(mask_bloom) > 0.025)
	{
		float maxbl = max(max(max(Bloom.r,Bloom.g),Bloom.b), mxg);
		maxbl = maxbl * max(mix(1.0, 2.0-colmx, bloom_dist), 0.0);
		if (mask_bloom > 0.025) cmask = max(min(cmask + maxbl*mask_bloom, 1.0), cmask); else cmask = max(mix(cmask, cmask*(1.0-0.5*maxbl) + plant(pow(Bloom,vec3(0.35)),maxbl), -mask_bloom),cmask);
	}
	
	color = pow(color, vec3(mask_gamma/gamma_in));
	color = color*cmask;
	color = min(color,1.0);
	color = pow(color, vec3(gamma_in/mask_gamma));

	cmask  = min(cmask, 1.0);
	cmask1 = min(cmask1, 1.0);
}

	float dark_compensate  = mix(max( clamp( mix (mcut, maskstr, mx),0.0, 1.0) - 1.0 + mask_compensate, 0.0) + 1.0, 1.0, mx); if(shadowMask < -0.5) dark_compensate = 1.0;
	float bb = mix(brightboost, brightboost1, mx) * dark_compensate;
	color*=bb;

	vec3  Ref = COMPAT_TEXTURE(LinearizePass, pos).rgb;	
	float maxb = COMPAT_TEXTURE(BloomPass, pos).a;
	float vig  = COMPAT_TEXTURE(PrePass, clamp(pos, 0.0+0.5*OriginalSize.zw, 1.0-0.5*OriginalSize.zw)).a;

	vec3 Bloom1 = Bloom;
	vec3 bcmask = mix(one, cmask1, bmask1);
	vec3 hcmask = mix(one, cmask1, hmask1);

	if(pr_scan > 0.025)
	{
		float mbl = max(max(Bloom.r,Bloom.g),Bloom.b);
		Bloom = mix(Bloom, mix(Bloom, plant(orig1,mbl), min(2.5*(1.0-w3),1.0)), min(2.0*pr_scan, 1.0));
	}	

	if (abs(bloom) > 0.025)
	{
		if (bloom < -0.01) Bloom1 = plant(Bloom, maxb);
		Bloom1 = min(Bloom1*(orig1+color), max(0.5*(colmx + orig1 - color),0.001*Bloom1));
		Bloom1 = 0.5*(Bloom1 + mix(Bloom1, mix(colmx*orig1, Bloom1, 0.5), 1.0-color));
		Bloom1 = bcmask*Bloom1 * max(mix(1.0, 2.0-colmx, bloom_dist), 0.0);
		color = pow(pow(color, vec3(mask_gamma/gamma_in)) + abs(bloom) * pow(Bloom1, vec3(mask_gamma/gamma_in)), vec3(gamma_in/mask_gamma));
	}

	if (halation > 0.01) {
		Bloom = 0.5*(Bloom + Bloom*Bloom);
		float mbl = max(max(Bloom.r,Bloom.g),Bloom.b); float cmxh = 0.5*(colmx + colmx*colmx);
		mbl = mix(mix(cmxh,mix(cmxh,mbl,mbl),colmx), mbl, mb);
		Bloom = plant(Bloom, mix(sqrt(mbl*cmxh), max((mbl - 0.15*(1.0-colmx)), 0.4*cmxh), pow(colmx, 0.25))) * mix(0.425, 1.0, colmx);
		Bloom = (3.0 - colmx - color)*plant(0.325+orig1/w3, 0.5*(1.0+w3))*hcmask*Bloom;
		color = pow(pow(color, vec3(mask_gamma/gamma_in)) + halation * pow(Bloom, vec3(mask_gamma/gamma_in)), vec3(gamma_in/mask_gamma));
	}
	else
	if (halation < -0.01) {
		float mbl = max(max(Bloom.r,Bloom.g),Bloom.b);
		Bloom = plant(Bloom + Ref + orig1 + Bloom*Bloom*Bloom, min(mbl*mbl,0.75));
		Bloom = 2.0*mix(1.0,w3,0.5*colmx)*hcmask*Bloom;
		color = color -halation*Bloom;
	}

	color = min(color,1.0);
	color = gc2(color,w3);
	
	if (smoothmask > 0.125) { float w4 = pow(w3, 0.425 + 0.3*smoothmask); w4 = max(w4 - 0.175*colmx*smoothmask, 0.2); color = mix(min(color/w4, plant(orig1,1.0 + 0.175*colmx*smoothmask))*w4, color, w4); }

	if (m_glow < 0.5) Glow = mix(Glow, 0.25*color, colmx);
	else
	{ 
		maxb = max(max(Glow.r,Glow.g),Glow.b);
		vec3 orig2 = plant(orig1 + 0.001*Ref, 1.0);
		Bloom = plant(Glow, 1.0);
		Ref = abs(orig2-Bloom);
		mx0 = max(max(orig2.r,orig2.g),orig2.b)-min(min(orig2.r,orig2.g),orig2.b);
		mx2 = max(max(Bloom.r,Bloom.g),Bloom.b)-min(min(Bloom.r,Bloom.g),Bloom.b);		
		Bloom = mix(maxb*min(Bloom,orig2), mix(mix(Glow, max(max(Ref.r,Ref.g),Ref.b)*Glow, max(mx,mx0)), mix(color, Glow, mx2), max(mx0,mx2)*Ref), min(sqrt((1.10-mx0)*(0.10+mx2)),1.0));
		if (m_glow > 1.5) Glow = mix(0.5*Glow*Glow,Bloom,Bloom);
		Glow = mix(m_glow_low*Glow, m_glow_high*Bloom, pow(colmx, m_glow_dist/gamma_in));
	}
	
	if (m_glow < 0.5) {
		if (glow >= 0.0) color = color + 0.5*Glow*glow; else color = color + abs(glow)*min(cmask2*cmask2,1.0)*Glow; }
	else { vec3 cmaskg = clamp(mix(one, cmask1, m_glow_mask),0.0, 1.0); color = color + abs(glow)*cmaskg*Glow; }

	color = min(color, 1.0);

	if (edgemask > 0.05) {
		mx0 = COMPAT_TEXTURE(Source, pos1 - dx).a; mx0 = COMPAT_TEXTURE(Source, pos1 - dx*(1.0-0.75*sqrt(mx0))).a; 
		mx2 = COMPAT_TEXTURE(Source, pos1 + dx).a; mx2 = COMPAT_TEXTURE(Source, pos1 + dx*(1.0-0.75*sqrt(mx2))).a; 
		float mx3 = COMPAT_TEXTURE(Source, pos1 - 4.0*dx).a;
		float mx4 = COMPAT_TEXTURE(Source, pos1 + 4.0*dx).a;
		mx4 = max(pow(abs(mx3-mx4),0.55-0.40*cx),min(max(mx3,mx4)/min(0.1+cx,1.0),1.0));
		mb = (1.0 - abs(pow(mx0,1.0-0.65*mx2)-pow(mx2,1.0-0.65*mx0)));
		mb = mx4*edgemask*(1.0001-mb*mb);
		vec3 temp = mix(color, orig1, mb);
		color = max(temp + mix(3.5*mb*mix(1.625*temp,temp,cx), vec3(0.0), pow(color, vec3(0.75)-0.5*colmx)),color); }
	
	color = color * mix(1.0, mix(0.5*(1.0+w3), w3, mx), pr_scan);  // preserve scanlines
	
	color = min(color, max(orig1,color)*mix(one, cmask1, mclip));  // preserve mask
	
	color = pow(color, vec3(1.0/gamma_out));

	float rc = 0.6*sqrt(max(max(color.r, color.g), color.b))+0.4;
	
	if (abs(addnoised) > 0.01) 
	{
		vec3 noise0 = noise(vec3(floor(OutputSize.xy * vTexCoord / noiseresd), float(FrameCount)));
		if (noisetype < 0.5) color = mix(color, noise0, 0.25*abs(addnoised) * rc); 
		else color = min(color * mix(1.0, 1.5*noise0.x, 0.5*abs(addnoised)), 1.0);
	}

	colmx = max(max(orig1.r,orig1.g),orig1.b);
	color = color + bmask*mix(cmask2, 0.125*(1.0-colmx)*color, min(20.0*colmx, 1.0));
	
	FragColor = mix(vec4(color*vig*humbar(mix(pos.y, pos.x, bardir))*post_br*corner(posb), 1.0), 
	            COMPAT_TEXTURE(StockPass, (floor(pos1*OriginalSize.xy)+0.5)*OriginalSize.zw), step(pos1.x,oimage-0.00025));
}

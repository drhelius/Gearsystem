// Gearsystem port of crt-guest-advanced-hd-pass2.slang from libretro slang-shaders (crt-guest-advanced-hd 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

/*
   CRT - Guest - Advanced - HD - Pass2
   
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
uniform float gsl;
uniform float scanline1;
uniform float scanline2;
uniform float beam_min;
uniform float beam_max;
uniform float beam_size;
uniform float cbulge;
uniform float glow;
uniform float inters;
uniform float bloom;
uniform float halation;
uniform float scans;
uniform float gamma_c;
uniform float gamma_c2;
uniform float no_scanlines;
uniform float MAXS;
uniform float tds;
uniform float clips;
uniform float hiscan;
uniform float SIGMA_VER;
uniform float VSHARPNESS;
uniform float HARNG;
uniform float S_SHARP;
uniform float rolling_scan;
uniform vec4 SourceSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float warpX;
uniform float warpY;
uniform float csize;
uniform float bsize1;
uniform float intres;
uniform float c_shape;
uniform float barspeed;
uniform float barintensity;
uniform float bardir;
uniform float internal_res;
uniform float scangamma;
uniform float sborder;
uniform float scan_falloff;
uniform float overscanX;
uniform float overscanY;
uniform float VShift;
uniform float bloom_dist;
uniform float bmask1;
uniform float hmask1;
uniform float interm;
uniform float ssharp;
uniform float bloomsamp;

float SIGMA_VER_ = SIGMA_VER * VSHARPNESS;
#define scans        1.5*scans     // scanline saturation
#define internal_res internal_res * (1.0 / (1.0 + hiscan))

#define COMPAT_TEXTURE(c,d) texture(c,d)
#define TEX0 vTexCoord
#define gl_FragCoord (vTexCoord * OutputSize.xy)

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D Pass1;
uniform sampler2D LinearizePass;
uniform sampler2D BloomPass;
uniform sampler2D PrePass;

#define eps 1e-10 

float wts = (bloomsamp < 0.5) ? -10.0 : -12.5;

float st(float x)
{
	return exp2(wts*x*x);
} 

vec3 sw0(float x, float color, float scanline, vec3 c)
{
	float tmp = mix(beam_min, beam_max, color);
	vec3 sat = mix(vec3(1.0) + scans, vec3(1.0), c);
	float ex = x*tmp;
	ex = (gsl > -0.5) ? ex*ex : mix(ex*ex, ex*ex*ex, 0.4);
	return exp2(-scanline*ex*sat);
} 

vec3 sw1(float x, float color, float scanline, vec3 c)
{	
	x = mix (x, beam_min*x, max(x-0.4*color,0.0));
	vec3 sat = mix(vec3(1.0) + scans, vec3(1.0), c);	
	float tmp = mix(1.2*beam_min, beam_max, color);
	float ex = x*tmp;
	return exp2(-scanline*ex*ex*sat);
}    

vec3 sw2(float x, float color, float scanline, vec3 c)
{
	float tmp = mix((2.5-0.5*color)*beam_min, beam_max, color);
	vec3 sat = mix(vec3(1.0) + scans, vec3(1.0), c);	
	tmp = mix(beam_max, tmp, pow(x, color+0.3));
	float ex = x*tmp;
	return exp2(-scanline*ex*ex*sat);
} 

vec3 gc(vec3 c)
{
	float mc = max(max(c.r,c.g),c.b);
	float mg = pow(mc, 1.0/gamma_c);
	return c * mg/(mc + eps);  
}

vec2 Overscan(vec2 pos, float dx, float dy){
	pos=pos*2.0-1.0;    
	pos*=vec2(dx,dy);
	return pos*0.5+0.5;
}  

vec2 Warp(vec2 pos)
{
	pos  = pos*2.0-1.0;
	pos = mix(pos, sign(pos)*pow(abs(pos), 1.0 + min(15.0*vec2(warpY,warpX)*vec2(cbulge), vec2(cbulge))), 0.075 + 0.125*(abs(pos)+pos*pos));
	pos = mix(pos, vec2(pos.x*inversesqrt(1.0-c_shape*pos.y*pos.y), pos.y*inversesqrt(1.0-c_shape*pos.x*pos.x)), vec2(warpX, warpY)/c_shape);
	return pos*0.5 + 0.5;
}

float invsqrsigma = 1.0/(2.0*SIGMA_VER_*SIGMA_VER_*internal_res*internal_res);

float gaussian(float x)
{
	return exp(-x*x*invsqrsigma);
} 

vec3 v_resample (vec2 tex0, vec4 Size) {

	float f = fract(Size.y * tex0.y);
	f = 0.5 - f;
	vec2 tex = tex0;
	tex.y = floor(Size.y * tex.y)*Size.w + 0.5*Size.w;
	vec3 color = vec3(0.0);
	vec2 dy  = vec2(0.0, Size.w);

	float w = 0.0;
	float wsum = 0.0;
	vec3 pixel;

	vec3 cmax = vec3(0.0);
	vec3 cmin = vec3(1.0);
	float vsharpness = max(VSHARPNESS*internal_res, 0.6);
	float sharp = gaussian(vsharpness) * S_SHARP;
	float ksharp = -2.0*vsharpness*invsqrsigma*sharp;
	float maxsharp = MAXS;
	float FPR = vsharpness;
	float FPRi = 1.0/vsharpness;
	float fpx = 0.0;

	float LOOPSIZE = ceil(2.0*FPR);
	
	float n = -LOOPSIZE;
	
	do
	{
		pixel  = COMPAT_TEXTURE(Pass1, tex + n*dy).rgb;

		w = gaussian(n+f) - sharp;
		fpx = clamp((abs(n+f)-FPR)*FPRi, 0.0, 1.0);
		if (w < 0.0) w = max(ksharp*fpx, maxsharp * ksharp * (1.0-fpx*fpx));
		else { cmax = max(cmax, pixel); cmin = min(cmin, pixel); }

		color = color + w * pixel;
		wsum  = wsum + w;

		n = n + 1.0;
			
	} while (n <= LOOPSIZE);

	color = color / wsum;

	color = clamp(mix(clamp(color, cmin, cmax), color, HARNG), 0.0, 1.0);

	return color; 
}

vec3 plant (vec3 tar, float r)
{
	float t = max(max(tar.r,tar.g),tar.b) + 0.00001;
	return tar * r / t;
}

void main()
{
	vec2 prescalex = vec2(textureSize(LinearizePass, 0)) / OriginalSize.xy;
	
	vec4 SourceSize_ = vec4(SourceSize.x, OriginalSize.y, SourceSize.z, OriginalSize.w);
	float gamma_in = 1.0/COMPAT_TEXTURE(LinearizePass, vec2(0.25,0.25)).a;
	float intera = COMPAT_TEXTURE(LinearizePass, vec2(0.75,0.25)).a;
	bool hscan = (hiscan > 0.5);
	bool inter6 = (intera < 0.35 && interm == 5.0);
	bool interb  = (((intera < 0.35 && !inter6) || (no_scanlines > 0.025)) && !hscan);
	bool vgascan = ((abs(intera-0.5) < 0.05) && (no_scanlines == 0.0));
	
	float SourceY = SourceSize_.y;
	float sy = 1.0;
	if (intres == 1.0) sy = max(round(SourceY/224.0),1.0);
	if (intres > 0.25 && intres != 1.0) sy = intres;
	if (inter6) sy*=2.0;
	
	// VGA Scan Mode
	if (vgascan) sy = 0.5; else if (abs(intera-0.75) < 0.05) sy = 1.0;	
	
	SourceSize_*=vec4(1.0, 1.0/sy, 1.0, sy);

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
	
	vec2 pos  = Warp(texcoord);

	float ii = float(inter6)*(floor(mod(float(FrameCount), 2.0)));

	float coffset = 0.5 + 0.5*ii;
	
	vec2 ps = SourceSize_.zw;
	float OGL2Pos = pos.y * SourceSize_.y - coffset;
	float f = fract(OGL2Pos);
	
	vec2 dx = vec2(ps.x,0.0);
	vec2 dy = vec2(0.0, ps.y);
   
	// Reading the texels

	vec2 pC4;
	
	pC4.y = floor(OGL2Pos) * ps.y + 0.5*ps.y; pC4.y = pC4.y + float(inter6)*0.25*dy.y;
	pC4.x = pos.x;

	if ((intres == 0.5 && prescalex.y < 1.5) || vgascan) pC4.y = floor(pC4.y * OriginalSize.y)*OriginalSize.w + 0.5*OriginalSize.w;
	if (interb && no_scanlines > 0.025) pC4.y = pC4.y + smoothstep(0.40-0.5*no_scanlines, 0.60 + 0.5*no_scanlines, f)*SourceSize_.w;
	
	vec3 color1 = COMPAT_TEXTURE(Pass1, pC4      ).rgb;
	vec3 scolor1 = 	COMPAT_TEXTURE(Pass1, pC4      ).aaa;

	float prescaley = float(textureSize(LinearizePass, 0).y) / OriginalSize.y;	
	
	if (interb && no_scanlines < 0.05 || hscan && vgascan || hscan) color1 = v_resample(pos, SourceSize_ * vec4(1.0, prescaley, 1.0, 1.0/prescaley));
	
	color1 = pow(color1, vec3(scangamma/gamma_in));
	
	pC4+=dy;

	if ((intres == 0.5 && prescalex.y < 1.5) || vgascan) pC4.y = floor((pos.y + 0.33*dy.y) * OriginalSize.y)*OriginalSize.w + 0.5*OriginalSize.w;
	
	vec3 color2 = COMPAT_TEXTURE(Pass1, pC4      ).rgb;
	vec3 scolor2 = 	COMPAT_TEXTURE(Pass1, pC4      ).aaa;
	
	color2 = pow(color2, vec3(scangamma/gamma_in));
	
	// calculating scanlines

	vec3 ctmp = color1; float w3 = 1.0; vec3 color = color1;
	vec3 one = vec3(1.0);

	if (hscan) { color2 = color1; scolor2 = scolor1; }

if (!interb || hscan)
{	
	float ssub = ssharp*abs(scolor1.x-scolor2.x);

	float shape1 = mix(scanline1, scanline2 + ssub * scolor1.x * 35.0, f);
	float shape2 = mix(scanline1, scanline2 + ssub * scolor2.x * 35.0, 1.0-f);
	
	float wt1 = st(f);
	float wt2 = st(1.0-f);

	vec3 color00 = color1*wt1 + color2*wt2;
	vec3 scolor0 = scolor1*wt1 + scolor2*wt2;
	
	ctmp = color00/(wt1+wt2);
	vec3 sctmp = max(scolor0/(wt1+wt2), ctmp);
	
	vec3 w1, w2;

	if (abs(rolling_scan) > 0.005) 
	{ 
		color1 = ctmp; color2 = ctmp;
		scolor1 = sctmp; scolor2 = sctmp;
	}
	
	vec3 cref1 = mix(sctmp, scolor1, beam_size); float creff1 = pow(max(max(cref1.r,cref1.g),cref1.b), scan_falloff);
	vec3 cref2 = mix(sctmp, scolor2, beam_size); float creff2 = pow(max(max(cref2.r,cref2.g),cref2.b), scan_falloff);

	if (tds > 0.5) { shape1 = mix(scanline2, shape1, creff1); shape2 = mix(scanline2, shape2, creff2); }

	float scanpix = OriginalSize.y/OutputSize.y;
	float f1 = fract(f - rolling_scan*float(FrameCount)*scanpix);
	float f2 = 1.0 - f1;

	float mc1 = max(max(color1.r,color1.g),color1.b) + eps;
	float mc2 = max(max(color2.r,color2.g),color2.b) + eps;
	
	cref1 = color1 / mc1;
	cref2 = color2 / mc2;

	if (gsl <  0.5) { w1 = sw0(f1,creff1,shape1,cref1); w2 = sw0(f2,creff2,shape2,cref2);} else
	if (gsl == 1.0) { w1 = sw1(f1,creff1,shape1,cref1); w2 = sw1(f2,creff2,shape2,cref2);} else
	                { w1 = sw2(f1,creff1,shape1,cref1); w2 = sw2(f2,creff2,shape2,cref2);}

	vec3 w3 = w1+w2;
	float wf1 = max(max(w3.r,w3.g),w3.b);
	if (wf1 > 1.0) {wf1 = 1.0/wf1; w1*=wf1, w2*=wf1;}

	if (abs(clips) > 0.005)
	{
		sy = mc1; one = (clips > 0.0) ? w1 : vec3(1.0);
		float sat = 1.0001-min(min(cref1.r,cref1.g),cref1.b);
		color1 = mix(color1, plant(pow(color1, vec3(0.70)-0.325*sat),sy), pow(sat,0.3333)*one*abs(clips));
		sy = mc2; one = (clips > 0.0) ? w2 : vec3(1.0);
		sat =       1.0001-min(min(cref2.r,cref2.g),cref2.b);
		color2 = mix(color2, plant(pow(color2, vec3(0.70)-0.325*sat),sy), pow(sat,0.3333)*one*abs(clips));
	}	
	
	color = (gc(color1)*w1 + gc(color2)*w2);

	if (bloomsamp > 0.5) {	
		vec3 Bloom = COMPAT_TEXTURE(BloomPass, pos).rgb;
		Bloom = pow(Bloom, vec3(scangamma/gamma_in));
		float cm = max(max(ctmp.r,ctmp.g),ctmp.b);
		float bm = max(max(Bloom.r,Bloom.g),Bloom.b);
		float bmix = clamp(max((cm-bm), (bm-cm)/(cm+0.25)-cm), 0.0, 1.0);
		if (abs(clips) > 0.005)
		{
			sy = cm + eps; one = (clips > 0.0) ? min(w1+w2,1.0) : vec3(1.0);
			cref1 = plant(Bloom,1.0);
			float sat = 1.0001-min(min(cref1.r,cref1.g),cref1.b);
			Bloom = mix(Bloom, plant(pow(Bloom, vec3(0.70)-0.325*sat),sy), pow(sat,0.3333)*one*abs(clips));
		}
		vec3 Bloom1 = (color1*wt1 + color2*wt2)/(wt1+wt2);
		Bloom = mix(Bloom, Bloom1, min(2.5*abs(plant(sqrt(Bloom1),1.0) - plant(sqrt(Bloom),1.0)), 1.0));
		bm = max(max(Bloom.r,Bloom.g),Bloom.b);
		Bloom = plant(mix(Bloom,Bloom1,bloomsamp-1.0), mix(bm, 1.125*cm, bmix));	
		Bloom = gc(Bloom)*(w1+w2);
		color = min(mix(color, Bloom, pow(cm, mix(0.4, 0.2, pow(cm,0.375)))), max(Bloom,color));
	}

	color = min(color, 1.0);
}
	
	if (interb) 
	{
		color = gc(color1);
	}

	float colmx = max(max(ctmp.r,ctmp.g),ctmp.b);

	color = pow(color, vec3(gamma_in/scangamma)); colmx = pow(colmx, gamma_in/scangamma);
  
	FragColor = vec4(color, colmx);
}

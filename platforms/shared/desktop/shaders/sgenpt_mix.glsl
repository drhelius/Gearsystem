// Gearsystem port of sgenpt-mix.slang from libretro slang-shaders.
// Original shader licensed MIT.

/*
   SGENPT-MIX - Sega Genesis Pseudo Transparency Mixer Shader - v10
   
   2011-2024 Hyllian - sergiogdb@gmail.com

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is 
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.

*/

uniform vec4 SourceSize;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform int FrameCount;
uniform float SGPT_BLEND_OPTION;
uniform float SGPT_BLEND_LEVEL;
uniform float SGPT_ADJUST_VIEW;
uniform float SGPT_LINEAR_GAMMA;

#define GAMMA_EXP		(SGPT_LINEAR_GAMMA+1.0)
#define GAMMA_IN(color)		pow(color, vec3(GAMMA_EXP, GAMMA_EXP, GAMMA_EXP))
#define GAMMA_OUT(color)	pow(color, vec3(1.0 / GAMMA_EXP, 1.0 / GAMMA_EXP, 1.0 / GAMMA_EXP))

//const vec3 Y = vec3(.2126, .7152, .0722);
const vec3 Y = vec3( 0.299,  0.587,  0.114);

vec3 min_s(vec3 central, vec3 adj1, vec3 adj2) {return min(central, max(adj1, adj2));}
vec3 max_s(vec3 central, vec3 adj1, vec3 adj2) {return max(central, min(adj1, adj2));}

//vec3 min_s(vec3 central, vec3 adj1, vec3 adj2) {return clamp(central, adj1, adj2);}

in vec2 vTexCoord;
out vec4 FragColor;
uniform sampler2D Source;

void main()
{
	vec2 dx = vec2(1.0, 0.0)*SourceSize.zw;
	vec2 dy = vec2(0.0, 1.0)*SourceSize.zw;

	// Reading the texels.
	vec3 C = GAMMA_IN(texture(Source, vTexCoord    ).xyz);
	vec3 L = GAMMA_IN(texture(Source, vTexCoord -dx).xyz);
	vec3 R = GAMMA_IN(texture(Source, vTexCoord +dx).xyz);

	//  Get min/max samples
	vec3 min_sample = min_s(C, L, R);
	vec3 max_sample = max_s(C, L, R);

	float contrast = dot(max(C, max(L, R)) - min(C, min(L, R)), Y);

	contrast = smoothstep(0.0, 1.0, (1.0 - SGPT_BLEND_LEVEL) * contrast);

	if (SGPT_BLEND_OPTION == 2) // Only Vertical Lines
	{
		vec3 UL = GAMMA_IN(texture(Source, vTexCoord -dx -dy).xyz);
		vec3 UR = GAMMA_IN(texture(Source, vTexCoord +dx -dy).xyz);
		vec3 DL = GAMMA_IN(texture(Source, vTexCoord -dx +dy).xyz);
		vec3 DR = GAMMA_IN(texture(Source, vTexCoord +dx +dy).xyz);

		min_sample = max_s(min_sample, min_s(C, DL, DR), min_s(C, UL, UR));
		max_sample = min_s(max_sample, max_s(C, DL, DR), max_s(C, UL, UR));
	}
	else if (SGPT_BLEND_OPTION == 3) // Only Checkerboard
	{
		vec3 U = GAMMA_IN(texture(Source, vTexCoord -dy).xyz);
		vec3 D = GAMMA_IN(texture(Source, vTexCoord +dy).xyz);

		min_sample = max(min_sample, min_s(C, U, D));
		max_sample = min(max_sample, max_s(C, U, D));
	}

	vec3 col_L = 0.5*( C + L + contrast*( C - L ));
	vec3 col_R = 0.5*( C + R + contrast*( C - R ));

	float contrast_L = dot(abs(C - col_L), Y);
	float contrast_R = dot(abs(C - col_R), Y);

	// Choose smaller contrast
	vec3 color = contrast_R < contrast_L ? col_R : col_L;

	color = SGPT_BLEND_OPTION > 0.5 ? clamp(color, min_sample, max_sample) : C;

	color = SGPT_ADJUST_VIEW > 0.5 ? vec3(dot(abs(C-color), vec3(1.0, 1.0, 1.0))) : color;

	FragColor = vec4(GAMMA_OUT(color), 1.0);
}

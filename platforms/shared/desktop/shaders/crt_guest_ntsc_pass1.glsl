// Gearsystem port of ntsc-pass1.slang from libretro slang-shaders (crt-guest-advanced-ntsc 2026-07-23-r1).
// Original shader licensed GPL v2 or later.

uniform vec4 OutputSize;
uniform vec4 OriginalSize;
uniform vec4 SourceSize;
uniform int FrameCount;
uniform float quality;
uniform float ntsc_sat;
uniform float cust_fringing;
uniform float cust_artifacting;
uniform float ntsc_bright;
uniform float ntsc_scale;
uniform float ntsc_fields;
uniform float ntsc_phase;
uniform float ntsc_gamma;
uniform float ntsc_rainbow1;
uniform float ntsc_taps;
uniform float auto_res;
uniform float ntsc_charp;
uniform float ntsc_charp3;
uniform float speedup;

// NTSC-Adaptive Pass1
// based on Themaister's NTSC shader

#pragma parameter ntsc-row0 "------------------------------------------------" 0.0 0.0 0.0 1.0
#pragma parameter ntsc-row1 "------------------------------------------------" 0.0 0.0 0.0 1.0
#pragma parameter ntsc-row2 "------------------------------------------------" 0.0 0.0 0.0 1.0
#pragma parameter ntsc-row3 "------------------------------------------------" 0.0 0.0 0.0 1.0

float FrameCount_ = float(FrameCount);
float auto_rez = mix(1.0, 0.5, clamp(auto_res * round(OriginalSize.x/300.0)-1.0, 0.0, 1.0));
#define ntsc_scale ntsc_scale * auto_rez
#define PI 3.14159265

in vec2 vTexCoord;
vec2 tex_coord;
vec2 pix_no;
float phase;
float BRIGHTNESS;
float SATURATION;
float FRINGING;
float ARTIFACTING;
float CHROMA_MOD_FREQ;
float MERGE;
out vec4 FragColor;
uniform sampler2D Source;

mat3 mix_mat;

const mat3 yiq2rgb_mat = mat3(
   1.0, 0.956, 0.6210,
   1.0, -0.2720, -0.6474,
   1.0, -1.1060, 1.7046);

vec3 yiq2rgb(vec3 yiq)
{
   return yiq * yiq2rgb_mat;
}

const mat3 yiq_mat = mat3(
      0.2989, 0.5870, 0.1140,
      0.5959, -0.2744, -0.3216,
      0.2115, -0.5229, 0.3114
);

vec3 rgb2yiq(vec3 col)
{
   return col * yiq_mat;
}

float get_luma(vec3 c)
{
	return dot(c, vec3(0.2989, 0.5870, 0.1140));
}

float smothstep (float e0, float e1, float x)
{
   return clamp((x - e0) / (e1 - e0), 0.0, 1.0);
}

void main()
{
   tex_coord = vTexCoord * vec2(speedup, 1.0);
   float vres = min(ntsc_scale, 1.0);
   float vsize = OriginalSize.x * auto_rez;
   pix_no = tex_coord * OriginalSize.xy * vec2(vres, vres / auto_rez) * vec2(4.0, 1.0);
   phase = (ntsc_phase < 1.5) ? ((vsize > 300.0) ? 2.0 : 3.0) : ((ntsc_phase > 2.5) ? 3.0 : 2.0);
   if (ntsc_phase == 4.0) phase = 3.0; else if (ntsc_phase == 5.0) phase = 2.0;
   CHROMA_MOD_FREQ = (phase == 2.0 && ntsc_phase != 5.0) ? (4.0 * PI / 15.0) : ((vsize <= 300.0 || phase == 3.0) ? (PI / 3.0) : (PI / 2.0));
   ARTIFACTING = cust_artifacting;
   FRINGING = cust_fringing;
   SATURATION = ntsc_sat;
   BRIGHTNESS = ntsc_bright;
   MERGE = 0.0;
   if (ntsc_fields == -1.0 && phase == 3.0) MERGE = 1.0;
   else if (ntsc_fields == 0.0) MERGE = 0.0;
   else if (ntsc_fields == 1.0) MERGE = 1.0;
   mix_mat = mat3(BRIGHTNESS, FRINGING, FRINGING, ARTIFACTING, 2.0 * SATURATION, 0.0, ARTIFACTING, 0.0, 2.0 * SATURATION);

if (tex_coord.x > 1.0) FragColor = vec4(0.0); else
{

   float res = ntsc_scale;
   float mit = ntsc_taps; if (ntsc_charp > 0.25 && phase == 2.0) mit = clamp(mit, 8.0, min(ntsc_taps,14.0));
   mit = smothstep(16.0, 8.0, mit) * 0.325;
   
   vec3 col = texture(Source, tex_coord).rgb;
 
   vec3 yiq = rgb2yiq(col);
   yiq.x = pow(yiq.x, ntsc_gamma);
   float lum = yiq.x; 

   vec3 tmp =  yiq; 
   vec3 yiq2 = yiq;	
   vec3 yiqs = yiq;
   vec3 yiqz = yiq;

if (MERGE > 0.5)
{
   float chroma_phase2 = (phase < 2.5) ? PI * (mod(pix_no.y, 2.0) + mod(FrameCount_+1.0, 2.0)) : 0.6667 * PI * (mod(pix_no.y, 3.0) + mod(FrameCount_+1.0, 2.0));
   float mod_phase2 = chroma_phase2 + pix_no.x * CHROMA_MOD_FREQ;
   float i_mod2 = cos(mod_phase2);
   float q_mod2 = sin(mod_phase2);
   yiq2.yz *= vec2(i_mod2, q_mod2); // Modulate.
   yiq2 *= mix_mat; // Cross-talk.
   yiq2.yz *= vec2(i_mod2, q_mod2); // Demodulate.
   yiq2.yz = mix(yiq2.yz, tmp.yz, mit);   

   if (res > 1.025)
   {
      mod_phase2 = chroma_phase2 + pix_no.x * CHROMA_MOD_FREQ * res;
      i_mod2 = cos(mod_phase2);
      q_mod2 = sin(mod_phase2);
      yiqs.yz *= vec2(i_mod2, q_mod2); // Modulate.
      yiq2.x = dot(yiqs, mix_mat[0]);  // Cross-talk.
   }
}
   
   float chroma_phase = (phase < 2.5) ? PI * (mod(pix_no.y, 2.0) + mod(FrameCount_, 2.0)) : 0.6667 * PI * (mod(pix_no.y, 3.0) + mod(FrameCount_, 2.0));
   float mod_phase = chroma_phase + pix_no.x * CHROMA_MOD_FREQ;
   float i_mod = cos(mod_phase);
   float q_mod = sin(mod_phase);
   yiq.yz *= vec2(i_mod, q_mod); // Modulate.
   yiq *= mix_mat; // Cross-talk.
   yiq.yz *= vec2(i_mod, q_mod); // Demodulate.
   yiq.yz = mix(yiq.yz, tmp.yz, mit);
   
    if (res > 1.025)
   {
      mod_phase = chroma_phase + pix_no.x * CHROMA_MOD_FREQ * res;
      i_mod = cos(mod_phase);
      q_mod = sin(mod_phase);
      yiqz.yz *= vec2(i_mod, q_mod); // Modulate.
      yiq.x = dot(yiqz, mix_mat[0]); // Cross-talk.
   }
      
   if (ntsc_phase == 4.0)
   {
      yiq.x = lum; yiq2.x = lum;
   }

   if (MERGE > 0.5) { if (ntsc_rainbow1 < 0.5 || phase > 2.5) yiq = 0.5*(yiq + yiq2); else yiq.x = 0.5*(yiq.x + yiq2.x); }
   
   FragColor = vec4(yiq, lum);
}

}

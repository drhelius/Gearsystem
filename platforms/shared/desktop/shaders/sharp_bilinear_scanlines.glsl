// Author: rsn8887 (based on TheMaister)
// License: Public domain
// Source: https://github.com/rsn8887/Sharp-Bilinear-Shaders
// Ported to Gearsystem GLSL preset uniforms.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform vec4 OriginalSize;
uniform float SourceLines;
uniform float ScanlineBaseBrightness;
uniform float ScanlineHorizontalModulation;
uniform float ScanlineVerticalModulation;

const float PI = 3.141592654;

vec2 sharp_bilinear_coord()
{
    vec2 texel = vTexCoord * SourceSize.xy;
    vec2 scale = max(floor(OutputSize.xy / SourceSize.xy), vec2(1.0));
    vec2 texel_floored = floor(texel);
    vec2 center_dist = fract(texel) - 0.5;
    vec2 region_range = 0.5 - 0.5 / scale;
    vec2 offset = (center_dist - clamp(center_dist, -region_range, region_range)) * scale + 0.5;

    return (texel_floored + offset) * SourceSize.zw;
}

void main()
{
    vec3 color = texture(Source, sharp_bilinear_coord()).rgb;
    float lines = (SourceLines > 0.0) ? SourceLines * SourceSize.y * OriginalSize.w : SourceSize.y;
    vec2 omega = vec2(PI * OutputSize.x, 2.0 * PI * lines);
    vec2 modulation = vec2(ScanlineHorizontalModulation, ScanlineVerticalModulation);
    float intensity = ScanlineBaseBrightness + dot(modulation * sin(vTexCoord * omega), vec2(1.0));

    FragColor = vec4(color * intensity, 1.0);
}

// Geargrafx port of crt-consumer from libretro slang-shaders.
// Original shader licensed GPL v2 or later.

in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 SourceSize;
uniform vec4 OutputSize;
uniform vec4 BackgroundColor;
uniform int FrameCount;
uniform float PRE_SCALE;
uniform float blurx;
uniform float blury;
uniform float warpx;
uniform float warpy;
uniform float corner;
uniform float smoothness;
uniform float inter = 1.0;
uniform float Downscale = 2.0;
uniform float scanlow;
uniform float scanhigh;
uniform float beamlow;
uniform float beamhigh;
uniform float preserve;
uniform float brightboost1;
uniform float brightboost2;
uniform float glow;
uniform float quality;
uniform float glow_str;
uniform float nois;
uniform float postbr;
uniform float GAMMA_OUT;
uniform float sat;
uniform float contrast;
uniform float WP;
uniform float vignette;
uniform float vpower;
uniform float vstr;

const float MaskDark = 0.2;
const float masksize = 1.0;

const mat3 D65_to_XYZ = mat3(
    0.4306190,  0.2220379,  0.0201853,
    0.3415419,  0.7066384,  0.1295504,
    0.1783091,  0.0713236,  0.9390944);

const mat3 XYZ_to_D65 = mat3(
     3.0628971, -0.9692660,  0.0678775,
    -1.3931791,  1.8760108, -0.2288548,
    -0.4757517,  0.0415560,  1.0693490);

const mat3 D50_to_XYZ = mat3(
    0.4552773,  0.2323025,  0.0145457,
    0.3675500,  0.7077956,  0.1049154,
    0.1413926,  0.0599019,  0.7057489);

const mat3 XYZ_to_D50 = mat3(
     2.9603944, -0.9787684,  0.0844874,
    -1.4678519,  1.9161415, -0.2545973,
    -0.4685105,  0.0334540,  1.4216174);

vec2 warp_position(vec2 pos)
{
    pos = pos * 2.0 - 1.0;
    pos *= vec2(1.0 + pos.y * pos.y * warpx, 1.0 + pos.x * pos.x * warpy);
    return pos * 0.5 + 0.5;
}

bool outside_source(vec2 pos)
{
    return pos.x < 0.0 || pos.x > 1.0 || pos.y < 0.0 || pos.y > 1.0;
}

vec3 sample_source(vec2 pos)
{
    if (outside_source(pos))
        return BackgroundColor.rgb;

    return texture(Source, pos).rgb;
}

float scanline_weight(float y, float luminance)
{
    float beam = mix(scanlow, scanhigh, y);
    float scan = mix(beamlow, beamhigh, luminance);
    float ex = y * scan;
    return exp2(-beam * ex * ex);
}

vec3 default_mask(vec2 pos, float luminance)
{
    pos = floor(pos / masksize);

    float phase = fract(pos.x * 0.4999);
    vec3 mask = phase < 0.4999 ? vec3(1.0, MaskDark, 1.0) : vec3(MaskDark, 1.0, MaskDark);
    return mix(mask, vec3(1.0), luminance * preserve);
}

mat4 contrast_matrix(void)
{
    float t = (1.0 - contrast) * 0.5;

    return mat4(contrast, 0.0,      0.0,      0.0,
                0.0,      contrast, 0.0,      0.0,
                0.0,      0.0,      contrast, 0.0,
                t,        t,        t,        1.0);
}

float vignette_amount(void)
{
    if (vignette < 0.5)
        return 1.0;

    vec2 pos = vTexCoord * (1.0 - vTexCoord);
    float value = pos.x * pos.y * vstr;
    return min(pow(value, vpower), 1.0);
}

vec3 apply_saturation(vec3 color)
{
    float luminance = length(color) * 0.5775;
    vec3 weights = vec3(0.4, 0.5, 0.1);
    if (luminance < 0.5)
        weights = weights * weights + weights * weights;

    luminance = dot(color, weights);
    return mix(vec3(luminance), color, sat);
}

vec3 glow_sample(vec2 texcoord)
{
    vec2 size = SourceSize.zw / quality;
    vec3 sum = vec3(0.0);
    float factor = 1.0 / glow;

    for (float x = -glow; x <= glow; x += 1.0)
    {
        for (float y = -glow; y <= glow; y += 1.0)
        {
            vec3 color = sample_source(texcoord + vec2(x, y) * size) * factor;
            sum += color * color;
        }
    }

    return glow_str * sum / (glow * glow);
}

float noise(vec2 coord)
{
    float timer = float(FrameCount) / 60.0;
    return fract(sin(timer * dot(coord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float corner_mask(vec2 coord)
{
    coord = min(coord, vec2(1.0) - coord) * vec2(1.0, SourceSize.y / SourceSize.x);
    vec2 distance = vec2(corner) - min(coord, vec2(corner));
    return clamp((corner - sqrt(dot(distance, distance))) * smoothness, 0.0, 1.0);
}

vec3 apply_color_temperature(vec3 color)
{
    if (WP == 0.0)
        return color;

    vec3 warmer = XYZ_to_D65 * (D50_to_XYZ * color);
    vec3 cooler = XYZ_to_D50 * (D65_to_XYZ * color);
    float amount = abs(WP) / 100.0;
    vec3 target = WP < 0.0 ? cooler : warmer;
    return mix(color, clamp(target, 0.0, 1.0), amount);
}

void main()
{
    vec2 pos = warp_position(vTexCoord.xy);
    if (outside_source(pos))
    {
        FragColor = vec4(BackgroundColor.rgb, 1.0);
        return;
    }

    vec2 tex_size = SourceSize.xy;
    float interlace_downscale = Downscale > 0.0 ? Downscale : 2.0;
    vec2 pixel_phase = fract(pos * tex_size);
    if (inter < 0.5 && tex_size.y > 400.0)
        pixel_phase.y = fract(pos.y * tex_size.y / interlace_downscale);

    vec2 texel = pos * tex_size;
    vec2 texel_floored = floor(texel);
    float region_range = 0.5 - 0.5 / PRE_SCALE;
    vec2 center_dist = pixel_phase - 0.5;
    vec2 filtered_phase = (center_dist - clamp(center_dist, -region_range, region_range)) * PRE_SCALE + 0.5;
    vec2 coords = (texel_floored + filtered_phase) / SourceSize.xy;

    vec3 sample1 = sample_source(vec2(coords.x + blurx * SourceSize.z, coords.y - blury * SourceSize.w));
    vec3 sample2 = sample_source(coords);
    vec3 sample3 = sample_source(vec2(coords.x - blurx * SourceSize.z, coords.y + blury * SourceSize.w));

    vec3 color = vec3(sample1.r * 0.5 + sample2.r * 0.5,
                      sample1.g * 0.25 + sample2.g * 0.5 + sample3.g * 0.25,
                      sample2.b * 0.5 + sample3.b * 0.5);

    color = apply_color_temperature(color);
    color = 2.0 * pow(color, vec3(2.8)) - pow(color, vec3(3.6));

    float luminance = color.r * 0.3 + color.g * 0.6 + color.b * 0.1;
    float scan_pos = fract(pixel_phase.y - 0.5);
    if (inter <= 0.5 || tex_size.y <= 400.0)
        color = color * scanline_weight(scan_pos, luminance) + color * scanline_weight(1.0 - scan_pos, luminance);

    float masked_luminance = color.r * 0.3 + color.g * 0.6 + color.b * 0.1;
    color *= default_mask(vTexCoord * OutputSize.xy, masked_luminance);
    color *= mix(brightboost1, brightboost2, max(max(color.r, color.g), color.b));
    color = pow(max(color, vec3(0.0)), vec3(1.0 / GAMMA_OUT));

    if (glow_str != 0.0)
        color += glow_sample(coords);
    if (sat != 1.0)
        color = apply_saturation(color);
    if (corner != 0.0)
        color = mix(BackgroundColor.rgb, color, corner_mask(pos + 0.5 / tex_size));
    if (nois != 0.0)
        color *= 1.0 + noise(coords * 2.0) / nois;

    color *= mix(1.0, postbr, luminance);
    vec4 result = vec4(color, 1.0);
    if (contrast != 1.0)
        result = contrast_matrix() * result;
    if (inter > 0.5 && SourceSize.y > 400.0 && fract(float(FrameCount) / 2.0) < 0.5)
        result *= 0.95;

    result.rgb *= vignette_amount();
    FragColor = result;
}
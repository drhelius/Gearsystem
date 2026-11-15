in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform vec4 OriginalSize;
uniform vec4 OutputSize;
uniform float SourceLines;
uniform float ScanlineIntensity;

void main()
{
    vec4 color = texture(Source, vTexCoord);
    float lines = (SourceLines > 0.0) ? SourceLines : OriginalSize.y;
    float source_y = (gl_FragCoord.y / OutputSize.y) * lines;
    float mask = fract(source_y) >= 0.5 ? 1.0 : 0.0;
    color.rgb *= 1.0 - (ScanlineIntensity * mask);
    FragColor = color;
}
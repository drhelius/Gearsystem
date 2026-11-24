# Gearsystem Shaders

Gearsystem desktop shader presets use `.gshader` files. A preset is an INI-style file that describes one or more GLSL fragment shader passes. Presets placed in this `shaders` directory, up to 64, are discovered once when the desktop app starts and appear in the `Video > Shader` menu after `Pixel Perfect`. Shader presets are not applied while the debugger is enabled.

GLSL files are normal fragment shaders. Gearsystem supplies a fullscreen vertex shader and prepends the correct GLSL version, so shader files should not include a `#version` line.

## Minimal Preset

```ini
[Preset]
Name=Scanlines Bilinear
Passes=1

[Pass0]
Path=scanlines.glsl
ScaleType=Viewport
Filter=Linear

[Parameters]
ScanlineIntensity=0.10

[Parameter.ScanlineIntensity]
Label=Intensity
Min=0.0
Max=1.0
Step=0.01
```

## Preset Sections

`[Preset]`

- `Name`: display name shown in the menu. If omitted, the filename without its extension is used.
- `Group`: optional submenu name. Presets with the same group are listed together in that submenu.
- `Passes`: number of passes, from 1 to 32.
- `Variants`: optional number of `[VariantN]` sections, up to 32.
- `DefaultVariant`: variant applied when the preset is loaded for the first time. Defaults to `0`.

`[PassN]`

- `Path`: GLSL fragment shader path. Relative paths are resolved from the `.gshader` file directory first, then from the `shaders` resource directory.
- `Alias`: optional name for the pass output. Later passes can sample it with that name. See Shader Inputs.
- `Filter`: `Nearest` by default. Use `Linear` or `Bilinear` for bilinear sampling on that pass input/output texture.
- `Feedback`: `true` when the pass needs the previous frame through `PassFeedback0`.
- `History`: `true` when the pass needs previous inputs to the same pass through `SourceHistory0` through `SourceHistory7`.
- `ScaleType`: sets both axes. Defaults to `Viewport`. Values are:
  - `Source` (or `Original`): relative to the original emulator frame, at native core resolution.
  - `Previous`: relative to the previous pass output. This is RetroArch's `source` scale type.
  - `Viewport` (or `FinalViewport`): relative to the final display viewport.
  - `Absolute`: fixed size from `AbsoluteWidth` and `AbsoluteHeight`.
- `ScaleTypeX`, `ScaleTypeY`: optional per-axis scale type overrides.
- `Scale`: output scale for both axes. Defaults to `1.0`.
- `ScaleX`, `ScaleY`: optional per-axis scale overrides.
- `AbsoluteWidth`, `AbsoluteHeight`: used with `ScaleType=Absolute`.
- `FloatFramebuffer`: `true` when the pass output needs signed or high-range intermediate values. Uses a floating-point render target.

The final pass is what Gearsystem displays. Non-final passes render to intermediate textures. Only the final pass needs
`Viewport`; passes that work at core resolution should use `Source` or `Previous` to avoid rendering at display size.

`[Parameters]`

Each entry creates a float uniform with the same name:

```ini
[Parameters]
Amount=0.5
```

Optional metadata for sliders:

```ini
[Parameter.Amount]
Label=Amount
Min=0.0
Max=1.0
Step=0.01
```

`Label` defaults to the parameter name. `Min`, `Max` and `Step` default to `0.0`, `1.0` and `0.01`, and the `[Parameters]`
value is clamped to the `Min` to `Max` range.

A parameter with `Min=0.0`, `Max=1.0` and a `Step` of `1.0` or higher is displayed as a checkbox. Other parameters with
a `Step` of `1.0` or higher are displayed as integer sliders.

Optional `Group` puts the parameter in a collapsible section of the Shader Parameters window, which opens from
`Video > Shader > Parameters...`:

```ini
[Parameter.Amount]
Label=Amount
Group=Glow
```

A preset can expose up to 256 parameters.

`[VariantN]`

A variant is a named set of parameter values. Selecting it resets every parameter to its `[Parameters]` value and then
applies the variant values. Variants are listed as submenus of the preset in `Video > Shader` and in the Shader
Parameters window. `Restore Defaults` restores the selected variant.

```ini
[Preset]
Name=My CRT
Variants=2

[Variant0]
Name=Slot Mask

[Variant1]
Name=Aperture Grille
MaskType=2.0
```

## Shader Inputs

Available samplers:

- `uniform sampler2D Source`: current pass input.
- `uniform sampler2D Original`: original emulator frame.
- `uniform sampler2D PassFeedback0`: previous final output, for feedback presets.
- `uniform sampler2D SourceHistory0` through `SourceHistory7`: previous inputs to the same pass, newest first, for history presets.
- `uniform sampler2D PassOutputN`: output of any previous pass, where `PassOutput0` is pass 0 output, `PassOutput1` is pass 1 output, and so on.
- `uniform sampler2D <Alias>`: output of a previous pass with that `Alias`.
- `uniform sampler2D <Alias>Feedback`: previous frame output of the non-final pass with that `Alias`. A pass can sample its own feedback.

A pass can use up to 16 samplers.

Available size uniforms are `vec4(width, height, 1.0 / width, 1.0 / height)`:

- `SourceSize`: current pass input size.
- `OriginalSize`: original emulator frame size.
- `OutputSize`: current pass output size.
- `FinalViewportSize`: final display viewport size.
- `PassFeedback0Size`: feedback texture size.
- `SourceHistorySize`: source history texture size.
- `PassOutputSizeN` (or `PassOutputNSize`): previous pass output texture sizes.
- `<Alias>Size`: output size of the pass with that `Alias`.

Other uniforms:

- `FrameCount`: increments once per new emulated frame. It starts at `0` when a preset is loaded.
- `FrameDirection`: `1` normally, `-1` while rewinding.
- `SourceHistoryCount`: number of valid source history textures, from `0` to `8`.
- `OriginalAspect`: aspect ratio of the `Original` frame, `OriginalSize.x / OriginalSize.y`. For example `1.333` for a
  `256x192` frame and `1.111` for the `160x144` Game Gear screen.
- `FramebufferScale`: display pixel density, as the rounded ratio between framebuffer pixels and window points, at
  least `1.0`. Use it to keep masks and grids at a fixed physical size on high-density displays.
- `BackgroundColor`: configured normal desktop background color as `vec4(r, g, b, 1)`.

Master System, Game Gear and SG-1000 video uniforms, updated every frame:

- `float OriginalFPS`: emulated frame rate in Hz, about `59.92` at 60 Hz (NTSC), `49.70` at 50 Hz (PAL) and `50.17` on
  an SG-1000 at 50 Hz.
- `float PixelClock`: rate of the `Original` texels in MHz. Each texel is one VDP pixel, whatever `Video > Overscan` and
  `Video > Hide Left Bar` are set to: `5.369` at 60 Hz, `5.320` on a Master System at 50 Hz and `5.370` on an SG-1000
  at 50 Hz. `0.0` on Game Gear, including Game Gear in Master System mode, because its LCD has no video signal.
- `float LineRate`: horizontal scan rate in kHz: `15.700` at 60 Hz, `15.557` on a Master System at 50 Hz and `15.702`
  on an SG-1000 at 50 Hz. `0.0` on Game Gear.
- `float SourceLines`: number of scanlines in `Original`. It always equals `OriginalSize.y` here. Scanline shaders use
  it instead of `OriginalSize.y` so the same shader also works in emulators that double lines.
- `vec4 ChromaPhase`: color subcarrier timing, for composite shaders. It is available on the Master System, at both 60
  Hz and 50 Hz, and on an SG-1000 at 60 Hz. `x` is the subcarrier phase at the left edge of `Original` row `0`, in
  cycles from `0.0` to `1.0`, with phase `0.0` at the first picture pixel of the VDP. It is `0.0` without a left
  border or crop. A full width `Video > Overscan` (284 or 320) moves it to `2/3` at 60 Hz and `1/3` at 50 Hz, and a
  `Video > Hide Left Bar` crop of the first 8 pixels moves it to `1/3` at 60 Hz and `2/3` at 50 Hz. `y` is cycles per
  `Original` texel: `2/3` at 60 Hz and `5/6` at 50 Hz. `z` is cycles per row: `228` at 60 Hz and `285` at 50 Hz. It is
  a whole number, so the phase is the same on every row. `w` is `0.0`, because a frame lasts a whole number of cycles,
  so the artifact pattern stays still from frame to frame. The phase at `Original` position `(px, row)` is
  `fract(x + px * y + row * z)`. The subcarrier is `ChromaPhase.y * PixelClock`: `3.580` MHz for NTSC and `4.434` MHz
  for PAL, which tells the two apart. `vec4(0.0)` on Game Gear and on an SG-1000 at 50 Hz, whose video encoder is not
  part of the console.
- `vec4 ActiveRegion`: picture area inside `Original` as `vec4(x, y, width, height)` in texels. It leaves out the
  overscan border and the 8 pixel column at the left that the VDP blanks to the border color when bit 5 of its register
  0 is set, unless `Video > Hide Left Bar` already removed that column from `Original`. `x` is `0`, `14` or `32` for
  `Video > Overscan` set to `Disabled` or `Top+Bottom`, `Full (284 width)` and `Full (320 width)`, plus `8` when the
  column is blanked. `y` is `0` with `Overscan` `Disabled`, otherwise `24` at 60 Hz or `48` at 50 Hz, minus `16` in the
  224-line mode. `width` is `256`, or `248` without the left column. `height` is `192`, or `224` in the 224-line mode.
  On Game Gear it is the whole frame.
- `vec4 BorderColor`: the backdrop color selected by VDP register 7, as `vec4(r, g, b, 1)`. It is the color of the
  overscan border and of the blanked left column. It comes from the palette the core uses in the current video mode:
  entry `16 + (register 7 & 15)` of the VDP color RAM in Master System graphics, or entry `register 7 & 15` of the
  TMS9918 palette in the SG-1000 graphics modes. `vec4(0.0)` on Game Gear, where the border is never visible.
- `int SystemType`: `0` Master System, `1` Game Gear, `2` SG-1000, `3` Game Gear in Master System mode.

`FrameCount`, `SourceHistory`, `PassFeedback0` and `<Alias>Feedback` only advance when the emulator produces a new
frame, or when rewinding steps back one. Redraws while paused, after a window resize or after a parameter change render
the same frame again with the same state.

Any parameter declared in `[Parameters]` is also available as a `float` uniform.

## Minimal GLSL Shader

```glsl
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;

void main()
{
    FragColor = texture(Source, vTexCoord);
}
```

## Feedback Example

```ini
[Preset]
Name=Feedback Blend
Passes=1

[Pass0]
Path=feedback_blend.glsl
ScaleType=Viewport
Filter=Nearest
Feedback=true

[Parameters]
FeedbackAmount=0.5
```

A feedback shader can sample the previous final frame. Save this one as `feedback_blend.glsl` next to the preset:

```glsl
in vec2 vTexCoord;
out vec4 FragColor;

uniform sampler2D Source;
uniform sampler2D PassFeedback0;
uniform float FeedbackAmount;

void main()
{
    vec3 current = texture(Source, vTexCoord).rgb;
    vec3 previous = texture(PassFeedback0, vTexCoord).rgb;
    FragColor = vec4(mix(current, previous, FeedbackAmount), 1.0);
}
```

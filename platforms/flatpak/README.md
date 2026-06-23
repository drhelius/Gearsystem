# Flatpak Platform for Gearsystem

[Flatpak](https://flatpak.org/) packages Gearsystem with a consistent runtime
and a sandboxed desktop environment.

## Contents

- `io.github.drhelius.Gearsystem.yml`: Flatpak manifest for local builds and a
  starting point for Flathub submission.
- `io.github.drhelius.Gearsystem.desktop`: desktop launcher metadata.
- `io.github.drhelius.Gearsystem.metainfo.xml`: AppStream metadata.
- `io.github.drhelius.Gearsystem.xml`: MIME type declarations for supported ROM
  extensions.
- `icons/`: application icons.

## Runtime

The manifest uses `org.freedesktop.Platform` and `org.freedesktop.Sdk` version
`25.08`. Gearsystem is built against SDL3 through the SDK-provided `sdl3`
pkg-config package, matching the current Linux makefiles.

The package does not request network access by default. The optional MCP HTTP
transport can be enabled by the user with an override when needed:

```bash
flatpak override --user --share=network io.github.drhelius.Gearsystem
```

## Build Locally

Install Flatpak Builder:

```bash
flatpak install -y flathub org.flatpak.Builder
```

Build and install from this directory:

```bash
flatpak-builder --force-clean --user --install build-dir io.github.drhelius.Gearsystem.yml
```

Run Gearsystem:

```bash
flatpak run io.github.drhelius.Gearsystem
```

## Lint

Flathub recommends checking both the manifest and generated repository:

```bash
flatpak run --command=flatpak-builder-lint org.flatpak.Builder manifest io.github.drhelius.Gearsystem.yml
flatpak run --command=flatpak-builder-lint org.flatpak.Builder repo repo
```

## Sandbox Notes

Flatpak stores application data under `~/.var/app/io.github.drhelius.Gearsystem/`.
Inside the sandbox, `XDG_CONFIG_HOME`, `XDG_DATA_HOME` and `XDG_CACHE_HOME` point
to app-specific directories below that path.

For normal gameplay, keep save data, save states, screenshots, shaders and BIOS
files in Gearsystem's configured folders rather than depending on files adjacent
to a ROM outside the sandbox.
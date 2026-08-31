# Gearsystem iOS User Guide

## Supported games and controls

- Supported files: `.sms`, `.gg`, `.sg`, `.mv`, `.bin`, `.rom`, and `.zip`.
- Controls: directional pad, buttons 1 and 2, and Start.
- One connected Apple-compatible game controller can be used for Player 1.

## Install

The app requires iOS 17 or newer.

To install it from this repository:

1. Open the Xcode project in `platforms/ios` using Xcode 26 or newer.
2. Select the app scheme.
3. Select an iPhone, iPad, or Simulator.
4. Press **Run**.

When installing on a physical device, Xcode may ask you to select your Apple development team.

## Add games

You can add games in several ways:

- In Files, Mail, AirDrop, or another app, open or share a supported file with the emulator.
- Use Finder file sharing while the iPhone or iPad is connected to a Mac.
- Copy files into the emulator's folder in iCloud Drive when iCloud is enabled.

Keep games at the top level of the emulator's Documents folder. Subfolders are not scanned.

If files were copied while the app was open, go to **Settings → Refresh ROM Library**.

## Library

- Games are displayed in a multi-column cover grid.
- Use the alphabetical index on the right to move quickly through a large library.
- Long-press a game, or use its menu, to add or remove it from Favorites, share it, or delete it.
- Deleting a game also deletes its file from the app's Documents folder.

## Box art

The app can download box art automatically when online artwork is available. An internet
connection is required.

A locally built version or an unmatched game may show a placeholder. This does not prevent the
game from working.

## Play

- Tap a game to open full-screen gameplay.
- Portrait places the game screen above the controls.
- Landscape places the directional and action controls on opposite sides.
- Use the top-left button to close the game.
- Use the top-right button for reset, save state, load state, audio, and favorite actions.
- Gameplay pauses while the action menu is open.
- Save and Load State use the slot selected in Settings.

## Screen size and filtering

The **Screen Size** setting offers:

- **Fit to Width**: fills the device width in portrait with square source pixels. In landscape it
  uses the widest size that fits between the controls without cropping.
- **Integer Scaling**: uses the largest integer scale that fits on the device, producing evenly
  sized source pixels.

**Smooth Scaling** is independent from screen size:

- Disabled uses sharp nearest-neighbor filtering.
- Enabled uses smoother bilinear filtering.

## Settings

Settings include audio, haptic feedback, save-state slot, screen size, image filtering, and
options specific to the emulated system.

PSG and YM2413 FM volume can be balanced independently in Settings.

Some system and video changes take effect when the next game is opened. Close and reopen the
game after changing them.

## Languages

The app includes English, Spanish, German, French, Japanese, and Simplified Chinese.

iOS selects the language from the device or per-app language settings.
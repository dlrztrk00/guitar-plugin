# SoundBox — guitar multi-fx plugin

An audio plugin (AU + VST3) for guitar, built with [JUCE](https://juce.com).
It's the native port of my browser SoundBox: the same effect chain, running as a
real plugin you can load in Logic Pro, Ableton, Reaper and other DAWs.

## Signal chain

`gate → compressor → distortion (4× oversampled) → 3-band EQ → cabinet sim → chorus → delay → reverb → level → limiter`

- **Distortion** — soft / hard / fuzz curves, oversampled to tame aliasing, DC-blocked.
- **EQ** — low shelf, mid peak, high shelf.
- **Cab sim** — speaker-style high/low roll-off with a presence dip.
- **Time fx** — feedback delay, chorus, algorithmic reverb.
- **Dynamics** — noise gate, input compressor, output limiter.
- Every effect has its own footswitch (true bypass) and is fully automatable.

## UI

The panel is HTML/CSS/JS rendered in a WebView, two-way bound to the plugin
parameters via JUCE's relay system — an amp head + speaker cabinet on the left,
a pedalboard on the right. Artist presets (tone + faceplate) load from a dropdown.

## Build

Requires CMake and Xcode. JUCE is fetched automatically.

```bash
cmake -B build -G Xcode
cmake --build build --config Release
```

The AU/VST3 are copied into `~/Library/Audio/Plug-Ins/` on build.

## Validate the AU

```bash
auval -v aufx Sbx1 Dlra
```

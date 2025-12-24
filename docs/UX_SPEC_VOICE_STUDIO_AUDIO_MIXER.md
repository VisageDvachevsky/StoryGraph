# UX Specification: Voice Studio & Audio Mixer Panels

> **Document Version**: 1.0
> **Status**: Ready for Implementation
> **Created**: 2025-12-24
> **Issue Reference**: [#3](https://github.com/VisageDvachevsky/AudioMixer/issues/3)

## Table of Contents

1. [Executive Summary](#executive-summary)
2. [User Research](#user-research)
   - [User Personas](#user-personas)
   - [User Journeys](#user-journeys)
3. [Voice Studio Panel](#voice-studio-panel)
   - [Information Architecture](#voice-studio-information-architecture)
   - [Controls & Functionality](#voice-studio-controls--functionality)
   - [Visual Model](#voice-studio-visual-model)
   - [Text Wireframe](#voice-studio-text-wireframe)
4. [Audio Mixer Panel](#audio-mixer-panel)
   - [Information Architecture](#audio-mixer-information-architecture)
   - [Controls & Functionality](#audio-mixer-controls--functionality)
   - [Visual Model](#audio-mixer-visual-model)
   - [Text Wireframe](#audio-mixer-text-wireframe)
5. [Integration Points](#integration-points)
6. [Layout Alternatives Comparison](#layout-alternatives-comparison)
7. [Error & Empty States](#error--empty-states)
8. [Acceptance Criteria](#acceptance-criteria)

---

## Executive Summary

This document provides comprehensive UX specifications for two critical editor panels in NovelMind:

1. **Voice Studio** — A non-destructive voice recording and editing environment for voice-over content
2. **Audio Mixer** — A mixing console for previewing and balancing audio channels (music, voice, SFX, ambient)

### Design Principles

1. **Professional, not complex** — Clean UI that doesn't overwhelm but provides full control
2. **IMGUI-compatible** — All components achievable with Qt Widgets (no DAW-level graphics)
3. **Non-blocking** — All heavy operations (waveform analysis, encoding) run async
4. **Undoable** — Every edit supports Undo/Redo
5. **Integrated** — Seamless connection to Voice Manager, Asset Browser, and AudioManager
6. **Accessible** — Keyboard navigation, tooltips, and clear feedback for all actions

---

## User Research

### User Personas

#### Persona 1: Sound Designer (Konstantin)
- **Role**: Professional audio engineer working on VN projects
- **Goals**: Record high-quality voice lines, apply consistent processing, manage takes
- **Pain Points**: Slow workflows, lack of real-time monitoring, poor take management
- **Tech Comfort**: High — expects professional audio tools

#### Persona 2: Solo Developer (Maria)
- **Role**: Indie dev creating a visual novel alone
- **Goals**: Quick recording, simple cleanup, hear how audio sounds in-game
- **Pain Points**: Too many options, confusing routing, no presets
- **Tech Comfort**: Medium — needs guided workflow

#### Persona 3: Writer/Translator (Alex)
- **Role**: Writes dialogue and checks voice sync with text
- **Goals**: Preview voice with correct timing, navigate by character/scene
- **Pain Points**: Can't preview in context, hard to find specific lines
- **Tech Comfort**: Low — prefers visual cues over technical controls

#### Persona 4: Voice Actor (Remote - Elena)
- **Role**: External contributor recording lines at home studio
- **Goals**: See text prompt, record multiple takes, mark best take
- **Pain Points**: Complex software, unclear what's expected, sync issues
- **Tech Comfort**: Low-Medium — familiar with recording but not game engines

### User Journeys

#### Voice Studio — Top 5 Scenarios

| # | Scenario | User | Steps | Success Criteria |
|---|----------|------|-------|------------------|
| 1 | Record new voice line | Maria | Select line → Monitor input → Record → Review → Save | Recording saved with correct metadata |
| 2 | Edit existing recording | Konstantin | Load file → Apply trim/fade → Preview → Export | Non-destructive edits applied, original preserved |
| 3 | Record multiple takes | Elena | See prompt → Record Take 1 → Record Take 2 → Compare → Select best | Multiple takes stored, active take marked |
| 4 | Apply processing preset | Maria | Load clip → Select "Voice Cleanup" preset → Preview → Export | Consistent processing applied |
| 5 | Review recording quality | Alex | Play recording → Check waveform → Listen for issues | Clear visual and audio feedback |

#### Voice Studio — Rare but Important Scenarios

| # | Scenario | User | Steps | Success Criteria |
|---|----------|------|-------|------------------|
| 1 | Recover corrupted take | Konstantin | Detect corruption → Load backup → Export clean version | Data recovered without loss |
| 2 | Batch process recordings | Konstantin | Select multiple files → Apply preset → Queue processing | All files processed consistently |
| 3 | Compare A/B processing | Konstantin | Enable A/B mode → Toggle between presets → Choose winner | Quick comparison without re-applying |
| 4 | Export in different format | Maria | Select output format → Configure quality → Export | File exported in correct format |
| 5 | Handle device disconnect | Maria | Device removed → Warning shown → Graceful fallback | No crash, clear recovery path |

#### Audio Mixer — Top 5 Scenarios

| # | Scenario | User | Steps | Success Criteria |
|---|----------|------|-------|------------------|
| 1 | Preview music track | Maria | Select track → Press Play → Adjust volume | Music plays at correct level |
| 2 | Balance voice/music | Konstantin | Play voice → Enable ducking → Adjust parameters | Voice audible over music |
| 3 | Test crossfade | Maria | Load Track A → Queue Track B → Configure crossfade → Test | Smooth transition between tracks |
| 4 | Mute specific channel | Konstantin | Click Mute on channel → Verify silence → Continue mixing | Channel silenced immediately |
| 5 | Solo voice channel | Alex | Click Solo on Voice → Hear only voice → Disable solo | Isolated channel playback |

#### Audio Mixer — Rare but Important Scenarios

| # | Scenario | User | Steps | Success Criteria |
|---|----------|------|-------|------------------|
| 1 | Configure output device | Konstantin | Open settings → Select output → Apply → Test | Audio plays through correct device |
| 2 | Debug missing audio | Maria | Open channel → Check route → Verify file → Fix | Problem identified and resolved |
| 3 | A/B test mix settings | Konstantin | Save current → Modify → Compare → Choose | Quick comparison workflow |
| 4 | Export mix preview | Maria | Configure mix → Export to file → Share | Mix exported for external review |
| 5 | Handle audio device change | Maria | Device changed → Update routing → Continue | Graceful device switching |

### Typical Errors & Frustrations

#### Voice Studio
- Microphone not detected or wrong device selected
- Recording started without monitoring → clipping not noticed
- Forgot to save after editing → lost work
- Can't find the right voice line in a large project
- Processing sounds different on export than in preview

#### Audio Mixer
- Wrong channel used for playback
- Ducking too aggressive → voice still obscured
- Crossfade too short → audible gap
- Solo forgotten → confusing why channels are silent
- Can't preview how audio sounds in actual game

---

## Voice Studio Panel

### Voice Studio Information Architecture

#### Section Priority (Always Visible → Collapsible → Advanced)

| Priority | Section | Visibility | Rationale |
|----------|---------|------------|-----------|
| 1 | Transport Controls | Always | Core playback/record needs instant access |
| 2 | Waveform Display | Always | Visual feedback is essential |
| 3 | Recording/Input | Always | Device selection and monitoring |
| 4 | Basic Edit (Trim/Fade) | Collapsible | Commonly used, not always needed |
| 5 | Filters (HP/LP/EQ) | Collapsible | Audio professionals need this |
| 6 | Noise Gate | Collapsible | Common cleanup tool |
| 7 | Presets | Collapsible | Speeds up workflow |
| 8 | Advanced Settings | Hidden (Settings) | Rarely changed options |

#### Information Hierarchy

```
Voice Studio Panel
├── Toolbar (File operations, Undo/Redo)
├── Main Content
│   ├── Left Column (1/3 width)
│   │   ├── Input Device Section
│   │   │   ├── Device Selector (dropdown)
│   │   │   ├── Input Gain (slider)
│   │   │   └── VU Meter (visual)
│   │   ├── Recording Controls
│   │   │   ├── Record/Stop/Cancel buttons
│   │   │   └── Recording Timer
│   │   └── Presets Section
│   │       ├── Preset Selector (dropdown)
│   │       └── Save Preset (button)
│   └── Right Column (2/3 width)
│       ├── Transport Bar
│       │   ├── Play/Stop/Loop buttons
│       │   ├── Position Display
│       │   └── Zoom Controls
│       ├── Waveform Display
│       │   ├── Waveform visualization
│       │   ├── Selection overlay
│       │   ├── Playhead indicator
│       │   └── Trim markers
│       ├── Edit Section (collapsible)
│       │   ├── Trim Controls
│       │   ├── Fade In/Out
│       │   ├── Gain/Normalize
│       │   └── Reset button
│       └── Filter Section (collapsible)
│           ├── High-Pass Filter
│           ├── Low-Pass Filter
│           ├── 3-Band EQ
│           └── Noise Gate
└── Status Bar
    ├── File Info
    ├── Edit Status
    └── Progress Indicator
```

### Voice Studio Controls & Functionality

#### Must Have (MVP)

| Control | Type | Purpose | Shortcut |
|---------|------|---------|----------|
| Play | Button | Start playback | Space |
| Stop | Button | Stop playback | Space |
| Record | Button | Start recording | R |
| Stop Record | Button | Stop recording | R |
| Input Device | Dropdown | Select microphone | — |
| Input Gain | Slider | Adjust input level | — |
| VU Meter | Display | Show input level | — |
| Waveform | Widget | Display audio | — |
| Trim to Selection | Button | Remove outside selection | Ctrl+T |
| Fade In | SpinBox | Fade duration (ms) | — |
| Fade Out | SpinBox | Fade duration (ms) | — |
| Undo | Button | Undo last action | Ctrl+Z |
| Redo | Button | Redo last action | Ctrl+Y |
| Save | Button | Save edits | Ctrl+S |
| Open | Button | Load audio file | Ctrl+O |

#### Should Have

| Control | Type | Purpose | Shortcut |
|---------|------|---------|----------|
| Loop | Toggle | Loop playback | L |
| Zoom In/Out | Buttons | Waveform zoom | +/- |
| Zoom to Fit | Button | Fit waveform to view | F |
| Pre-Gain | SpinBox | Input boost (dB) | — |
| Normalize | Toggle | Auto-normalize | — |
| Normalize Target | SpinBox | Target level (dBFS) | — |
| High-Pass | Toggle | Enable HP filter | — |
| HP Frequency | SpinBox | HP cutoff (Hz) | — |
| Low-Pass | Toggle | Enable LP filter | — |
| LP Frequency | SpinBox | LP cutoff (Hz) | — |
| Preset Select | Dropdown | Apply preset | — |
| Save Preset | Button | Save current settings | — |
| Export | Button | Export processed audio | Ctrl+E |

#### Could Have (Future)

| Control | Type | Purpose | Shortcut |
|---------|------|---------|----------|
| 3-Band EQ | Sliders | Frequency adjustment | — |
| Noise Gate | Controls | Reduce background noise | — |
| Spectral Display | Widget | Frequency visualization | — |
| A/B Compare | Toggle | Compare settings | — |
| Batch Process | Dialog | Process multiple files | — |
| Auto-Trim Silence | Button | Remove leading/trailing silence | — |
| Voice Activity | Indicator | Show speech detection | — |

#### Keyboard Shortcuts & Focus Behavior

| Context | Key | Action |
|---------|-----|--------|
| Global | Space | Play/Pause toggle |
| Global | R | Record/Stop Record toggle |
| Global | L | Toggle loop |
| Global | Ctrl+Z | Undo |
| Global | Ctrl+Shift+Z | Redo |
| Global | Ctrl+S | Save |
| Waveform focused | +/= | Zoom in |
| Waveform focused | - | Zoom out |
| Waveform focused | F | Zoom to fit |
| Waveform focused | Home | Go to start |
| Waveform focused | End | Go to end |
| Waveform focused | Left/Right | Move selection |
| Waveform focused | Shift+Left/Right | Extend selection |

#### Parameters Reference

| Parameter | Type | Range | Default | Read-Only |
|-----------|------|-------|---------|-----------|
| Input Device | String | System devices | First available | No |
| Input Gain | Float | -12 to +12 dB | 0 dB | No |
| VU Level (RMS) | Float | -60 to 0 dB | — | Yes |
| VU Level (Peak) | Float | -60 to 0 dB | — | Yes |
| Clipping | Bool | — | false | Yes |
| Playback Position | Float | 0 to duration | 0 | Yes |
| Duration | Float | 0 to ∞ seconds | — | Yes |
| Selection Start | Float | 0 to duration | 0 | No |
| Selection End | Float | 0 to duration | 0 | No |
| Trim Start | Int | 0 to samples | 0 | No |
| Trim End | Int | 0 to samples | 0 | No |
| Fade In | Float | 0 to 5000 ms | 0 | No |
| Fade Out | Float | 0 to 5000 ms | 0 | No |
| Pre-Gain | Float | -20 to +20 dB | 0 | No |
| Normalize Enabled | Bool | — | false | No |
| Normalize Target | Float | -20 to 0 dBFS | -1 | No |
| HP Enabled | Bool | — | false | No |
| HP Frequency | Float | 20 to 500 Hz | 80 | No |
| LP Enabled | Bool | — | false | No |
| LP Frequency | Float | 1000 to 20000 Hz | 12000 | No |
| EQ Enabled | Bool | — | false | No |
| EQ Low Gain | Float | -12 to +12 dB | 0 | No |
| EQ Mid Gain | Float | -12 to +12 dB | 0 | No |
| EQ High Gain | Float | -12 to +12 dB | 0 | No |
| Noise Gate Enabled | Bool | — | false | No |
| Noise Gate Threshold | Float | -60 to -20 dB | -40 | No |

### Voice Studio Visual Model

#### Component Specifications

```
┌─────────────────────────────────────────────────────────────────────┐
│ Voice Studio                                              [_][□][X] │
├─────────────────────────────────────────────────────────────────────┤
│ [📂][💾][↶][↷] ─────────────────────────────────────────────────── │
├────────────────────────┬────────────────────────────────────────────┤
│ INPUT DEVICE           │ [▶][⏹][🔁] 00:00.000 / 00:05.234  [🔍-][🔍+]│
│ ┌────────────────────┐ ├────────────────────────────────────────────┤
│ │ USB Microphone   ▼ │ │                                            │
│ └────────────────────┘ │  ▁▂▃▄▅▆▇█▇▆▅▄▃▂▁▂▃▄▅▆▇█▇▆▅▄▃▂▁           │
│                        │  ═══════════════════════════════════════   │
│ Input Gain             │  ↑                    ↑                    │
│ [═══════○════════] 0dB │  Selection Start      Playhead             │
│                        │                                            │
│ ▐█████████░░░░░░░░▌   │ ════════════════════════════════════════   │
│ -60    -12   0   +3   │                                            │
│                        ├────────────────────────────────────────────┤
│────────────────────────│ ▼ EDIT                                     │
│ RECORDING              │ ┌──────────────────────────────────────┐   │
│ [⏺ Record] [⏹ Stop]   │ │ Trim: [Trim to Selection] [Reset]    │   │
│ 00:00.000              │ │ Fade In:  [  0.0 ] ms                 │   │
│                        │ │ Fade Out: [  0.0 ] ms                 │   │
│────────────────────────│ │ Pre-Gain: [  0.0 ] dB                 │   │
│ PRESETS                │ │ [☐] Normalize to [ -1.0 ] dBFS        │   │
│ ┌────────────────────┐ │ └──────────────────────────────────────┘   │
│ │ Voice Cleanup    ▼ │ │                                            │
│ └────────────────────┘ │ ▼ FILTERS                                  │
│ [Save Preset]          │ ┌──────────────────────────────────────┐   │
│                        │ │ [☐] High-Pass  [  80 ] Hz             │   │
│                        │ │ [☐] Low-Pass   [12000] Hz             │   │
│                        │ │ [☐] 3-Band EQ                         │   │
│                        │ │     Low:  [═══○═══] 0dB               │   │
│                        │ │     Mid:  [═══○═══] 0dB               │   │
│                        │ │     High: [═══○═══] 0dB               │   │
│                        │ │ [☐] Noise Gate  Threshold: [-40] dB   │   │
│                        │ └──────────────────────────────────────┘   │
├────────────────────────┴────────────────────────────────────────────┤
│ intro.alex.001.wav │ 48000Hz Mono 16bit │ Modified ●               │
└─────────────────────────────────────────────────────────────────────┘
```

#### Color Coding

| Element | Color | Hex | Purpose |
|---------|-------|-----|---------|
| Waveform | Cyan | #00BCD4 | Audio visualization |
| Selection | Blue (30% opacity) | #2196F3 | Selected region |
| Playhead | Green | #4CAF50 | Current position |
| Trim Markers | Orange | #FF9800 | Trim boundaries |
| Clipping Warning | Red | #F44336 | Level overflow |
| VU Safe | Green | #4CAF50 | Normal levels |
| VU Caution | Yellow | #FFC107 | Approaching limit |
| VU Danger | Red | #F44336 | Clipping zone |
| Record Active | Red (pulsing) | #F44336 | Recording in progress |

#### Widget Dimensions

| Widget | Width | Height | Notes |
|--------|-------|--------|-------|
| Panel minimum | 400px | 300px | Can resize larger |
| Left column | 200px fixed | — | Device/recording controls |
| Right column | Flexible | — | Waveform and edits |
| VU Meter | 180px | 24px | Horizontal bar |
| Waveform | Flexible | 120px min | Scrollable |
| SpinBox | 80px | 24px | Standard size |
| Button | 80px min | 28px | Text dependent |
| Dropdown | 180px | 24px | Full width in column |

### Voice Studio Text Wireframe

```
╔══════════════════════════════════════════════════════════════════════════╗
║ VOICE STUDIO                                                    [−][□][×]║
╠══════════════════════════════════════════════════════════════════════════╣
║ [📂 Open] [💾 Save] [💾 Save As] [📤 Export] │ [↶ Undo] [↷ Redo]         ║
╠═══════════════════════╦══════════════════════════════════════════════════╣
║ INPUT DEVICE          ║  TRANSPORT                                       ║
║ ┌───────────────────┐ ║  [▶ Play] [⏹ Stop] [🔁 Loop]                    ║
║ │ USB Microphone  ▼ │ ║                                                  ║
║ └───────────────────┘ ║  Position: 00:01.234 / 00:05.678                 ║
║                       ║  Zoom: [−] ═══○═══ [+] [Fit]                     ║
║ Input Gain            ╠══════════════════════════════════════════════════╣
║ [-12dB ═══○═══ +12dB] ║  WAVEFORM                                        ║
║ Current: +0.0 dB      ║  ┌────────────────────────────────────────────┐  ║
║                       ║  │    ▁▂▃▄▅▆▇█▇▆▅▄▃▂▁▁▂▃▄▅▆▇█▇▆▅▄▃▂▁▁▂▃     │  ║
║ LEVEL METER           ║  │    ▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔▔     │  ║
║ ▐████████████░░░░░▌   ║  │ ───────────────────▼─────────────────────  │  ║
║  -60  -20  -6  0 +3   ║  │ [        Selection        ]                │  ║
║ RMS: -18.2 dB         ║  └────────────────────────────────────────────┘  ║
║ Peak: -12.1 dB        ║                                                  ║
║                       ╠══════════════════════════════════════════════════╣
║ ─────────────────── ║  ▼ EDIT                              [expanded]  ║
║ RECORDING             ║  ┌────────────────────────────────────────────┐  ║
║ ┌─────────────────┐   ║  │ Trim:                                      │  ║
║ │ [⏺ Record    ] │   ║  │   [✂ Trim to Selection]  [↩ Reset Trim]   │  ║
║ │ [⏹ Stop      ] │   ║  │                                            │  ║
║ │ [✕ Cancel    ] │   ║  │ Fades:                                     │  ║
║ └─────────────────┘   ║  │   Fade In:  [    0.0 ▼] ms                 │  ║
║ Duration: 00:00.000   ║  │   Fade Out: [    0.0 ▼] ms                 │  ║
║                       ║  │                                            │  ║
║ ─────────────────── ║  │ Gain:                                      │  ║
║ PRESETS               ║  │   Pre-Gain: [    0.0 ▼] dB                 │  ║
║ ┌───────────────────┐ ║  │   [☐] Normalize to [ -1.0 ▼] dBFS         │  ║
║ │ (No Preset)     ▼ │ ║  └────────────────────────────────────────────┘  ║
║ └───────────────────┘ ║                                                  ║
║ [💾 Save Preset]      ║  ▼ FILTERS                          [expanded]  ║
║                       ║  ┌────────────────────────────────────────────┐  ║
║ Available Presets:    ║  │ [☐] High-Pass Filter                       │  ║
║ • Voice Cleanup       ║  │       Cutoff: [   80.0 ▼] Hz               │  ║
║ • Podcast Quality     ║  │                                            │  ║
║ • Natural Voice       ║  │ [☐] Low-Pass Filter                        │  ║
║ • Heavy Compression   ║  │       Cutoff: [12000.0 ▼] Hz               │  ║
║                       ║  │                                            │  ║
║                       ║  │ [☐] 3-Band EQ                              │  ║
║                       ║  │       Low  (300Hz):  [══○══] 0 dB          │  ║
║                       ║  │       Mid  (1kHz):   [══○══] 0 dB          │  ║
║                       ║  │       High (3kHz):   [══○══] 0 dB          │  ║
║                       ║  │                                            │  ║
║                       ║  │ [☐] Noise Gate                             │  ║
║                       ║  │       Threshold: [  -40.0 ▼] dB            │  ║
║                       ║  │       Reduction: [  -80.0 ▼] dB            │  ║
║                       ║  └────────────────────────────────────────────┘  ║
╠══════════════════════════════════════════════════════════════════════════╣
║ 📁 intro.alex.001.wav │ 48000 Hz │ Mono │ 16-bit │ Modified ● │ ████░░  ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Audio Mixer Panel

### Audio Mixer Information Architecture

#### Section Priority

| Priority | Section | Visibility | Rationale |
|----------|---------|------------|-----------|
| 1 | Transport Controls | Always | Play/pause needs instant access |
| 2 | Master Volume | Always | Most important control |
| 3 | Channel Strips | Always | Core mixing interface |
| 4 | Now Playing | Always | Show current track info |
| 5 | Crossfade | Collapsible | Used when switching tracks |
| 6 | Auto-Ducking | Collapsible | Voice/music balance |
| 7 | Asset Selection | Collapsible | For loading new audio |

#### Information Hierarchy

```
Audio Mixer Panel
├── Toolbar (Browse audio)
├── Main Content
│   ├── Preview Section
│   │   ├── Now Playing (track info)
│   │   ├── Transport Controls (play/pause/stop)
│   │   ├── Seek Slider
│   │   ├── Position/Duration Display
│   │   └── Loop Toggle
│   ├── Mixer Section
│   │   ├── Master Channel
│   │   │   ├── Label
│   │   │   ├── Volume Fader
│   │   │   └── Level Meter
│   │   └── Channel Strips (x6)
│   │       ├── Channel Label
│   │       ├── Volume Fader
│   │       ├── Mute Button
│   │       ├── Solo Button
│   │       └── Level Meter
│   ├── Crossfade Section (collapsible)
│   │   ├── Duration Control
│   │   ├── Next Track Selector
│   │   └── Crossfade Button
│   └── Ducking Section (collapsible)
│       ├── Enable Toggle
│       ├── Duck Amount
│       ├── Attack Time
│       └── Release Time
└── Status Bar
    └── Playback Status
```

### Audio Mixer Controls & Functionality

#### Must Have (MVP)

| Control | Type | Purpose | Shortcut |
|---------|------|---------|----------|
| Play | Button | Start playback | Space |
| Pause | Button | Pause playback | Space |
| Stop | Button | Stop playback | Escape |
| Seek Slider | Slider | Navigate in track | — |
| Loop | Toggle | Loop playback | L |
| Master Volume | Slider | Overall volume | — |
| Channel Volume (x6) | Sliders | Per-channel volume | — |
| Mute (per channel) | Toggle | Mute channel | M + number |
| Browse Audio | Button | Select audio file | — |

#### Should Have

| Control | Type | Purpose | Shortcut |
|---------|------|---------|----------|
| Solo (per channel) | Toggle | Solo channel | S + number |
| Crossfade Duration | SpinBox | Transition time | — |
| Crossfade To | Button | Start crossfade | — |
| Duck Enable | Toggle | Auto-ducking on/off | — |
| Duck Amount | SpinBox | How much to duck | — |
| Duck Attack | SpinBox | Fade in time | — |
| Duck Release | SpinBox | Fade out time | — |
| Position Display | Label | Current time | — |
| Duration Display | Label | Total time | — |

#### Could Have (Future)

| Control | Type | Purpose | Shortcut |
|---------|------|---------|----------|
| Level Meters | Visual | Show audio levels | — |
| Waveform Preview | Widget | Visual scrubbing | — |
| Pan (per channel) | Slider | Stereo position | — |
| Effects Send | Slider | Effect routing | — |
| Channel Link | Toggle | Stereo link | — |
| Output Device | Dropdown | Select audio output | — |
| Mix Presets | Dropdown | Save/load mix settings | — |

#### Audio Channels (from AudioManager)

| Channel | Index | Purpose | Default Volume |
|---------|-------|---------|----------------|
| Master | — | Overall output | 100% |
| Music | 0 | Background music | 80% |
| Sound | 1 | Sound effects | 100% |
| Voice | 2 | Voice-over | 100% |
| Ambient | 3 | Ambient sounds | 70% |
| UI | 4 | UI sounds | 100% |
| Reserved | 5 | Future use | 100% |

#### Parameters Reference

| Parameter | Type | Range | Default | Read-Only |
|-----------|------|-------|---------|-----------|
| Master Volume | Float | 0-100% | 100% | No |
| Channel Volume (x6) | Float | 0-100% | Varies | No |
| Channel Mute | Bool | — | false | No |
| Channel Solo | Bool | — | false | No |
| Playback Position | Float | 0 to duration | 0 | Yes |
| Duration | Float | 0 to ∞ | — | Yes |
| Is Playing | Bool | — | false | Yes |
| Is Paused | Bool | — | false | Yes |
| Loop Enabled | Bool | — | false | No |
| Current Track | String | — | — | Yes |
| Crossfade Duration | Float | 0-10000 ms | 1000 | No |
| Duck Enabled | Bool | — | true | No |
| Duck Amount | Float | 0-100% | 30% | No |
| Duck Attack | Float | 0-1000 ms | 200 | No |
| Duck Release | Float | 0-2000 ms | 200 | No |

### Audio Mixer Visual Model

#### Component Specifications

```
┌─────────────────────────────────────────────────────────────────────┐
│ Audio Mixer                                               [_][□][X] │
├─────────────────────────────────────────────────────────────────────┤
│ [📂 Browse Audio...]                                                │
├─────────────────────────────────────────────────────────────────────┤
│ NOW PLAYING                                                         │
│ ┌─────────────────────────────────────────────────────────────────┐ │
│ │ 🎵 main_theme.ogg                                               │ │
│ └─────────────────────────────────────────────────────────────────┘ │
│ [▶ Play] [⏸ Pause] [⏹ Stop]  [🔁 Loop]                            │
│ ═══════════════○═════════════════════════════════════════════════  │
│ 01:23.456 / 03:45.678                                               │
├─────────────────────────────────────────────────────────────────────┤
│ MIXER                                                               │
│ ┌──────┬──────┬──────┬──────┬──────┬──────┬──────┐                 │
│ │MASTER│Music │Sound │Voice │Ambient│ UI  │ Rsvd │                 │
│ ├──────┼──────┼──────┼──────┼──────┼──────┼──────┤                 │
│ │ ▮▮▮  │ ▮▮▮  │ ▮▮▮  │ ▮▮▮  │ ▮▮▮  │ ▮▮▮  │ ▮▮▮  │  ← Level Meters │
│ │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │                 │
│ │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │                 │
│ │ ┃●┃  │ ┃●┃  │ ┃●┃  │ ┃●┃  │ ┃●┃  │ ┃●┃  │ ┃●┃  │  ← Faders      │
│ │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │                 │
│ │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │ ┃ ┃  │                 │
│ ├──────┼──────┼──────┼──────┼──────┼──────┼──────┤                 │
│ │ 100% │  80% │ 100% │ 100% │  70% │ 100% │ 100% │  ← Values       │
│ ├──────┼──────┼──────┼──────┼──────┼──────┼──────┤                 │
│ │      │ [M]  │ [M]  │ [M]  │ [M]  │ [M]  │ [M]  │  ← Mute         │
│ │      │ [S]  │ [S]  │ [S]  │ [S]  │ [S]  │ [S]  │  ← Solo         │
│ └──────┴──────┴──────┴──────┴──────┴──────┴──────┘                 │
├─────────────────────────────────────────────────────────────────────┤
│ ▼ CROSSFADE                                                         │
│ Duration: [1000.0] ms    Next Track: [Select...]  [⟳ Crossfade]    │
├─────────────────────────────────────────────────────────────────────┤
│ ▼ AUTO-DUCKING                                                      │
│ [☑] Enable Ducking                                                  │
│ Duck Amount: [30] %   Attack: [200] ms   Release: [200] ms          │
└─────────────────────────────────────────────────────────────────────┘
```

#### Color Coding

| Element | Color | Hex | Purpose |
|---------|-------|-----|---------|
| Master Channel | Gold | #FFD700 | Distinguish from others |
| Music Channel | Purple | #9C27B0 | Music identification |
| Sound Channel | Blue | #2196F3 | SFX identification |
| Voice Channel | Green | #4CAF50 | Voice identification |
| Ambient Channel | Teal | #009688 | Ambient identification |
| UI Channel | Orange | #FF9800 | UI identification |
| Mute Active | Red | #F44336 | Muted state |
| Solo Active | Yellow | #FFC107 | Soloed state |
| Meter Safe | Green | #4CAF50 | Normal levels |
| Meter Warning | Yellow | #FFC107 | Approaching limit |
| Meter Clip | Red | #F44336 | Clipping |
| Seek Progress | Accent | #0078D4 | Current position |

#### Widget Dimensions

| Widget | Width | Height | Notes |
|--------|-------|--------|-------|
| Panel minimum | 450px | 350px | Can resize larger |
| Channel strip | 60px | — | Fixed width |
| Fader | 24px | 150px | Vertical slider |
| Level meter | 16px | 150px | Vertical bar |
| Mute button | 24px | 24px | Square |
| Solo button | 24px | 24px | Square |
| Seek slider | Flexible | 16px | Horizontal |
| Transport button | 60px | 28px | Text + icon |

### Audio Mixer Text Wireframe

```
╔══════════════════════════════════════════════════════════════════════════╗
║ AUDIO MIXER                                                     [−][□][×]║
╠══════════════════════════════════════════════════════════════════════════╣
║ [📂 Browse Audio...]                                                     ║
╠══════════════════════════════════════════════════════════════════════════╣
║ NOW PLAYING                                                              ║
║ ┌──────────────────────────────────────────────────────────────────────┐ ║
║ │ 🎵 bgm/main_theme.ogg                                                │ ║
║ └──────────────────────────────────────────────────────────────────────┘ ║
║                                                                          ║
║ [▶ Play] [⏸ Pause] [⏹ Stop]                              [🔁 Loop: OFF] ║
║                                                                          ║
║ ════════════════════════════○════════════════════════════════════════   ║
║ 00:01:23.456                                              00:03:45.678   ║
╠══════════════════════════════════════════════════════════════════════════╣
║ MIXER                                                                    ║
║ ┌─────────┬─────────┬─────────┬─────────┬─────────┬─────────┬─────────┐ ║
║ │ MASTER  │  MUSIC  │  SOUND  │  VOICE  │ AMBIENT │   UI    │ RESERVED│ ║
║ ├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤ ║
║ │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │ ║
║ │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │ ║
║ │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │   ▌▐    │ ║
║ │  ┃█┃   │  ┃█┃   │  ┃█┃   │  ┃█┃   │  ┃█┃   │  ┃█┃   │  ┃█┃   │ ║
║ │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │ ║
║ │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │ ║
║ │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │ ║
║ │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │  ┃ ┃   │ ║
║ │  └─┘   │  └─┘   │  └─┘   │  └─┘   │  └─┘   │  └─┘   │  └─┘   │ ║
║ ├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤ ║
║ │  100%   │   80%   │  100%   │  100%   │   70%   │  100%   │  100%   │ ║
║ ├─────────┼─────────┼─────────┼─────────┼─────────┼─────────┼─────────┤ ║
║ │         │ [M] [S] │ [M] [S] │ [M] [S] │ [M] [S] │ [M] [S] │ [M] [S] │ ║
║ └─────────┴─────────┴─────────┴─────────┴─────────┴─────────┴─────────┘ ║
╠══════════════════════════════════════════════════════════════════════════╣
║ ▼ CROSSFADE                                                  [collapsed] ║
║   Duration: [ 1000.0 ▼] ms                                               ║
║   Next Track: [                              ▼] [⟳ Crossfade To...]      ║
╠══════════════════════════════════════════════════════════════════════════╣
║ ▼ AUTO-DUCKING                                               [collapsed] ║
║   [☑] Enable Auto-Ducking (music ducks when voice plays)                 ║
║   Duck Amount: [   30 ▼] %                                               ║
║   Attack Time: [  200 ▼] ms    Release Time: [  200 ▼] ms                ║
╠══════════════════════════════════════════════════════════════════════════╣
║ ● Playing │ Music Channel │ 44100 Hz Stereo                             ║
╚══════════════════════════════════════════════════════════════════════════╝
```

---

## Integration Points

### Voice Studio Integrations

| System | Direction | Data/Events | Purpose |
|--------|-----------|-------------|---------|
| Voice Manifest | Read/Write | Line metadata, file paths | Access voice line info |
| Asset Browser | Receive | Asset selected event | Open audio files |
| Settings Registry | Read/Write | Device prefs, default format | Persist user preferences |
| Undo Manager | Send | Edit commands | Global undo/redo |
| Event Bus | Publish | Recording completed, asset updated | Notify other panels |
| Recording Studio | Share | Recorder instance | Avoid device conflicts |
| Audio Manager | Use | Preview playback | Play processed audio |

### Audio Mixer Integrations

| System | Direction | Data/Events | Purpose |
|--------|-----------|-------------|---------|
| Audio Manager | Control | Channels, playback | Mix control |
| Asset Browser | Receive | Asset selected | Load audio |
| Settings Registry | Read/Write | Volume defaults, duck settings | Persist preferences |
| Event Bus | Publish | Volume changed, playback state | Notify other panels |
| Voice Studio | Coordinate | Playback priority | Avoid audio conflicts |
| Timeline Panel | Sync | Playback position | Synchronized preview |
| Play Mode Controller | Respect | Play state | Don't interfere with runtime |

### Settings Registry Keys

```cpp
// Voice Studio Settings
"voiceStudio/inputDevice"           // string: Device ID
"voiceStudio/inputGain"             // float: Default gain (dB)
"voiceStudio/defaultSampleRate"     // int: 44100, 48000, 96000
"voiceStudio/defaultBitDepth"       // int: 16, 24, 32
"voiceStudio/autoTrimSilence"       // bool: Enable auto-trim
"voiceStudio/silenceThreshold"      // float: Silence threshold (dB)
"voiceStudio/lastPreset"            // string: Last used preset name
"voiceStudio/recentFiles"           // QStringList: Recent files

// Audio Mixer Settings
"audioMixer/masterVolume"           // float: Master volume (0-1)
"audioMixer/channelVolumes"         // QVariantList: Per-channel volumes
"audioMixer/channelMutes"           // QVariantList: Per-channel mute state
"audioMixer/duckingEnabled"         // bool: Auto-ducking enabled
"audioMixer/duckAmount"             // float: Duck amount (0-1)
"audioMixer/duckAttack"             // float: Attack time (ms)
"audioMixer/duckRelease"            // float: Release time (ms)
"audioMixer/crossfadeDuration"      // float: Crossfade duration (ms)
"audioMixer/loopEnabled"            // bool: Loop playback
```

### API Interactions

#### Voice Studio → Voice Manifest

```cpp
// Load line info
VoiceManifestLine* line = manifest->getLine("intro.alex.001");
QString filePath = line->getFilePath("en");

// Save recorded file
manifest->markAsRecorded("intro.alex.001", "en", outputPath);
manifest->addTake("intro.alex.001", "en", take);
```

#### Audio Mixer → AudioManager

```cpp
// Set channel volumes
audioManager->setChannelVolume(AudioChannel::Music, 0.8f);
audioManager->setMasterVolume(1.0f);

// Playback control
auto handle = audioManager->playMusic(config);
audioManager->pause(handle);
audioManager->seek(handle, 5.0f);

// Crossfade
audioManager->crossfadeTo(nextPath, duration, transition);

// Ducking
audioManager->setVoiceDucking(true, duckAmount, attackMs, releaseMs);
```

---

## Layout Alternatives Comparison

### Voice Studio Layouts

#### Option A: Two-Column (Recommended)

```
┌────────────────┬─────────────────────────────────┐
│ Input/Record   │ Transport + Waveform            │
│ Controls       │─────────────────────────────────│
│────────────────│ Edit Controls                   │
│ Presets        │─────────────────────────────────│
│                │ Filters                         │
└────────────────┴─────────────────────────────────┘
```

**Pros:**
- Input always visible (prevents missed clipping)
- Waveform gets maximum width
- Natural left-to-right workflow

**Cons:**
- Fixed left column wastes space when not recording

#### Option B: Stacked Vertical

```
┌──────────────────────────────────────────────────┐
│ Transport + Input (single row)                   │
├──────────────────────────────────────────────────┤
│ Waveform Display                                 │
├──────────────────────────────────────────────────┤
│ Edit Controls (tabs: Trim | Filters | Presets)   │
└──────────────────────────────────────────────────┘
```

**Pros:**
- More vertical waveform space
- Simpler layout
- Works better on narrow screens

**Cons:**
- VU meter less visible during recording
- Tabs hide frequently used controls

#### Option C: Floating Panels

```
┌──────────────────────────────────────────────────┐
│ Waveform Display (full width)                    │
├──────────────────────────────────────────────────┤
│ Transport Controls (docked bottom)               │
└──────────────────────────────────────────────────┘
[Floating: Input Panel] [Floating: Edit Panel]
```

**Pros:**
- Maximum waveform visibility
- Flexible arrangement

**Cons:**
- Complex to manage
- Window clutter
- Not consistent with other panels

**Selected: Option A** — Two-column layout provides the best balance of visibility and workflow.

### Audio Mixer Layouts

#### Option A: Horizontal Mixer (Recommended)

```
┌──────────────────────────────────────────────────┐
│ Preview/Transport                                │
├──────────────────────────────────────────────────┤
│ [MST] [CH1] [CH2] [CH3] [CH4] [CH5] [CH6]       │
│  vertical channel strips                         │
├──────────────────────────────────────────────────┤
│ Crossfade / Ducking (collapsible)               │
└──────────────────────────────────────────────────┘
```

**Pros:**
- Familiar mixer interface
- All channels visible at once
- Easy relative comparison

**Cons:**
- Requires minimum width
- Not efficient for few channels

#### Option B: Vertical List

```
┌──────────────────────────────────────────────────┐
│ Preview/Transport                                │
├──────────────────────────────────────────────────┤
│ Master:  [═══════○═══] 100% [M] [S]             │
│ Music:   [═══○═══════]  80% [M] [S]             │
│ Sound:   [═══════○═══] 100% [M] [S]             │
│ Voice:   [═══════○═══] 100% [M] [S]             │
│ ...                                              │
├──────────────────────────────────────────────────┤
│ Crossfade / Ducking                              │
└──────────────────────────────────────────────────┘
```

**Pros:**
- Works at any width
- More label space
- Expandable

**Cons:**
- Less visual impact
- Harder to compare levels
- Scrolling needed with many channels

#### Option C: Tab-Based

```
┌──────────────────────────────────────────────────┐
│ Preview/Transport                                │
├──────────────────────────────────────────────────┤
│ [Master] [Music] [Sound] [Voice] [Ambient] [UI] │
│ ┌──────────────────────────────────────────────┐ │
│ │ Selected Channel Controls                     │ │
│ │ Volume: [═══════○═══]                        │ │
│ │ Effects / Routing                            │ │
│ └──────────────────────────────────────────────┘ │
└──────────────────────────────────────────────────┘
```

**Pros:**
- Maximum detail per channel
- Future-proof for complex routing

**Cons:**
- Can't see all channels at once
- More clicks to adjust
- Inconsistent with mixer conventions

**Selected: Option A** — Horizontal mixer is industry standard and most intuitive.

---

## Error & Empty States

### Voice Studio States

#### Empty States

| State | Message | Action |
|-------|---------|--------|
| No file loaded | "No audio loaded. Open a file or record new audio." | [Open File] [Record] buttons |
| No input device | "No microphone detected. Please connect a recording device." | [Refresh Devices] button |
| Recording in progress | "Recording... Press Stop to save or Cancel to discard." | Timer counting up |
| Processing | "Processing audio..." | Progress bar with cancel option |

#### Error States

| Error | Message | Recovery |
|-------|---------|----------|
| Device not found | "⚠️ Selected device not available: {name}" | Device dropdown auto-selects default |
| File open failed | "❌ Could not open file: {path}\nReason: {error}" | Show file dialog |
| Recording failed | "❌ Recording failed: {error}" | Stop recording, show error details |
| Export failed | "❌ Export failed: {error}\nCheck disk space and permissions." | Retry button |
| Corrupt file | "⚠️ Audio file may be corrupted. Some data may be missing." | Load what's possible, warn user |

#### Validation States

| Condition | Visual Feedback |
|-----------|-----------------|
| Clipping detected | VU meter shows red, "Clipping!" warning text |
| No selection for trim | "Trim to Selection" button disabled, tooltip explains |
| Unsaved changes | Title bar shows "●", close prompts save dialog |
| Recording too short | Warning: "Recording is very short (<100ms). Continue?" |

### Audio Mixer States

#### Empty States

| State | Message | Action |
|-------|---------|--------|
| No track loaded | "No audio selected. Browse for an audio file to preview." | [Browse Audio] button |
| Audio not playing | (Transport shows stopped state) | Play button enabled |
| All channels muted | "⚠️ All channels are muted" | Highlight muted channels |

#### Error States

| Error | Message | Recovery |
|-------|---------|----------|
| File not found | "❌ Audio file not found: {path}" | Browse for replacement |
| Decode error | "❌ Could not decode audio: {error}" | Try different file |
| Playback failed | "❌ Playback error: {error}" | Stop, show error |
| Device changed | "ℹ️ Audio device changed. Restarting playback..." | Auto-restart |

#### State Indicators

| State | Visual Indicator |
|-------|------------------|
| Playing | Green "●" indicator, animated transport |
| Paused | Yellow "⏸" indicator |
| Stopped | Gray "⏹" indicator |
| Looping | Loop button highlighted |
| Solo active | "SOLO" badge on channel, other channels dimmed |
| Mute active | "M" button red, channel dimmed |
| Ducking active | Duck indicator on Voice channel |

---

## Acceptance Criteria

### Voice Studio Panel

- [ ] **D1: User Journeys** — All 10 scenarios documented with clear steps
- [ ] **D2: Information Architecture** — Section priorities defined, collapsible groups implemented
- [ ] **D3: Controls** — Must/Should/Could categorized, shortcuts defined
- [ ] **D4: Visual Model** — Colors, dimensions, and component specs defined
- [ ] **D5: Integrations** — All system connections documented with API examples
- [ ] **D6: Wireframe** — Text-based wireframe with all UI elements

### Audio Mixer Panel

- [ ] **D1: User Journeys** — All 10 scenarios documented with clear steps
- [ ] **D2: Information Architecture** — Section priorities defined
- [ ] **D3: Controls** — Must/Should/Could categorized, shortcuts defined
- [ ] **D4: Visual Model** — Colors, channel strips, and layout defined
- [ ] **D5: Integrations** — AudioManager integration documented
- [ ] **D6: Wireframe** — Text-based wireframe with all channels

### Quality Requirements

- [ ] **Stable IDs** — All controls have stable ID anchors for tutorial system
- [ ] **Settings Registry** — All configurable parameters use SettingsRegistry
- [ ] **Feedback** — Every operation has progress/status/toast feedback
- [ ] **Async Operations** — Waveform analysis, encoding run off UI thread
- [ ] **Undo/Redo** — All edits support undo/redo via NMUndoManager

### Implementation Notes

1. **UI Framework**: Qt 6 Widgets (no DAW-level graphics required)
2. **Audio Backend**: Existing `audio::AudioManager` and `audio::AudioRecorder`
3. **Persistence**: `QSettings` via SettingsRegistry pattern
4. **Threading**: `QThread` or `QtConcurrent` for audio processing
5. **Styling**: Use existing `NMStyleManager` dark theme

---

## Appendix: Tooltips & Learn More

### Voice Studio Tooltips

| Control | Tooltip |
|---------|---------|
| Input Gain | "Adjust microphone input level. Aim for peaks around -12dB." |
| Record | "Start recording from the selected input device (R)" |
| Trim to Selection | "Remove audio outside the selected region (Ctrl+T)" |
| Fade In | "Gradually increase volume at the start (in milliseconds)" |
| High-Pass | "Remove low-frequency rumble below the cutoff frequency" |
| Normalize | "Automatically adjust peak level to the target value" |
| Noise Gate | "Reduce audio below the threshold to suppress background noise" |

### Audio Mixer Tooltips

| Control | Tooltip |
|---------|---------|
| Master Volume | "Overall output volume for all audio" |
| Channel Mute | "Mute this channel (M+{number})" |
| Channel Solo | "Solo this channel - hear only this channel (S+{number})" |
| Loop | "Repeat playback continuously (L)" |
| Crossfade | "Smoothly transition to the next track" |
| Duck Amount | "How much to reduce music volume when voice plays" |
| Duck Attack | "How quickly music fades down when voice starts" |
| Duck Release | "How quickly music returns when voice stops" |

---

*Document End — Ready for Implementation*

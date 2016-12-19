# Mixer application

A simple AGL-style PulseAudio mixer app.

## Features

  * Supports all reported sinks and sources
  * Multi-channel support via a per-channel volume control slider

## Limitations

  * No source-output or sink-input support
  * No mute toggle support
  * No channel slider binding
  * No logarithmic volume control
  * No event support (does not listen or respond to PA events from other clients)
  * Fixed raw volume range

# obs-doom-transition

![OBS Plugin](https://img.shields.io/badge/OBS-Plugin-blue)

**obs-doom-transition** is an OBS Studio plugin that adds a "Doom screen melt" style transition to your scene switching. Inspired by the classic screen-melting effect from the 1993 game *DOOM*, this transition brings a nostalgic touch to your live streams and recordings.

## Features

- **Doom Screen Melt Transition**: Adds a dramatic melting effect when switching between scenes, similar to the iconic transition from the original DOOM game.
- **Customizable Speed**: Adjust the speed of the melting transition to suit your style.
- **Lightweight**: Minimal impact on OBS performance.
- **Simple to Use**: Easily added and customized within OBS Studio's transition options.

## Installation

### Windows

1. Download the latest release from the [Releases](https://github.com/LucienHH/obs-doom-transition/releases) section.
2. Extract the contents of the downloaded zip file.
3. Copy the `obs-doom-transition` folder into your OBS Studio plugins directory:
   - Default path: `C:\Program Files\obs-studio\obs-plugins\`
4. Restart OBS Studio.

### macOS / Linux

Installation instructions for macOS and Linux will be added soon. You can try to compile the plugin yourself on Linux using the guide below.

#### How to manually build plugin on Linux (Ubuntu/Debian/Mint):

```
git clone https://github.com/LucienHH/obs-doom-transition/
cd obs-doom-transition
sudo apt-get install cmake build-essential
mkdir build
cd build
cmake ..
make
```

1. Create `~/.config/obs-studio/plugins/obs-doom-transition` folder
2. copy freshly built `obs-doom-transition.so` file from the `build/` folder in cloned git repo to `bin/64bit/` subfolder of the plugin folder
3. copy contents of entire `data/` folder from the git repo to `~/.config/obs-studio/plugins/obs-doom-transition/data`

## Usage

1. Open OBS Studio.
2. Go to the **"Scene Transitions"** section.
3. Click on the **"+"** button to add a new transition.
4. Select **"Doom"** from the list of available transitions.
5. Adjust the transition speed using the provided slider (optional).
6. Enjoy the DOOM-style melting effect when switching between scenes!

## Contributing

Contributions are welcome! Please feel free to submit pull requests or open issues if you encounter bugs or have suggestions for improvements.

## License

This project is licensed under the [MIT License](LICENSE).

---

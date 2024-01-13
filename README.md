![WiPadRetro](./images/banner.png)

# WiPadRetro

WiPadRetro transforms your WiFi-enabled handheld device into a PC gamepad. It allows you to map the buttons on the handheld device to corresponding keyboard keypresses on a PC by streaming the button states through UDP. 

**NOTE:** This project was only tested on a Miyoo Mini Plus, running on other devices might require some additional modifications.

![WiPadRetro Demonstration (Contra)](./images/gameplay.gif)

## Building

WiPadRetro is split into two main components: WiPadRetro (the sender) running on the handheld device, and WiPadRetroLink (the receiver) running on the PC. If you don't want to build the project yourself, you can download the latest release [here](https://github.com/adildsw/WiPadRetro/releases).

### Building WiPadRetro
Cross-compiling WiPadRetro for the appropriate handheld architecture will require its corresponding toolchain/development environment (e.g., [Miyoo Mini Toolchain](https://github.com/MiyooMini/union-toolchain)). Additionally, you will also need to have the SDL1.2 library compiled for the corresponding handheld architecture.

#### 1. Load the appropriate toolchain for the handheld device with the cross-compiler and SDL1.2.

#### 2. Clone the repository
```
git clone https://github.com/adildsw/WiPadRetro
```

#### 3. Navigate to the *WiPadRetro/wipadretro* directory
```
cd WiPadRetro/wipadretro
``` 

#### 4. Edit the *CC*, *SDL_CFLAGS*, and *SDL_LIBS* flags within the *makefile* to link to the cross-compiler and SDL1.2 library directories
```
# Cross-compiler and SDL paths (Modify these to match your system)
CC = ${CROSS_COMPILE}gcc
SDL_CFLAGS = -I${PREFIX}/include/SDL/
SDL_LIBS = -L${PREFIX}/lib/ -lSDL -lSDL_image -lSDL_ttf
```

#### 5. Once the makefile is configured, run the following command to build WiPadRetro
```
make clean && make
```
The WiPadRetro binary will be located as *app* in the *WiPadRetro/wipadretro/build* directory.

#### 6. [Optional] Modifying handheld device button mapping:
A *config.ini* is created in the *WiPadRetro/wipadretro/build* directory when WiPadRetro is run for the first time. This file contains the default button mappings for the handheld device. You can edit this file to change the default button mappings associated with the handheld device.

### Installing WiPadRetroLink
WiPadRetroLink is written in Python and requires you to have Python 3.6+ installed on your system. WiPadRetroLink can be installed simply using the setup.py script.

#### 1. Clone the repository
```
git clone https://github.com/adildsw/WiPadRetro
```
#### 2. Navigate to the *WiPadRetro/wipadretrolink* directory
```
cd WiPadRetro/wipadretrolink
``` 
#### 3. Run the setup.py script
```
pip install .
```
#### 4. Once the setup is complete, you can run WiPadRetroLink in the terminal using the following command
```
wprlink
```

![WiPadRetro Retroarch Connection](./images/retroarch.gif)

**Note:** WiPadRetro inputs have been tested to work with RetroArch. However, OpenEmu does not seem to recognize the inputs.

## Contribution

This project is a work in progress, and the current build might not be optimally efficient, and can lead to missed frame inputs. Contributions are highly welcomed and appreciated. If you have ideas for improvements or optimizations, feel free to fork the repository, make your changes, and submit a pull request.


## License

WiPadRetro is licensed under the GPL 3.0 License.

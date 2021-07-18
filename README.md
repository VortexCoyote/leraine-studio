# leraine-studio
A cross-platform open-source VSRG chart editor

![screenshot](https://i.imgur.com/WmF2Gny.png "screenshot")
# Compilation

This project uses [cmake tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) for [vscode](https://code.visualstudio.com/) and [vcpkg](https://github.com/microsoft/vcpkg).

Define `VCPKG_ROOT` within `.vscode/settings.json` with your full vcpkg install location.

## **Windows**

preferred compiler: `msvc` 

vcpkg packages: 
```
imgui-sfml:x64-windows
sfml:x64-windows
```
## **Linux**

preferred compiler: `gcc`

vcpkg packages: 
```
imgui-sfml:x64-linux
sfml:x64-linux
```
additional linux packages:
```
libglade2-dev
```
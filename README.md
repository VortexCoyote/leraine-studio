# leraine-studio
A cross-platform open-source VSRG chart editor

![screenshot](https://i.imgur.com/WmF2Gny.png "screenshot")

## Why does this exist?
As a VSRG charter myself, I've always been discontent with the current availible editors. All having their own perks, while all are lacking features from each other. This chart editor is a personal attempt to combine the editing convenience from the osu!mania editor, the look and UI of Arrow Vortex and the timing tools from DDreamStudio, while keeping myself as the target audience as a priority. 
## Why "Leraine"?
"Leraine" by Kettel and Secede is one of my favourite songs from one of my favourite albums. Since this editor is somewhat personal, I thought it would be nice to name it accordingly.  
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

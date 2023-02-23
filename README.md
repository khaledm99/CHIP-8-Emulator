# CHIP-8-Emulator
WIP CHIP-8 Emulator, written in C++. Uses SDL2 to render the CHIP-8 screen.
This project was created following the guide at https://tobiasvl.github.io/blog/write-a-chip-8-emulator/.
This guide is a minimal guide to walk a novice emulator dev through creating a CHIP-8 emulator without explicitly providing implementation solutions.
The project currently implements the opcodes necessary to run a handful of CHIP-8 test ROMs, several of which can be found in the build folder.
Display of the CHIP-8 screen fully works and was implemented with SDL2. 
TODO: 
- Reorganize and document code
- Debug and fix incorrect behaviour with jumping opcodes
- Complete implementation of remaining opcodes
- Implement debug features such as single-cycle stepping, emulator pausing, memory viewing/editing, color-scheme selection, and variable CPU-option selection
- Record demo/screenshots for git repo

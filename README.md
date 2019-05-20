# skryptNES

A work in progress NES emulator written in C.

# Progress

* Can only load Mapper 0 games, like Super Mario Bros, Donkey Kong etc.
* 65C02 emulation is almost fully accurate. 
  * Almost all instructions are implemented and appear to work correctly.
  * All addressing modes are implemented, including the newer 65C02 indirect modes. Implementation may need to be fixed.
  * Cycle count may not be accurate. Also note that speed is set to 400Hz for debug purposes by default.
* PPU and APU support is non existent, so it is definitely not usable anytime now.


# rtrb-capi-example

An example program that uses [`rtrb-capi`][rtrb-capi] and [SDL][sdl] to play a
note.

A producer thread writes audio samples to an `rtrb` ring buffer. An SDL audio
callback then reads samples from that ring buffer.

## Build

You need Rust, Cargo, CMake, and a C/C++ compiler installed.

Configure and build the project:

```bash
$ cmake -S . -B build
$ cmake --build build
```

Then run `./build/src/rtrb-example` (the path to `rtrb-example` may be different
for you). The program should play a note for four seconds and exit.

## Acknowledgements

Most of the SDL audio code is from https://blog.fredrb.com/2023/08/08/audio-programming-note-sdl/.

[rtrb-capi]: https://github.com/zachcmadsen/rtrb-capi
[sdl]: https://github.com/libsdl-org/SDL
# rtrb-capi-example

An example program that uses [`rtrb-capi`][rtrb-capi] and [SDL][sdl] to play a
note.

A producer thread generates and writes samples to an `rtrb` ring buffer. The SDL
audio callback then reads samples from the ring buffer.

## Build

You need Rust, Cargo, CMake, and a C compiler installed.

Configure and build the project:

```bash
$ cmake -S . -B build
$ cmake --build build
```

You should be able to run `./build/bin/rtrb-example` (the path to `rtrb-example`
in the `bin` directory may be different for you) and hear a note.

## Acknowledgements

Most of the SDL audio code is from https://blog.fredrb.com/2023/08/08/audio-programming-note-sdl/.

[rtrb-capi]: https://github.com/zachcmadsen/rtrb-capi
[sdl]: https://github.com/libsdl-org/SDL
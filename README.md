# From C++ livecoding audio to WebAudio API
This is an example of how to go from livecoding audio in C++ to compiling with emscripten into something that can run on a modern web browser. This repo is mostly here to remind me how I got it all set up and the Makefiles etc, hopefully it can be helpful to others!

Here's a demo of what it generates for web: https://elf-audio.github.io/cpp2emscripten-demo/

The example is a sort of gong simulation, just for testing purposes, the purpose isn't to explain that code, it's the process of 

Prerequisites: Mac only, xcode tools and emscripten installed

To compile and run the livecoding version, clone this repo, cd to it and type `make`

That should build the livecoding c++ app and the web version

## Livecoding version
If you then type `./audio` it should run the C++ livecoding version - you can go into src/MyLiveAudio.h and tweak the code to change the sound as you wish.

This livecoding system all works because of this: https://github.com/elf-audio/cppsketch and is based on the audio example in there.

## WebAudio version
If you cd to the www directory and type `python -m SimpleHTTPServer 8080`, which runs a local webserver from this directory, then go to http://localhost:8080/ in your browser (then press start), that is the C++ code, transpiled into Javascript, that's making the noise you hear!

How this works: the makefile calls `emcc` to compile a file called `src/em.cpp` which basically just calls the dsp code in the main source, asking it for samples. Then there's a little bit of javascript that interacts with emscripten in `wwww/index.html` to grab that audio and send it out of the speaker. The html file is heavily commented with my findings.

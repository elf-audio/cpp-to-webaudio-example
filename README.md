# From C++ livecoding audio to WebAudio API
This is an example of how to go from livecoding audio in C++ to compiling with emscripten into something that can run on a modern web browser. This repo is mostly here to remind me how I got it all set up and the Makefiles etc, hopefully it can be helpful to others!

The example is a sort of gong simulation, just for testing purposes.

Prerequisites: Mac only, xcode tools and emscripten installed

To compile and run the livecoding version, clone this repo, cd to it and type `make`

That should build the livecoding c++ app and the web version

## Livecoding version
If you then type `./audio` it should run the C++ livecoding version - you can go into src/MyLiveAudio.h and tweak the code to change the sound as you wish.

## WebAudio version
If you cd to the www directory and type `python -m SimpleHTTPServer 8080`, which runs a local webserver from this directory, then go to http://localhost:8080/ in your browser (then press start), that is the C++ code, transpiled into Javascript, that's making the noise you hear!


RTAUDIO = lib/rtaudio
CPPSKETCH = lib/cppsketch
FRAMEWORKS = -framework CoreAudio -framework CoreFoundation
INCLUDES = -I$(CPPSKETCH)/ -I$(RTAUDIO)
CPPFILES = $(CPPSKETCH)/Dylib.cpp $(CPPSKETCH)/FileWatcher.cpp $(CPPSKETCH)/liveCodeUtils.cpp
CPPFLAGS = -D__MACOSX_CORE__ -stdlib=libc++ -std=c++14 $(FRAMEWORKS)
SOURCES = src/main.cpp $(CPPFILES) $(RTAUDIO)/RtAudio.cpp
EMSCRIPTEN_OPTIONS = -s WASM=1 \
-s ALLOW_MEMORY_GROWTH=1 \
-s EXPORTED_FUNCTIONS="['_getSamples', '_malloc', '_free']" \
-s EXTRA_EXPORTED_RUNTIME_METHODS="['cwrap','ccall']"

all:
	g++ $(CPPFLAGS) $(SOURCES) $(INCLUDES) -o audio
	emcc src/em.cpp -O3 $(INCLUDES) -Ilib/freeverb -o www/em.js $(EMSCRIPTEN_OPTIONS)

# this don't work
# serve:
# 	cd www
# 	python -m SimpleHTTPServer 8080



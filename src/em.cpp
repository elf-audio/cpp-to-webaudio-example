#include "MyLiveAudio.h"
MyLiveAudio audio;
extern "C" {
	void getSamples(float *samples, int length, int numChans) {
		audio.audioOut(samples, length, numChans);
	}
}
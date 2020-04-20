#include "LiveAudio.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

using namespace std;


// get rid of DC!!!
// listen to more gongs
// work out how to get some dispersion going
// each strike should vary how much it gives to each Delay
// how to rein in feedback
// initial pitch bend at the beginning wow


//
//  OnePole.h
//

#pragma once

class OnePole {
public:
	float last;
	float feedback;
	
	OnePole() {
		last = 0;
		feedback = 0.5;
	}
	
	float filter(float val) {
		float out = val * (1-feedback) + last * feedback;
		last = val;
		return out;
	}
	
	float filterRecursive(float val) {
		float out = val * (1-feedback) + last * feedback;
		last = out;
		return out;
	}
	
	void clear() {
		last = 0;
	}
};



class GongDelay {
public:
	vector<float> data;
	int readHead = 0;
	int writeHead = 0;
	int length = 0;
	OnePole filter;
	float freqToSamplePeriod(float freq) {
		return 44100.f / freq;
	}

float freq;

	GongDelay(float freq) {
		this->freq = freq;
		int l = freqToSamplePeriod(freq);
		data.resize(l, 0);
		setLength(l);
		filter.feedback = 0.7;
	}
	void setLength(int length) {
		this->length = length;
		readHead = length;
		writeHead = 0;
		std::fill(data.begin(), data.begin() + length, 0);
	}

	float read() {
		float val = data[readHead];
		readHead++;
		readHead %= length;
		return val;
	}
	long t = 0;
	void write(float inp) {
		// filter.feedback = 0.7 + sin(t *0.01/ freq) * 0.3;
		t++;
		data[writeHead] = filter.filterRecursive(inp);
		writeHead++;
		writeHead %= length;
	}
};
float clip(float inp) {
	if(inp>1) return 1;
	if(inp<-1) return -1;
	return inp;
}

#include "impulse.h"
#include "Biquad.h"

float clampf(float inp, float from, float to) {
	if(inp < from) return from;
	if(inp>to) return to;
	return inp;
}

float mapf(float inp, float inMin, float inMax, float outMin, float outMax, bool clamp) {
	float norm = (inp - inMin) / (inMax - inMin);
	float f = outMin + (outMax - outMin) * norm;
	if(clamp) {
		if(outMax > outMin) {
			return clampf(f, outMin, outMax);
		} else {
			return clampf(f, outMax, outMin);
		}
	}
	return f;
}


float randuf() {
	return rand() / (float)  RAND_MAX;
}
float randf() {
	return randuf() * 2.f - 1.f;
}
float randf(float to) {
	return randuf() * to;
}


float randf(float from, float to) {
	return from + randuf() * (to - from);
}

int randi(int to) {
	return rand() % (to+1);
}
int randi(int from, int to) {
	return from + rand() % (to+1-from);
}
#include "revmodel.hpp"

class DCFilter {
public:
	/*
	Some audio algorithms (asymmetric waveshaping, cascaded filters, ...) can produce DC
offset. This offset can accumulate and reduce the signal/noise ratio.

So, how to fix it? The example code from Julius O. Smith's document is:
*/

	float R;

	DCFilter() {
		// "R" depends on sampling rate and the low frequency point. Do not set "R" to a fixed value
		// (e.g. 0.99) if you don't know the sample rate. Instead set R to:
		// (-3dB @ 40Hz): R = 1-(250/samplerate)
		// (-3dB @ 30Hz): R = 1-(190/samplerate)
		// (-3dB @ 20Hz): R = 1-(126/samplerate)
		R = 1.f - 30.f / 44100.f; // (30Hz)

	}
	float prevInput = 0.f;
	float prevOutput = 0.f;
	float filter(float input) {
		// y(n) = x(n) - x(n-1) + R * y(n-1)
		// "R" between 0.9 .. 1
		// n=current (n-1)=previous in/out value

		float output = input - prevInput + R * prevOutput;
		prevInput = input;
		prevOutput = output;
		return output;
	}
};

class GongNetwork {
public:
	vector<GongDelay> delays;

	Biquad sympatheticFilter;	
	float fundamental;
	DCFilter dcFilter;
	float pan;
	GongNetwork(float fundamental, const vector<float> &harmonics, float pan) {
		this->pan = pan;
		this->fundamental = fundamental;
		

		for(auto h : harmonics) {
			float hh = h;
			//hh = round(hh);
			delays.emplace_back(GongDelay(fundamental * hh));

		}
		sympatheticFilter.calc(Biquad::LOWPASS,900,44100,0.5,0, false);

	}


	void process(float inp, float &outL, float &outR) {
		
		vector<float> outs(delays.size());
		
		for(int i = 0; i < delays.size(); i++) {
			outs[i] = delays[i].read();
		}


		for(int i = 0; i < delays.size(); i++) {
			float sympathetic = 0.f;
			for(int k = 0; k < delays.size(); k++) {
				if(i!=k) {
					sympathetic += outs[i];
				}
			}
			sympathetic /= (delays.size()-1.f);
			sympathetic = sympatheticFilter.filter(sympathetic);
			float o = 
			dcFilter.filter(
				outs[i] * 0.995
			 + sympathetic * 0.04);//out*0.99/(float)delays.size()+inp;
			delays[i].write(inp + o);
		}

		outL = 0;
		outR = 0;

	bool panning = true;
	float maxPanWidth = min(pan, 1.f - pan);
		for(int i = 0; i < outs.size(); i++) {
			if(panning) {
				float rrr = i * (M_PI+ fundamental);
				int iii = rrr;
				float pan = rrr - iii;
				pan = this->pan + pan * maxPanWidth;
				outL += pan * outs[i];
				outR += (1.f - pan) * outs[i];
			} else {
				outL += outs[i];
				outR += outs[i];
			}
		}
	
	}
};



// float randf() {
// 	return 2.f * ((rand() % 10000) / 10000.f - 0.5f);
// }

class Exciter {
public:
	int pos = 0;
	int durationSamples = 0;
	Biquad noiseFilter;
	
	Exciter(float duration, float offset = 0) {
		durationSamples = duration * 22050.f;
		pos = offset * 22050.f;
		noiseFilter.calc(Biquad::LOWPASS,2000,44100,1,0, false);
	}

	int minSpeed = 3;
	float noise = 1;
	float force = 1;
	int speed = 3;
	float getSample() {



		float out = 0;
		
		if(pos==0) {
			speed = randi(minSpeed, 8);
			force = randf(1, 4);
			noise = randf(0.5, 2);
			// printf("Speed: %d\n", speed);
			noiseFilter.calc(Biquad::LOWPASS,randf(1000, 4000), 44100, 1, 0, false);
		}


		if(pos<impulse.size()/speed) {
			int noiseSize = 1000;
			float r = + randf() * 0.2 * (noiseSize - pos) / (float)noiseSize;

			if(pos>noiseSize) r = 0;
			//if(pos<20) r *= pos / 20.f;
			//out = r;
			r = noiseFilter.filter(r) * 0.4 * noise;
			out = impulse[pos*speed]*0.4 + r;
			out *= force;
		}
		pos++;
		pos %= durationSamples;
		return out;
	}
};
class Limiter {
public:
	float gain = 1;
	void process(float &l, float &r) {
		if(max(l, r)>1) {
			gain = 1.f / max(l, r);
		} else {
			if(gain<1) gain *= 1.0001;
		}
		l *= gain;
		r *= gain;
	}
};
class Gong {
public:
	GongNetwork network;
	Exciter exciter;
	Gong(float fundamental, const vector<float> &harmonics, float pan, float duration, float offset = 0) :
	network(fundamental, harmonics, pan),
	exciter(duration, offset) {

	}

	void getSample(float &l, float &r) {
		network.process(exciter.getSample(), l, r);
	}

};


class MyLiveAudio : public LiveAudio {
public:


	vector<Gong> gongs;
	Limiter limiter;
	DCFilter dcl, dcr;
	
	revmodel freeverb;

	MyLiveAudio() : LiveAudio() {
 		gongs = {
 			{40, {1, 2.04, 3.01, 4.2, 5.4, 6.8, 9.8, 11.2, 19}, 0.7, 4},
			{99, {1, 2.02, 3.02, 4.2, 5.4, 6.8, 9.8, 11.2, 19}, 0.7, 4, 3},
 			{68, {1, 2.03, 3.02, 4.4, 3.9, 2.2, 6.83, 9.3}, 0.3, 8, 3},
 			{77, {1, 2.03, 3.02, 4.4, 3.9, 6.83, 9.3}, 0.5, 8, 6.5},
 			{77, {1, 1.99, 2.94, 4.4, 3.9, 6.83, 9.3}, 0.5, 5},
 			{202, {1, 2.03, 3.02, 4.4, 3.9, 6.83, 9.3}, 0.5, 3}
 		};
		freeverb.setroomsize(0.9);
		freeverb.setwet(0.1);
		freeverb.setdry(0.5);
		freeverb.setwidth(0.4);
		freeverb.setdamp(0.5);
 	}

	int printPos = 0;
float maxOut = 0;
	void audioOut(float *samples, int length, int numChans) override {
		//printf("lg: %f\n", limiter.gain);
		for(int i = 0; i < length; i++) {

			for(auto &g: gongs) {
				float l, r;
				g.getSample(l, r);
				samples[i*2] += l;//outL;
				samples[i*2+1] += r;//outR;
			}


			samples[i*2] *= 0.1;//outL;
			samples[i*2+1] *= 0.1;//outR;
		

			samples[i*2] = dcl.filter(samples[i*2]);		
			samples[i*2+1] = dcr.filter(samples[i*2+1]);		
			limiter.process(samples[i*2], samples[i*2+1]);
		// allpasses.process(samples[i*2], samples[i*2+1]);

			printPos++;
			if(printPos==4410) {
				printPos = 0;
				//printf("%f\n", samples[i*2]);
			}
		}
		freeverb.processInPlaceStereoInterleaved(samples, length);



		for(int i = 0; i < length; i++) {
			float a = abs(max(samples[i*2], samples[i*2+1]));
			if(a>maxOut) {
				maxOut = a;
			} else {
				maxOut *= 0.9999;
			}
		}
//		printf("Max: %.2f\n", maxOut);
	}	
};










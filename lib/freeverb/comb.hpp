// Comb filter class declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#pragma once

#include "denormals.h"

class comb
{
public:
	comb() {
		filterstore = 0;
		bufidx = 0;
	}
	void	setbuffer(float *buf, int size) {
		buffer = buf;
		bufsize = size;
	}
	inline  float	process(float input) {
		float output;

		output = buffer[bufidx];
		undenormalise(output);

		filterstore = (output * damp2) + (filterstore * damp1);
		undenormalise(filterstore);

		buffer[bufidx] = input + (filterstore * feedback);

		if (++bufidx >= bufsize) bufidx = 0;

		return output;
	}
	void	mute() {
		for (int i = 0; i < bufsize; i++)
			buffer[i] = 0;
	}

	void	setdamp(float val) {
		damp1 = val;
		damp2 = 1 - val;
	}
	float	getdamp() {return damp1;}
	void	setfeedback(float val) {feedback = val;}
	float	getfeedback() {return feedback;}
private:
	float	feedback;
	float	filterstore;
	float	damp1;
	float	damp2;
	float	*buffer;
	int		bufsize;
	int		bufidx;
};



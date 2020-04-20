// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#pragma once
#include "comb.hpp"
#include "allpass.hpp"
#include "tuning.h"

class revmodel
{
public:
	revmodel()
	{
		// Tie the components to their buffers
		combL[0].setbuffer(bufcombL1, combtuningL1);
		combR[0].setbuffer(bufcombR1, combtuningR1);
		combL[1].setbuffer(bufcombL2, combtuningL2);
		combR[1].setbuffer(bufcombR2, combtuningR2);
		combL[2].setbuffer(bufcombL3, combtuningL3);
		combR[2].setbuffer(bufcombR3, combtuningR3);
		combL[3].setbuffer(bufcombL4, combtuningL4);
		combR[3].setbuffer(bufcombR4, combtuningR4);
		combL[4].setbuffer(bufcombL5, combtuningL5);
		combR[4].setbuffer(bufcombR5, combtuningR5);
		combL[5].setbuffer(bufcombL6, combtuningL6);
		combR[5].setbuffer(bufcombR6, combtuningR6);
		combL[6].setbuffer(bufcombL7, combtuningL7);
		combR[6].setbuffer(bufcombR7, combtuningR7);
		combL[7].setbuffer(bufcombL8, combtuningL8);
		combR[7].setbuffer(bufcombR8, combtuningR8);
		allpassL[0].setbuffer(bufallpassL1, allpasstuningL1);
		allpassR[0].setbuffer(bufallpassR1, allpasstuningR1);
		allpassL[1].setbuffer(bufallpassL2, allpasstuningL2);
		allpassR[1].setbuffer(bufallpassR2, allpasstuningR2);
		allpassL[2].setbuffer(bufallpassL3, allpasstuningL3);
		allpassR[2].setbuffer(bufallpassR3, allpasstuningR3);
		allpassL[3].setbuffer(bufallpassL4, allpasstuningL4);
		allpassR[3].setbuffer(bufallpassR4, allpasstuningR4);

		// Set default values
		allpassL[0].setfeedback(0.5f);
		allpassR[0].setfeedback(0.5f);
		allpassL[1].setfeedback(0.5f);
		allpassR[1].setfeedback(0.5f);
		allpassL[2].setfeedback(0.5f);
		allpassR[2].setfeedback(0.5f);
		allpassL[3].setfeedback(0.5f);
		allpassR[3].setfeedback(0.5f);
		setwet(initialwet);
		setroomsize(initialroom);
		setdry(initialdry);
		setdamp(initialdamp);
		setwidth(initialwidth);
		setmode(initialmode);

		// Buffer will be full of rubbish - so we MUST mute them
		mute();
	};
	void	mute()
	{
		if (getmode() >= freezemode)
			return;

		for (int i = 0; i < numcombs; i++)
		{
			combL[i].mute();
			combR[i].mute();
		}
		for (int i = 0; i < numallpasses; i++)
		{
			allpassL[i].mute();
			allpassR[i].mute();
		}
	}
	void	processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip)
	{
		float outL, outR, input;

		while (numsamples-- > 0)
		{
			outL = outR = 0;
			input = (*inputL + *inputR) * gain;

			// Accumulate comb filters in parallel
			for (int i = 0; i < numcombs; i++)
			{
				outL += combL[i].process(input);
				outR += combR[i].process(input);
			}

			// Feed through allpasses in series
			for (int i = 0; i < numallpasses; i++)
			{
				outL = allpassL[i].process(outL);
				outR = allpassR[i].process(outR);
			}

			// Calculate output MIXING with anything already there
			*outputL += outL * wet1 + outR * wet2 + *inputL * dry;
			*outputR += outR * wet1 + outL * wet2 + *inputR * dry;

			// Increment sample pointers, allowing for interleave (if any)
			inputL += skip;
			inputR += skip;
			outputL += skip;
			outputR += skip;
		}
	}
	void processmixMono(float *inputL, long numsamples)
	{
		float outL, input;

		while (numsamples-- > 0)
		{
			outL = 0;
			input = (*inputL) * gain;

			// Accumulate comb filters in parallel
			for (int i = 0; i < numcombs; i++)
			{
				outL += combL[i].process(input);
			}

			// Feed through allpasses in series
			for (int i = 0; i < numallpasses; i++)
			{
				outL = allpassL[i].process(outL);
			}

			// Calculate output MIXING with anything already there
			*inputL += outL * wet1 + *inputL * dry;

			// Increment sample pointers, allowing for interleave (if any)
			inputL += 1;
		}
	}

	void processInPlaceStereoInterleaved(float *data, long numFrames) {
		float outL, outR, ins;

		for (int i = 0; i < numFrames; i++)
		{
			wet1 = wet1Target * 0.05 + wet1 * 0.95;
			wet2 = wet2Target * 0.05 + wet2 * 0.95;
			dry = dryTarget * 0.05 + dry * 0.95;

			outL = outR = 0;
			ins = (data[i * 2] + data[i * 2 + 1]) * gain;

			// Accumulate comb filters in parallel
			for (int i = 0; i < numcombs; i++)
			{
				outL += combL[i].process(ins);
				outR += combR[i].process(ins);
			}

			// Feed through allpasses in series
			for (int i = 0; i < numallpasses; i++)
			{
				outL = allpassL[i].process(outL);
				outR = allpassR[i].process(outR);
			}

			// Calculate output MIXING with anything already there
			data[i * 2] = outL * wet1 + outR * wet2 + data[i * 2] * dry;
			data[i * 2 + 1] = outR * wet1 + outL * wet2 + data[i * 2 + 1] * dry;

		}
	}


	void	processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip)
	{
		float outL, outR, input;

		while (numsamples-- > 0)
		{
			outL = outR = 0;
			input = (*inputL + *inputR) * gain;

			// Accumulate comb filters in parallel
			for (int i = 0; i < numcombs; i++)
			{
				outL += combL[i].process(input);
				outR += combR[i].process(input);
			}

			// Feed through allpasses in series
			for (int i = 0; i < numallpasses; i++)
			{
				outL = allpassL[i].process(outL);
				outR = allpassR[i].process(outR);
			}

			// Calculate output REPLACING anything already there
			*outputL = outL * wet1 + outR * wet2 + *inputL * dry;
			*outputR = outR * wet1 + outL * wet2 + *inputR * dry;

			// Increment sample pointers, allowing for interleave (if any)
			inputL += skip;
			inputR += skip;
			outputL += skip;
			outputR += skip;
		}
	}
	void	setroomsize(float value)
	{
		roomsize = (value * scaleroom) + offsetroom;
		update();
	}
	float	getroomsize()
	{
		return (roomsize - offsetroom) / scaleroom;
	}
	void	setdamp(float value)
	{
		damp = value * scaledamp;
		update();
	}
	float	getdamp()
	{
		return damp / scaledamp;
	}
	void	setwet(float value)
	{
		wet = value * scalewet;
		update();
	}
	float	getwet()
	{
		return wet / scalewet;
	}
	void	setdry(float value)
	{
		dryTarget = value * scaledry;
	}
	float	getdry()
	{
		return dry / scaledry;
	}
	void	setwidth(float value)
	{
		width = value;
		update();
	}
	float	getwidth()
	{
		return width;
	}
	void	setmode(float value)
	{
		mode = value;
		update();
	}
	float	getmode()
	{
		if (mode >= freezemode)
			return 1;
		else
			return 0;
	}
private:
	void	update()
	{
// Recalculate internal values after parameter change

		int i;

		wet1Target = wet * (width / 2 + 0.5f);
		wet2Target = wet * ((1 - width) / 2);

		if (mode >= freezemode)
		{
			roomsize1 = 1;
			damp1 = 0;
			gain = muted;
		}
		else
		{
			roomsize1 = roomsize;
			damp1 = damp;
			gain = fixedgain;
		}

		for (i = 0; i < numcombs; i++)
		{
			combL[i].setfeedback(roomsize1);
			combR[i].setfeedback(roomsize1);
		}

		for (i = 0; i < numcombs; i++)
		{
			combL[i].setdamp(damp1);
			combR[i].setdamp(damp1);
		}
	}
private:
	float	gain;
	float	roomsize, roomsize1;
	float	damp, damp1;
	float	wet, wet1, wet2;
	float	dry;
	float	width;
	float	mode;

	// The following are all declared inline
	// to remove the need for dynamic allocation
	// with its subsequent error-checking messiness

	// Comb filters
	comb	combL[numcombs];
	comb	combR[numcombs];

	// Allpass filters
	allpass	allpassL[numallpasses];
	allpass	allpassR[numallpasses];

	// Buffers for the combs
	float	bufcombL1[combtuningL1];
	float	bufcombR1[combtuningR1];
	float	bufcombL2[combtuningL2];
	float	bufcombR2[combtuningR2];
	float	bufcombL3[combtuningL3];
	float	bufcombR3[combtuningR3];
	float	bufcombL4[combtuningL4];
	float	bufcombR4[combtuningR4];
	float	bufcombL5[combtuningL5];
	float	bufcombR5[combtuningR5];
	float	bufcombL6[combtuningL6];
	float	bufcombR6[combtuningR6];
	float	bufcombL7[combtuningL7];
	float	bufcombR7[combtuningR7];
	float	bufcombL8[combtuningL8];
	float	bufcombR8[combtuningR8];

	// Buffers for the allpasses
	float	bufallpassL1[allpasstuningL1];
	float	bufallpassR1[allpasstuningR1];
	float	bufallpassL2[allpasstuningL2];
	float	bufallpassR2[allpasstuningR2];
	float	bufallpassL3[allpasstuningL3];
	float	bufallpassR3[allpasstuningR3];
	float	bufallpassL4[allpasstuningL4];
	float	bufallpassR4[allpasstuningR4];
	float wet1Target = 0;
	float wet2Target = 0;
	float dryTarget = 0;
};


#pragma once

#include "WaveFile.h"
#include "WaveOut.h"

#include <thread>

class MusicPlayer
{
public:
	MusicPlayer(std::string filename);
	~MusicPlayer();

	void Play();

	void Stop();

	void SetVolume(int volume=100);
	int GetVolume();

private:
	WaveFile waveBGM;
	WaveOut waveOut;
	bool doPlay;
	std::thread playThread;

	void PlayLoop();
};


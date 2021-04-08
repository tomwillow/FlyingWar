#include "MusicPlayer.h"

using namespace std;

MusicPlayer::MusicPlayer(std::string filename):
waveBGM(filename), waveOut(waveBGM.GetHeader(),5000),doPlay(true)
{
	//waveOut.Start();
}

MusicPlayer::~MusicPlayer()
{
	Stop();
}

void MusicPlayer::Play()
{
	doPlay = true;
	waveOut.Start();
	playThread = thread(&MusicPlayer::PlayLoop, this);
}

void MusicPlayer::Stop()
{
	doPlay = false;
	waveOut.Stop();
	if (playThread.joinable())
		playThread.join();
}

void MusicPlayer::SetVolume(int volume)
{
	waveOut.SetVolume(volume);
}

int MusicPlayer::GetVolume()
{
	return waveOut.GetVolume();
}

void MusicPlayer::PlayLoop()
{
	int bufsize = waveOut.GetBufSize();

	char* pos = waveBGM.GetData();
	while (doPlay)
	{
		int remain = waveBGM.GetDataSize() - (pos - waveBGM.GetData());

		waveOut.PlayAudio(pos, min(bufsize,remain));

		pos += bufsize;

		if (remain < bufsize)
			pos = waveBGM.GetData();
	}
}

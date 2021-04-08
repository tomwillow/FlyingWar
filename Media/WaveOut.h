#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <thread>

class WaveOut
{
public:
	WaveOut(PWAVEFORMATEX pWaveformat,int buf_ms=80);
	~WaveOut();

	//open & prepareHeader
	void Start();

	//填入数据，若当前没有空余缓冲区，则先阻塞，填充后返回
	//write
	void PlayAudio(char* buf, unsigned int nSize);

	//reset -> 等待线程播放完成并退出 -> unprepare -> close
	void Stop();

	void SetVolume(int volume=100);
	int GetVolume();

	unsigned int GetBufSize();
private:
	char* buf1,*buf2;
	unsigned int buf_size;
	bool isplaying1, isplaying2;
	bool is_on_stopping;
	HANDLE		m_hThread;
	DWORD		m_ThreadID;
	BOOL		m_bDevOpen;
	HWAVEOUT	m_hWaveOut;
	int			m_BufferQueue;
	WAVEFORMATEX m_Waveformat;
	WAVEHDR wavehdr1,wavehdr2;
	CRITICAL_SECTION m_Lock;

	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	void StartThread();
	void StopThread();
	void Open();

	//unprepare & close
	void Close();

	//若传入的指针指向wavehdr1，则将isplaying1设为false
	inline void SetFinishSymbol(PWAVEHDR pWaveHdr);
};


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

	//�������ݣ�����ǰû�п��໺�������������������󷵻�
	//write
	void PlayAudio(char* buf, unsigned int nSize);

	//reset -> �ȴ��̲߳�����ɲ��˳� -> unprepare -> close
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

	//�������ָ��ָ��wavehdr1����isplaying1��Ϊfalse
	inline void SetFinishSymbol(PWAVEHDR pWaveHdr);
};


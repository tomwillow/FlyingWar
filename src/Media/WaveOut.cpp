#include "WaveOut.h"

#include "tstring.h"

#pragma comment(lib,"winmm.lib")

//#define _PRINT

#ifdef _PRINT
#include <iostream>
#endif

#include <stdexcept>

using namespace std;

WaveOut::WaveOut(PWAVEFORMATEX pWaveformat, int buf_ms) :
	m_hThread(0), m_ThreadID(0), m_bDevOpen(false), m_hWaveOut(0), m_BufferQueue(0), isplaying1(false), isplaying2(false),is_on_stopping(false)
{
	//��ʼ����Ƶ��ʽ
	memcpy(&m_Waveformat, pWaveformat, sizeof(WAVEFORMATEX));
	m_Waveformat.nBlockAlign = (m_Waveformat.wBitsPerSample * m_Waveformat.nChannels) >> 3;
	m_Waveformat.nAvgBytesPerSec = m_Waveformat.nBlockAlign * m_Waveformat.nSamplesPerSec;

	//���仺����
	buf_size = buf_ms * m_Waveformat.nSamplesPerSec * m_Waveformat.nBlockAlign / 1000;
	buf1 = new char[buf_size];

	buf2 = new char[buf_size];

	//���WAVEHDR
	ZeroMemory(&wavehdr1, sizeof(WAVEHDR));
	ZeroMemory(&wavehdr2, sizeof(WAVEHDR));

	//����WAVEHDR
	wavehdr1.lpData = buf1;
	wavehdr1.dwBufferLength = buf_size;

	wavehdr2.lpData = buf2;
	wavehdr2.dwBufferLength = buf_size;

	InitializeCriticalSection(&m_Lock);

}

WaveOut::~WaveOut()
{
	StopThread();
	Close();
	delete[] buf1;
	delete[] buf2;
	DeleteCriticalSection(&m_Lock);
}

void WaveOut::Start()
{
	StartThread();
	try
	{
		Open();
	}
	catch (runtime_error e)
	{
		StopThread();
		throw e;
	}
}

void WaveOut::PlayAudio(char* in_buf, unsigned int in_size)
{
	if (!m_bDevOpen)
	{
		throw runtime_error("waveOut has not been opened");
	}

	//�ȴ����ֿ�д�뻺��
	while (1)
	{
		if (isplaying1 && isplaying2)//������д�룬�����ȴ�
		{
			Sleep(10);
#ifdef _PRINT
			//printf("PlayAudio::waitting\n");
#endif
			continue;
		}
		else
		{
			//һ����������һ����д�룬��break
#ifdef _PRINT
			//printf("PlayAudio::break\n");
#endif
			break;
		}
	}

	//��û���ڲ��ŵ�hdr��Ϊ��ǰhdr
	char* now_buf=nullptr;
	WAVEHDR* now_wavehdr = nullptr;
	bool* now_playing = nullptr;
	if (isplaying1 == false)
	{
		now_buf = buf1;
		now_wavehdr = &wavehdr1;
		now_playing = &isplaying1;
	}

	if (isplaying2 == false)
	{
		now_buf = buf2;
		now_wavehdr = &wavehdr2;
		now_playing = &isplaying2;
	}

	if (in_size > buf_size)//����buf�������û��������׳��쳣
	{
		throw runtime_error("input buffer size is bigger than self");
	}

	if (in_size <= buf_size)
	{
		now_wavehdr->dwBufferLength = in_size;
	}

	memcpy(now_buf, in_buf, in_size);


	if (is_on_stopping==false && waveOutWrite(m_hWaveOut, now_wavehdr, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
#ifdef _PRINT
		cout << "waveOutWrite fail" << endl;
#endif
		throw runtime_error("waveOutWrite fail");
	}

	EnterCriticalSection(&m_Lock);
	*now_playing = true;
	LeaveCriticalSection(&m_Lock);


}

DWORD __stdcall WaveOut::ThreadProc(LPVOID lpParameter)
{
	WaveOut* pWaveOut = (WaveOut*)lpParameter;
#ifdef _PRINT
	printf("ThreadProc::enter\n");
	cout << "pWaveOut=" << pWaveOut << endl;
#endif

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
		switch (msg.message)
		{
		case WOM_OPEN:
			break;
		case WOM_CLOSE:
			break;
		case WOM_DONE:
			//�����ɷ���
			WAVEHDR* pWaveHdr = (WAVEHDR*)msg.lParam;
			pWaveOut->SetFinishSymbol(pWaveHdr);
			break;
		}
	}
#ifdef _PRINT
	printf("ThreadProc::exit\n");
#endif
	return msg.wParam;
}

void WaveOut::StartThread()
{
	if (m_hThread)
	{
		throw runtime_error("thread has been running");
	}

	m_hThread = CreateThread(0, 0, ThreadProc, this, 0, &m_ThreadID);

	if (!m_hThread)
	{
		throw runtime_error("CreateThread fail");
	}
}

void WaveOut::StopThread()
{
	if (!m_hThread)
	{
		return;
	}

	if (m_hThread)
	{
		PostThreadMessage(m_ThreadID, WM_QUIT, 0, 0);

		while (1)
		{
			DWORD ExitCode;
			GetExitCodeThread(m_hThread, &ExitCode);
			if (ExitCode != STILL_ACTIVE)
			{
				break;
			}
			else
				Sleep(1);
		}

		//TerminateThread(m_hThread, 0);
		m_hThread = 0;
	}
}

void WaveOut::Open()
{
	if (m_bDevOpen)
	{
		throw runtime_error("waveOut has been opened");
	}

	//lphWaveOut: PHWaveOut;   {���ڷ����豸�����ָ��; ��� dwFlags=WAVE_FORMAT_QUERY, ����Ӧ�� nil}
	//uDeviceID: UINT;         {�豸ID; ����ָ��Ϊ: WAVE_MAPPER, ������������ݸ����Ĳ��θ�ʽѡ����ʵ��豸}
	//lpFormat: PWaveFormatEx; {TWaveFormat �ṹ��ָ��; TWaveFormat ����Ҫ����Ĳ��θ�ʽ}
	//dwCallback: DWORD        {�ص�������ַ�򴰿ھ��; ����ʹ�ûص�����, ��Ϊ nil}
	//dwInstance: DWORD        {���ص�������ʵ������; �����ڴ���}
	//dwFlags: DWORD           {��ѡ��}// long120823
	MMRESULT mRet;
	mRet = waveOutOpen(0, WAVE_MAPPER, &m_Waveformat, 0, 0, WAVE_FORMAT_QUERY);
	if (mRet != MMSYSERR_NOERROR)
	{
		throw runtime_error("waveOutOpen fail");
	}

	mRet = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_Waveformat, m_ThreadID, 0, CALLBACK_THREAD);
	if (mRet != MMSYSERR_NOERROR)
	{
		throw runtime_error("waveOutOpen fail");
	}

	if (waveOutPrepareHeader(m_hWaveOut, &wavehdr1, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		throw runtime_error("waveOutPrepareHeader fail");
	}

	if (waveOutPrepareHeader(m_hWaveOut, &wavehdr2, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
	{
		throw runtime_error("waveOutPrepareHeader fail");
	}

	m_bDevOpen = TRUE;
	is_on_stopping = false;
	isplaying1 = false;
	isplaying2 = false;
}

void WaveOut::Close()
{
	if (!m_bDevOpen)
	{
		return;
	}

	if (!m_hWaveOut)
	{
		return;
	}

	MMRESULT mRet;
	if ((mRet = waveOutUnprepareHeader(m_hWaveOut, &wavehdr1, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
	{
		TCHAR info[260];
		waveOutGetErrorText(mRet, info, 260);
		throw runtime_error(tto_string(info));
	}

	if ((mRet = waveOutUnprepareHeader(m_hWaveOut, &wavehdr2, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
	{
		TCHAR info[260];
		waveOutGetErrorText(mRet, info, 260);
		throw runtime_error(tto_string(info));
	}

	mRet = waveOutClose(m_hWaveOut);
	if (mRet != MMSYSERR_NOERROR)
	{
		throw runtime_error("waveOutClose fail");
	}
	m_hWaveOut = 0;
	m_bDevOpen = FALSE;
}

void WaveOut::Stop()
{
	if (m_hWaveOut == 0)
		return;

	is_on_stopping = true;

	//��reset
	MMRESULT mRet;
	if ((mRet = waveOutReset(m_hWaveOut)) != MMSYSERR_NOERROR)
	{
		TCHAR info[260];
		waveOutGetErrorText(mRet, info, 260);
		throw runtime_error(to_string(info));
	}

#ifdef _PRINT
	printf("waveOutReset succeed.\n");
#endif

	//���̷߳��͹ر��źţ�����ֱ���߳��˳�
	StopThread();

	Close();
}

void WaveOut::SetVolume(int volume)
{
	WORD single_volume = volume*0xFFFF/100;
	DWORD dwVolume = single_volume;
	dwVolume <<= 16;
	dwVolume |= single_volume;

	MMRESULT mRet;
	if ((mRet = waveOutSetVolume(m_hWaveOut, dwVolume)) != MMSYSERR_NOERROR)
	{
		TCHAR info[260];
		waveOutGetErrorText(mRet, info, 260);
		throw runtime_error(to_string(info));
	}
}

int WaveOut::GetVolume()
{
	DWORD dwVolume;
	waveOutGetVolume(m_hWaveOut, &dwVolume);

	return (dwVolume>>16) *100/0xFFFF;
}

unsigned int WaveOut::GetBufSize()
{
	return buf_size;
}

inline void WaveOut::SetFinishSymbol(PWAVEHDR pWaveHdr)
{
	EnterCriticalSection(&m_Lock);
	if (pWaveHdr == &wavehdr1)
	{
		isplaying1 = false;
#ifdef _PRINT
		printf("1 is finished.\n");
#endif
	}
	else
	{
		isplaying2 = false;
#ifdef _PRINT
		printf("2 is finished.\n");
#endif
	}
	LeaveCriticalSection(&m_Lock);
}
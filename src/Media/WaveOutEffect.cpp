#include "WaveOutEffect.h"

#include "tstring.h"
#include "WaveFile.h"

#pragma comment(lib,"winmm.lib")

//#define _PRINT

#ifdef _PRINT
#include <chrono>
#endif

#include <stdexcept>

using namespace std;

WaveOutEffect::WaveOutEffect(PWAVEFORMATEX pWaveformat) :m_hThread(0), m_ThreadID(0)
{
	//初始化音频格式
	memcpy(&m_Waveformat, pWaveformat, sizeof(WAVEFORMATEX));
	m_Waveformat.nBlockAlign = (m_Waveformat.wBitsPerSample * m_Waveformat.nChannels) >> 3;
	m_Waveformat.nAvgBytesPerSec = m_Waveformat.nBlockAlign * m_Waveformat.nSamplesPerSec;

}

WaveOutEffect::WaveOutEffect(int in_rate, int in_channels, int in_bits) :m_hThread(0), m_ThreadID(0)
{
	m_Waveformat.wFormatTag = WAVE_FORMAT_PCM;
	m_Waveformat.nChannels = in_channels;
	m_Waveformat.nSamplesPerSec = in_rate;
	m_Waveformat.wBitsPerSample = in_bits;
	m_Waveformat.nAvgBytesPerSec = m_Waveformat.nChannels * m_Waveformat.nSamplesPerSec * m_Waveformat.wBitsPerSample / 8;
	m_Waveformat.nBlockAlign = m_Waveformat.nChannels * m_Waveformat.wBitsPerSample / 8;
	m_Waveformat.cbSize = 0;
}

WaveOutEffect::~WaveOutEffect()
{
	StopPlay();
	StopThread();

	for (auto& pr : effects)
	{
		delete[] pr.second.buf;
	}
}

void WaveOutEffect::AddFile(int id, std::string filename)
{
	WaveFile wav(filename);
	wav.ConvertTo(m_Waveformat);

	MemoryBlock block;
	block.buf = new char[wav.GetDataSize()];
	block.buf_size = wav.GetDataSize();
	memcpy(block.buf, wav.GetData(), block.buf_size);

	effects[id] = block;

	if (!m_hThread)
		StartThread();
}

void WaveOutEffect::Play(int id)
{
#ifdef _PRINT
	auto t0 = chrono::high_resolution_clock::now();
#endif
	MMRESULT mRet;
	MemoryBlock& block = effects.find(id)->second;

	PWAVEHDR pwavehdr = new WAVEHDR;

	//清空WAVEHDR
	ZeroMemory(pwavehdr, sizeof(WAVEHDR));

	//设置WAVEHDR
	pwavehdr->lpData = new char[block.buf_size];
	memcpy(pwavehdr->lpData, block.buf, block.buf_size);

	pwavehdr->dwBufferLength = block.buf_size;

	unique_lock<mutex> ul(m);
	for (auto waveout : finished)
	{
		mRet = waveOutPrepareHeader(waveout, pwavehdr, sizeof(WAVEHDR));
#ifdef _DEBUG
		if (mRet != MMSYSERR_NOERROR)
		{
			TCHAR info[260];
			waveOutGetErrorText(mRet, info, 260);
			throw runtime_error(to_string(info));
		}
#endif

		MMRESULT mRet = waveOutWrite(waveout, pwavehdr, sizeof(WAVEHDR));
		if (mRet != MMSYSERR_NOERROR)
		{
			if (mRet == 7)
			{
#ifdef _PRINT
				cout << "waveOutWrite fail: no enough memory" << endl;
#endif
				//设备内存不足直接中止操作
				delete[] pwavehdr->lpData;
				delete pwavehdr;
				return;
			}
#ifdef _DEBUG
			TCHAR info[260];
			waveOutGetErrorText(mRet, info, 260);
			throw runtime_error(to_string(info));
#endif
		}

		playing.insert(waveout);
		finished.erase(waveout);

#ifdef _PRINT
		cout << "Old device=" << waveout << endl;
		cout << "playing device num=" << playing.size() << endl;
		cout << "elapsed = " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now()-t0).count() << endl;
#endif

		return;
	}
	ul.unlock();

	HWAVEOUT waveout=0;

	mRet = waveOutOpen(&waveout, WAVE_MAPPER, &m_Waveformat, m_ThreadID, 0, CALLBACK_THREAD);
#ifdef _DEBUG
	if (mRet != MMSYSERR_NOERROR)
	{
		throw runtime_error("waveOutOpen fail");
	}
#endif

	mRet = waveOutPrepareHeader(waveout, pwavehdr, sizeof(WAVEHDR));
#ifdef _DEBUG
	if (mRet != MMSYSERR_NOERROR)
	{
		TCHAR info[260];
		waveOutGetErrorText(mRet, info, 260);
		throw runtime_error(to_string(info));
	}
#endif

	mRet = waveOutWrite(waveout, pwavehdr, sizeof(WAVEHDR));
	if (mRet != MMSYSERR_NOERROR)
	{
		if (mRet == 7)
		{
#ifdef _PRINT
			cout << "waveOutWrite fail: no enough memory" << endl;
#endif
			//设备内存不足直接中止操作
			delete[] pwavehdr->lpData;
			delete pwavehdr;
			return;
		}
#ifdef _DEBUG
		TCHAR info[260];
		waveOutGetErrorText(mRet, info, 260);
		throw runtime_error(to_string(info));
#endif
	}

	m.lock();
	playing.insert(waveout);
#ifdef _PRINT
	cout << "New device=" << waveout << endl;
	cout << "playing device num=" << playing.size() << endl;
	cout << "elapsed = " << chrono::duration_cast<chrono::milliseconds>(chrono::high_resolution_clock::now() - t0).count() << endl;
#endif
	m.unlock();
}

void WaveOutEffect::PrepareDevice(int num)
{
	for (int i = 0; i < num; ++i)
	{
		MMRESULT mRet;
		HWAVEOUT waveout = 0;

		mRet = waveOutOpen(&waveout, WAVE_MAPPER, &m_Waveformat, m_ThreadID, 0, CALLBACK_THREAD);
#ifdef _DEBUG
		if (mRet != MMSYSERR_NOERROR)
		{
			throw runtime_error("waveOutOpen fail");
		}
#endif

		//填入空数据，使设备打开后迅速播放完成
		PWAVEHDR pwavehdr = new WAVEHDR;
		ZeroMemory(pwavehdr, sizeof(WAVEHDR));
		pwavehdr->lpData = 0;
		pwavehdr->dwBufferLength = 0;

		mRet = waveOutPrepareHeader(waveout, pwavehdr, sizeof(WAVEHDR));
#ifdef _DEBUG
		if (mRet != MMSYSERR_NOERROR)
		{
			TCHAR info[260];
			waveOutGetErrorText(mRet, info, 260);
			throw runtime_error(to_string(info));
		}
#endif

		mRet = waveOutWrite(waveout, pwavehdr, sizeof(WAVEHDR));
		if (mRet != MMSYSERR_NOERROR)
		{
			if (mRet == 7)
			{
#ifdef _PRINT
				cout << "waveOutWrite fail: no enough memory" << endl;
#endif
				//设备内存不足直接中止操作
				delete[] pwavehdr->lpData;
				delete pwavehdr;
				return;
			}
#ifdef _DEBUG
			TCHAR info[260];
			waveOutGetErrorText(mRet, info, 260);
			throw runtime_error(to_string(info));
#endif
		}

		m.lock();
		playing.insert(waveout);
#ifdef _PRINT
		cout << "New device=" << waveout << endl;
		cout << "playing device num=" << playing.size() << endl;
#endif
		m.unlock();
	}
}

void WaveOutEffect::StopPlay()
{
	MMRESULT mRet;
	for (auto& waveout : playing)
	{
		//正在播放的设备需要Reset
		mRet = waveOutReset(waveout);
#ifdef _DEBUG
		if (mRet != MMSYSERR_NOERROR)
		{
			TCHAR info[260];
			waveOutGetErrorText(mRet, info, 260);
			throw runtime_error(to_string(info));
		}
#endif
	}
	for (auto& waveout : finished)
	{
		//播放完成或者reset的设备直接close关闭
		mRet = waveOutClose(waveout);
#ifdef _DEBUG
		if (mRet != MMSYSERR_NOERROR)
		{
			throw runtime_error("waveOutClose fail");
		}
#endif
	}
}

void WaveOutEffect::SetDeviceState(HWAVEOUT waveout)
{
	m.lock();
	auto it = playing.find(waveout);
	finished.insert(*it);
	playing.erase(it);
	m.unlock();
}

DWORD __stdcall WaveOutEffect::ThreadProc(LPVOID lpParameter)
{
	WaveOutEffect* pWaveOutEffect = (WaveOutEffect*)lpParameter;
#ifdef _PRINT
	printf("ThreadProc::enter\n");
	cout << "pWaveOut=" << pWaveOutEffect << endl;
#endif

	MSG msg;
	while (GetMessage(&msg, 0, 0, 0))
	{
			HWAVEOUT waveout = (HWAVEOUT)msg.wParam;
			WAVEHDR* pwavehdr = (WAVEHDR*)msg.lParam;
		switch (msg.message)
		{
		case WOM_OPEN:
#ifdef _PRINT
			cout << waveout << " WOM_OPEN" << endl;
#endif
			break;
		case WOM_CLOSE:
#ifdef _PRINT
			cout << waveout << " WOM_CLOSE" << endl;
#endif
			break;
		case WOM_DONE:
#ifdef _PRINT
			cout << waveout << " WOM_DONE" << endl;
#endif
			MMRESULT mRet = waveOutUnprepareHeader(waveout, pwavehdr, sizeof(WAVEHDR));
#ifdef _DEBUG
			if (mRet != MMSYSERR_NOERROR)
			{
				TCHAR info[260];
				waveOutGetErrorText(mRet, info, 260);
				throw runtime_error(to_string(info));
			}
#endif

			delete[] pwavehdr->lpData;
			delete pwavehdr;

			//将设备从playing移动到finished
			pWaveOutEffect->SetDeviceState(waveout);
			break;
		}
	}
#ifdef _PRINT
	printf("ThreadProc::exit\n");
#endif
	return msg.wParam;
}

void WaveOutEffect::StartThread()
{
#ifdef _DEBUG
	if (m_hThread)
	{
		throw runtime_error("thread has been running");
	}
#endif

	m_hThread = CreateThread(0, 0, ThreadProc, this, 0, &m_ThreadID);

#ifdef _DEBUG
	if (!m_hThread)
	{
		throw runtime_error("CreateThread fail");
	}
#endif
}

void WaveOutEffect::StopThread()
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

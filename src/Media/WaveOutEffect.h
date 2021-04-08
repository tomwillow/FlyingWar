#pragma once

#include <Windows.h>
#include <mmsystem.h>
#include <unordered_map>
#include <unordered_set>

#include <mutex>

class WaveOutEffect
{
public:
	WaveOutEffect(PWAVEFORMATEX pWaveformat);
	WaveOutEffect(int in_rate = 44100, int in_channels = 2, int in_bits = 16);
	~WaveOutEffect();

	//添加音效文件，若文件header不匹配格式，会自动进行转换
	//exception:	若内存不足将抛异常
	void AddFile(int id, std::string filename);

	//播放对应id的音效
	//exception:	若对应id没有加载，由stl抛出异常
	//				若内存不足将抛异常
	void Play(int id);

	//预打开设备，防止第一次播放时主线程卡顿
	void PrepareDevice(int num);

	//停止所有声音，不关闭监听线程
	void StopPlay();

private:
	struct MemoryBlock
	{
		char* buf;
		int buf_size;
	};

	std::unordered_map<int, MemoryBlock> effects;//音效data

	WAVEFORMATEX m_Waveformat;//主格式

	std::unordered_set<HWAVEOUT> playing,finished;

	//将设备从playing集合转移至finished集合
	//会获取锁
	void SetDeviceState(HWAVEOUT waveout);

	std::mutex m;
	HANDLE m_hThread;
	DWORD m_ThreadID;

	//监听线程
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	void StartThread();
	void StopThread();
};


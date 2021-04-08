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

	//�����Ч�ļ������ļ�header��ƥ���ʽ�����Զ�����ת��
	//exception:	���ڴ治�㽫���쳣
	void AddFile(int id, std::string filename);

	//���Ŷ�Ӧid����Ч
	//exception:	����Ӧidû�м��أ���stl�׳��쳣
	//				���ڴ治�㽫���쳣
	void Play(int id);

	//Ԥ���豸����ֹ��һ�β���ʱ���߳̿���
	void PrepareDevice(int num);

	//ֹͣ�������������رռ����߳�
	void StopPlay();

private:
	struct MemoryBlock
	{
		char* buf;
		int buf_size;
	};

	std::unordered_map<int, MemoryBlock> effects;//��Чdata

	WAVEFORMATEX m_Waveformat;//����ʽ

	std::unordered_set<HWAVEOUT> playing,finished;

	//���豸��playing����ת����finished����
	//���ȡ��
	void SetDeviceState(HWAVEOUT waveout);

	std::mutex m;
	HANDLE m_hThread;
	DWORD m_ThreadID;

	//�����߳�
	static DWORD WINAPI ThreadProc(LPVOID lpParameter);
	void StartThread();
	void StopThread();
};


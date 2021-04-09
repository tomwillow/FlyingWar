#include "WaveFile.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <fstream>
#include <stdexcept>
#include <vector>
#include <cstring>
#include <climits>

using namespace std;

WaveFile::WaveFile(int in_rate, int in_channels, int in_bits) :data(nullptr), data_size(0), sample_count(0)
{
	header.wFormatTag = WAVE_FORMAT_PCM;
	header.nChannels = in_channels;
	header.nSamplesPerSec = in_rate;
	header.wBitsPerSample = in_bits;
	header.nAvgBytesPerSec = header.nChannels * header.nSamplesPerSec * header.wBitsPerSample / 8;
	header.nBlockAlign = header.nChannels * header.wBitsPerSample / 8;
	header.cbSize = 0;
}

WaveFile::WaveFile(std::string filename) :data(nullptr), data_size(0), sample_count(0)
{
	ifstream ifs(filename, ios::out | ios::binary);
	if (!ifs.is_open())
		throw runtime_error("Can not open file:" + filename);

	unsigned int file_size = 0;
	while (!ifs.eof())
	{
		char sym[5];
		ifs.read(sym, 4); sym[4] = 0;
		uint32_t sz;
		ifs.read((char*)&sz, 4);

		unsigned int remain_size = file_size - ifs.tellg();
		if (sz > remain_size)
		{
			WaveFile::~WaveFile();
			throw runtime_error("Wrong block size, pos=" + to_string(ifs.tellg()));
		}

		if (string(sym) == "RIFF")
		{
			ifs.read(sym, 4); sym[4] = 0;
			file_size = sz + 8;
			if (string(sym) != "WAVE")
			{
				WaveFile::~WaveFile();
				throw runtime_error("[0x08-0x0B] is not \"WAVE\"");
			}
			continue;
		}

		if (string(sym) == "fmt ")
		{
			if (sz == 16)
			{
				ifs.read((char*)&header, sz);
				header.cbSize = 0;
				continue;
			}
			if (sz == 18)
			{
				ifs.read((char*)&header, sz);
				continue;
			}

			if (header.wBitsPerSample != 8 && header.wBitsPerSample != 16)
			{
				WaveFile::~WaveFile();
				throw runtime_error("The bits per sample is not 8 or 16.");
			}

			if (header.nChannels != 1 && header.nChannels != 2)
			{
				WaveFile::~WaveFile();
				throw runtime_error("The number of channels is not 1 or 2.");

			}

			if (header.wFormatTag != WAVE_FORMAT_PCM)
			{
				WaveFile::~WaveFile();
				throw runtime_error("wFormatTag is not WAVE_FORMAT_PCM");
			}

			WaveFile::~WaveFile();
			throw runtime_error("fmt block's size is not 16 or 18");
		}

		if (string(sym) == "data")
		{
			data_size = sz;
			sample_count = data_size / (header.wBitsPerSample / 8);

			data = new char[data_size];
			ifs.read(data, data_size);

			remain_size = file_size - ifs.tellg();
			if (remain_size == 0)
				break;
			continue;
		}

		char* temp = new char[sz];
		ifs.read(temp, sz);
		Block block;
		memcpy(block.sym, sym, 4);
		block.data_size = sz;
		block.data = temp;
		otherBlocks.push_back(block);

		if (sz % 2)
			ifs.seekg(1, ios::cur);

		remain_size = file_size - ifs.tellg();
		if (remain_size == 0)
			break;
	}


	ifs.close();
}

WaveFile::WaveFile(const WaveFile& another)
{
	memcpy(&header, &another.header, sizeof(header));
	data_size = another.data_size;
	data = new char[data_size];
	memcpy(data, another.data, data_size);
	sample_count = another.sample_count;

	for (auto& block : another.otherBlocks)
	{
		Block my_block;
		memcpy(my_block.sym, block.sym, 4);
		my_block.data_size = block.data_size;
		my_block.data = new char[my_block.data_size];
		memcpy(my_block.data, block.data, my_block.data_size);
		otherBlocks.push_back(my_block);
	}
}

WaveFile::~WaveFile()
{
	if (data)
		delete[] data;
	for (auto& block : otherBlocks)
		delete[] block.data;
}

WAVEFORMATEX* WaveFile::GetHeader()
{
	return &header;
}

char* WaveFile::GetData()
{
	return data;
}

int WaveFile::GetDataSize()
{
	return data_size;
}

void WaveFile::Clear()
{
	if (data)
	{
		delete[] data;
		data = nullptr;
		data_size = 0;
		sample_count = 0;
	}
}

void WaveFile::Add(const WaveFile& another)
{
	WaveFile copy(another);
	if (header.nChannels != copy.header.nChannels)//channels not equal
	{
		if (copy.header.nChannels == 1)
			copy.CloneMonoToStereo();
		else
			copy.ConvertToMono();
	}

	if (header.nSamplesPerSec != copy.header.nSamplesPerSec)//sample rate not equal
	{
		copy.Resample(header.nSamplesPerSec);
	}

	//start combining
	int new_data_size = data_size + copy.data_size;
	char* new_data = new char[new_data_size];

	memcpy(new_data, data, data_size);
	memcpy(new_data + data_size, copy.data, copy.data_size);

	//update properties
	delete[] data;
	data = new_data;
	data_size = new_data_size;
	sample_count += copy.sample_count;
}

void WaveFile::Add(char* new_data, unsigned int new_data_size)
{
	if (new_data_size % header.nBlockAlign != 0)
		throw runtime_error("try to add not-aligned data");

	char* p = new char[data_size + new_data_size];
	memcpy(p, data, data_size);
	memcpy(p + data_size, new_data, new_data_size);
	delete[] data;
	data = p;
	data_size += new_data_size;
	sample_count += new_data_size / (header.wBitsPerSample / 8);
}

#ifdef _USE_COMPLEX
//if length is over data, the overflow part of Complex vector is 0.
std::vector<Complex> WaveFile::GetComplex(int channel, int group_start, int length)
{
	vector<Complex> cp(length);
	//double time_step = 1.0 / (double)header.nSamplesPerSec;
	int group_count = sample_count / header.nChannels;
	int avaliable_count = min(length, group_count - group_start);
	if (header.nChannels == 1 && channel == 0)
	{
		switch (header.wBitsPerSample)
		{
		case 8:
			for (int i = 0; i < avaliable_count; ++i)
			{
				cp[i] = Complex((unsigned char)data[group_start + i], 0);
			}
			break;
		case 16:
		{
			short* pMono = (short*)data;
			for (int i = 0; i < avaliable_count; ++i)
			{
				cp[i] = Complex(pMono[group_start + i], 0);
			}
			break;
		}
		}
	}

	if (header.nChannels == 2 && (channel == 0 || channel == 1))
	{
		switch (header.wBitsPerSample)
		{
		case 8:
			for (int i = 0; i < avaliable_count; ++i)
			{
				cp[i] = Complex((unsigned char)data[2 * (group_start + i) + channel], 0);
			}
			break;
		case 16:
			short* pData = (short*)data;
			for (int i = 0; i < avaliable_count; ++i)
			{
				cp[i] = Complex(pData[2 * (group_start + i) + channel], 0);//step*i/(double)SHRT_MAX
			}
			break;
		}
	}

	if (avaliable_count < length)
	{
		for (int i = avaliable_count; i < length; ++i)
		{
			cp[i] = { 0,0 };
		}
	}

	return cp;
}

void WaveFile::SetFromComplex(std::vector<Complex>& cp, int channel, int group_start, int length)
{
	if (header.nChannels == 1 && channel == 0)
	{
		switch (header.wBitsPerSample)
		{
		case 8:
			for (int i = 0; i < length; ++i)
			{
				data[i] = cp[group_start + i].r;
			}
			break;
		case 16:
		{
			short* pMono = (short*)data;
			for (int i = 0; i < length; ++i)
			{
				pMono[i] = cp[group_start + i].r;
			}
			break;
		}
		}
	}

	if (header.nChannels == 2 && (channel == 0 || channel == 1))
	{
		switch (header.wBitsPerSample)
		{
		case 8:
			for (int i = 0; i < length; ++i)
			{
				data[2 * (group_start + i) + channel] = cp[i].r;
			}
			break;
		case 16:
			short* pData = (short*)data;
			for (int i = 0; i < length; ++i)
			{
				pData[2 * (group_start + i) + channel] = cp[i].r;
			}
			break;
		}
	}
}

void WaveFile::Filter(bool isHighPass, double freq)
{
	int step = GetLowerAndNearest2Power(header.nSamplesPerSec);
	int group_count = sample_count / header.nChannels;

	int group_start = 0;
	//Mute();
	//AddSin(20, 0.5);
	//AddSin(1000, 0.5);
	while (1)
	{
		int avaliable_count = min(group_count - group_start, step);

		for (int ch = 0; ch < header.nChannels; ++ch)
		{
			vector<Complex> cp = GetComplex(ch, group_start, step);
			FFT(cp, step);
			//Abs(cp);
			//ComplexWriteToFile(cp, "cp.csv");

			if (isHighPass)
			{
				//high pass
				int start1 = 0;
				int end1 = freq / (double)header.nSamplesPerSec * step;
				int start2 = (header.nSamplesPerSec - freq) / (double)header.nSamplesPerSec * step;
				int end2 = cp.size();
				//clear left side
				for (int i = start1; i < end1; ++i)
				{
					cp[i].r = 0; cp[i].i = 0;
				}
				//clear right side
				for (int i = start2; i < end2; ++i)
				{
					cp[i].r = 0; cp[i].i = 0;
				}
			}
			else
			{
				//low pass
				int start = freq / (double)header.nSamplesPerSec * step;
				int end = (header.nSamplesPerSec - freq) / (double)header.nSamplesPerSec * step;
				for (int i = start; i < end; ++i)
				{
					cp[i].r *= 0; cp[i].i *= 0;
				}
			}

			iFFT(cp, step);

			SetFromComplex(cp, ch, group_start, avaliable_count);
		}

		group_start += step;
		if (group_start >= group_count)
			break;
	}
}
#endif

void WaveFile::SaveToFile(std::string newFileName)
{
	ofstream ofs(newFileName, ios::out | ios::binary);
	if (!ofs.is_open())
		throw runtime_error("Can not write file:" + newFileName);

	//RIFF sz
	unsigned int filesize = 0;
	filesize += 12;//RIFF sz WAVE
	filesize += 8 + sizeof(header);//fmt sz header
	filesize += 8 + data_size;//"data" sz data
	for (auto& block : otherBlocks)
	{
		filesize += 8 + block.data_size;
		if (block.data_size % 2)
			filesize += 1;
	}

	unsigned int riff_size = filesize - 8;
	ofs.write("RIFF", 4);
	ofs.write((char*)&riff_size, 4);

	//WAVE
	ofs.write("WAVE", 4);

	//fmt
	ofs.write("fmt ", 4);
	int header_size = sizeof(header);
	ofs.write((char*)&header_size, 4);
	ofs.write((char*)&header, sizeof(header));

	//data
	ofs.write("data", 4);
	ofs.write((char*)&data_size, 4);
	ofs.write(data, data_size);

	//other
	for (auto& block : otherBlocks)
	{
		ofs.write(block.sym, 4);
		ofs.write((char*)&block.data_size, 4);
		ofs.write(block.data, block.data_size);
		if (block.data_size % 2)
			ofs.write("", 1);
	}

	if (!ofs.good())
		throw runtime_error("Error occured when writting.");

	ofs.close();
}

void WaveFile::ConvertToMono()
{
	if (header.nChannels != 2)
		throw runtime_error("File is not a stereo file.");

	int mono_size = data_size / 2;
	char* mono = new char[mono_size];

	switch (header.wBitsPerSample)
	{
	case 8:
	{
		unsigned char* pData = (unsigned char*)mono;
		unsigned char* pSrc = (unsigned char*)data;
		for (int i = 0; i < sample_count / 2; ++i)
		{
			pData[i] = ((unsigned short)pSrc[2 * i] + (unsigned short)pSrc[2 * i + 1]) / 2;
		}
		break;
	}
	case 16:
	{
		short* pMono = (short*)mono;
		short* pStereo = (short*)data;
		for (int i = 0; i < sample_count / 2; ++i)
		{
			pMono[i] = ((int)pStereo[2 * i] + (int)pStereo[2 * i + 1]) / 2;
		}
		break;
	}
	default:
		throw runtime_error("The bits per sample is not 8 or 16.");
	}

	//update properties
	delete[] data;
	data = mono;
	data_size = mono_size;

	sample_count /= 2;
	header.nChannels = 1;
	header.nAvgBytesPerSec = header.nSamplesPerSec * (header.wBitsPerSample / 8) * header.nChannels;//eg. 44000*2*2
}

void WaveFile::CloneMonoToStereo()
{
	if (header.nChannels != 1)
		throw runtime_error("File is not a mono file.");

	//int step = header.wBitsPerSample / 8;

	int stereo_size = data_size * 2;
	char* stereo = new char[stereo_size];

	switch (header.wBitsPerSample)
	{
	case 8:
		for (int i = 0; i < sample_count; ++i)
		{
			stereo[2 * i] = data[i];
			stereo[2 * i + 1] = data[i];
		}
		break;
	case 16:
	{
		short* pMono = (short*)data;
		short* pStereo = (short*)stereo;
		for (int i = 0; i < sample_count; ++i)
		{
			pStereo[2 * i] = pMono[i];
			pStereo[2 * i + 1] = pMono[i];
		}
		break;
	}
	default:
		throw runtime_error("The bits per sample is not 8 or 16.");
	}

	//update properties
	delete[] data;
	data = stereo;
	data_size = stereo_size;

	sample_count *= 2;
	header.nChannels = 2;
	header.nAvgBytesPerSec = header.nSamplesPerSec * (header.wBitsPerSample / 8) * header.nChannels;//eg. 44000*2*2
}

void WaveFile::ConvertTo(WAVEFORMATEX fmt)
{
	if (fmt.nChannels != header.nChannels)
	{
		switch (fmt.nChannels)
		{
		case 1:
			ConvertToMono();
			break;
		case 2:
			CloneMonoToStereo();
			break;
		default:
			throw runtime_error("unsupported channels=" + to_string(fmt.nChannels));
		}
	}

	if (fmt.nSamplesPerSec != header.nSamplesPerSec)
	{
		Resample(fmt.nSamplesPerSec);
	}

	//to do: 8 to 16, 16 to 8
	if (fmt.wBitsPerSample != header.wBitsPerSample)
		throw runtime_error("unfinished");
}

void WaveFile::Resample(int new_nSamplesPerSec)
{
	double eq_speed = (double)header.nSamplesPerSec / (double)new_nSamplesPerSec;
	ChangeSpeed(eq_speed);

	header.nSamplesPerSec = new_nSamplesPerSec;
	header.nAvgBytesPerSec = header.nSamplesPerSec * (header.wBitsPerSample / 8) * header.nChannels;//eg. 44000*2*2
}

void WaveFile::Repeat(int times)
{
	if (times <= 0)
		throw runtime_error("Repeating times must be rather than 0.");

	if (times == 1)
		return;

	//calc
	int new_data_size = data_size * times;
	char* new_data = new char[new_data_size];

	//fill in
	for (int i = 0; i < times; ++i)
	{
		memcpy(new_data + data_size * i, data, data_size);
	}

	//update properties
	delete[] data;
	data = new_data;
	data_size = new_data_size;
}

void WaveFile::ChangeSpeed(double speed)
{
	if (speed < 0)
		throw runtime_error("Speed can not be negative.");

	//calc
	int new_data_size = (int)(data_size / speed);
	char* new_data = new char[new_data_size];
	sample_count = (int)(sample_count / speed);

	//stretch
	switch (header.wBitsPerSample)
	{
	case 8:
		switch (header.nChannels)
		{
		case 1:
			for (int i = 0; i < sample_count; ++i)
			{
				new_data[i] = data[(int)(i * speed)];
			}
			break;
		case 2:
			for (int i = 0; i < sample_count / 2; ++i)
			{
				new_data[2 * i] = data[(int)(2 * i * speed)];
				new_data[2 * i + 1] = data[(int)((2 * i + 1) * speed)];
			}
			break;
		default:
			throw runtime_error("The number of channels is not 1 or 2.");
		}
		break;
	case 16:
	{
		switch (header.nChannels)
		{
		case 1:
		{
			short* pDst = (short*)new_data;
			short* pSrc = (short*)data;
			for (int i = 0; i < sample_count; ++i)
			{
				pDst[i] = pSrc[(int)(i * speed)];
			}
			break;
		}
		case 2:
		{
			short* pDst = (short*)new_data;
			short* pSrc = (short*)data;
			for (int i = 0; i < sample_count / 2; ++i)
			{
				pDst[2 * i] = pSrc[(int)(2 * i * speed)];
				pDst[2 * i + 1] = pSrc[(int)((2 * i + 1) * speed)];
			}
			break;
		}
		default:
			throw runtime_error("The number of channels is not 1 or 2.");
		}
		break;
	}
	default:
		throw runtime_error("The bits per sample is not 8 or 16.");
	}

	//update properties
	delete[] data;
	data = new_data;
	data_size = new_data_size;
}

void WaveFile::Mute()
{
	memset(data, 0, data_size);
}

void WaveFile::AddSin(double freq, double amp, double phase_second)
{
	double step = 1.0 / (double)header.nSamplesPerSec;
	switch (header.nChannels)
	{
	case 1:
	{
		switch (header.wBitsPerSample)
		{
		case 8:
		{
			unsigned char* p8bits = (unsigned char*)data;
			for (int i = 0; i < sample_count; ++i)
			{
				p8bits[i] += (1.0 + sin(2.0 * M_PI * freq * (i * step - phase_second))) * 0.5 * amp * UCHAR_MAX;
			}
			break;
		}
		case 16:
		{
			short* pMono = (short*)data;
			for (int i = 0; i < sample_count; ++i)
			{
				pMono[i] += amp * SHRT_MAX * sin(2.0 * M_PI * freq * (i * step - phase_second));
			}
			break;
		}
		}
		break;
	}
	case 2:
		switch (header.wBitsPerSample)
		{
		case 8:
		{
			unsigned char* p8bits = (unsigned char*)data;
			for (int i = 0; i < sample_count / 2; ++i)
			{
				p8bits[2 * i] += (1.0 + sin(2.0 * M_PI * freq * (i * step - phase_second))) * 0.5 * amp * UCHAR_MAX;
				p8bits[2 * i + 1] += (1.0 + sin(2.0 * M_PI * freq * (i * step - phase_second))) * 0.5 * amp * UCHAR_MAX;
			}
			break;
		}
		case 16:
		{
			short* pData = (short*)data;
			for (int i = 0; i < sample_count / 2; ++i)
			{
				pData[2 * i] += amp * SHRT_MAX * sin(2.0 * M_PI * freq * (i * step - phase_second));
				pData[2 * i + 1] += amp * SHRT_MAX * sin(2.0 * M_PI * freq * (i * step - phase_second));
			}
			break;
		}
		}
	}
}

std::ostream& operator<<(std::ostream& out, const WaveFile& wav)
{
	out << "Number of channels: " << wav.header.nChannels << endl;
	out << "Sampling rate: " << wav.header.nSamplesPerSec << endl;
	out << "Samples per channel: " << wav.sample_count / wav.header.nChannels << endl;
	out << "Number of samples: " << wav.sample_count << endl;
	out << "The bits per sample: " << wav.header.wBitsPerSample << endl;
	return out;
}

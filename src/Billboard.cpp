#include "Billboard.h"

#include <fstream>
#include <sstream>

using namespace std;

Billboard::Billboard(std::string filename, int item_limit):
	filename(filename),item_limit(item_limit)
{
	LoadFromFile();
}

void Billboard::LoadFromFile()
{
	fstream in(filename,fstream::in| fstream::out| fstream::app);

	if (!in.is_open())
		throw runtime_error("failed to open: " + filename);

	string line;
	while (getline(in, line))
	{
		string name;
		int score;
		stringstream ss(line);
		ss >> name >> score;
		record.insert({ score,name });
	}

	in.close();

	if (record.size() > item_limit)
	{
		for (int i = 0; i < record.size() - item_limit; ++i)
		{
			record.erase(record.begin());
		}
	}
}

void Billboard::SaveToFile()
{
	ofstream out(filename);

	if (!out.is_open())
		throw runtime_error("failed to open: " + filename);

	for (auto& pr : record)
	{
		string& name = pr.second;
		int score = pr.first;
		out << name << " " << score << endl;
	}

	out.close();
}

bool Billboard::CanRefreshRecord(int new_score)
{
	if (record.size() < item_limit)
		return true;

	if (record.begin()->first < new_score)
		return true;

	return false;
}

void Billboard::RefreshRecord(std::string name, int new_score)
{
	if (record.size() >= item_limit)
	{
		record.erase(record.begin());
	}

	record.insert({ new_score,name });

	SaveToFile();
}

const std::multimap<int, std::string>& Billboard::GetRecord()
{
	return record;
}

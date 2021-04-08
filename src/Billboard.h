#pragma once

#include <string>
#include <map>

class Billboard
{
public:
	Billboard(std::string filename,int item_limit=5);

	void LoadFromFile();

	void SaveToFile();

	bool CanRefreshRecord(int new_score);

	void RefreshRecord(std::string name, int new_score);

	const std::multimap <int, std::string>& GetRecord();

private:
	std::string filename;
	std::multimap <int,std::string> record;
	const int item_limit;
};


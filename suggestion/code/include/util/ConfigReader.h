#ifndef CONFIGREADER_H_INCLUDED
#define CONFIGREADER_H_INCLUDED
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <fstream>

using namespace std;

class ConfigReader
{
public:
	ConfigReader(const string& file,map<string, string> &cmap)
	{
		ifstream in(file.c_str());
		string line;
		while (getline(in, line))
		{
			strip(line);
			if (line.empty() || line[0] == '#') continue;
			vector<string> kp;
			size_t pos = line.find(',');
			if (pos == size_t(-1))
			{
				continue;
			}
			string key = line.substr(0, pos);
			string value = line.substr(pos+1, line.size()-pos-1);
			strip(key);
			strip(value);
			cmap[key] = value;
		}
		in.close();
	}
private:
	bool isspace(char ch) const
	{
		return (ch == ' ' || ch == '\t');
	}
	void strip(string& s) const
	{
		size_t i, j;
		for (i=0; i<s.size() && isspace(s[i]); i++);
		for (j=s.size(); j>0 && isspace(s[j-1]); j--);
		s = s.substr(i, j-i);
	}

};

#endif // CONFIGREADER_H_INCLUDED

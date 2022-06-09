#include "strutil.h"


int countchar(std::string s, char c)
{
	int count = 0;
	for (int i=0; i<s.size(); i++) {
		if (s[i] == c) count++;
	}
	return count;
}

std::vector<std::string> split(std::string s, std::string delim)
{
	std::vector<std::string> v;
	if (s.find(delim) == std::string::npos) {
		v.push_back(s);
		return v;
	}
	size_t pos=0;
	size_t start;
	while (pos < s.length()) {
		start = pos;
		pos = s.find(delim,pos);
		if (pos == std::string::npos) {
			v.push_back(s.substr(start,s.length()-start));
			return v;
		}
		v.push_back(s.substr(start, pos-start));
		pos += delim.length();
	}
	return v;
}

std::vector<std::string> bifurcate(std::string strg, char c, bool fromback)
{
	std::vector<std::string> result;
	if (countchar(strg, c) == 0) {
		result.push_back(strg);
	}
	else {
		std::size_t eq;
		if (fromback)
			eq = strg.find_last_of(c);
		else
			eq = strg.find_first_of(c);
		result.push_back(strg.substr(0,eq));
		result.push_back(strg.substr(eq+1));
	}
	return result;
}


//https://stackoverflow.com/questions/5343190/how-do-i-replace-all-instances-of-a-string-with-another-string/
void replace_all(std::string& str, const std::string& from, const std::string& to) 
{
    if(from.empty())
        return;
    std::string wsRet;
    wsRet.reserve(str.length());
    size_t start_pos = 0, pos;
    while((pos = str.find(from, start_pos)) != std::string::npos) {
        wsRet += str.substr(start_pos, pos - start_pos);
        wsRet += to;
        pos += from.length();
        start_pos = pos;
    }
    wsRet += str.substr(start_pos);
    str.swap(wsRet); // faster than str = wsRet;
}

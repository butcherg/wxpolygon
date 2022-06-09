
/*
    myConfig
    
    Part of wxPolygon - Interactive tool to build polygons for OpenSCAD
    Copyright (C) 2022  Glenn Butcher, glenn.butcher@gmail.com

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/

#ifndef MYCONFIG_H
#define MYCONFIG_H

#include <string>
#include <map>


class myConfig
{
public:
	myConfig(std::string conffile);
	myConfig() { }

	//use to set and use a global configuration:
	static void loadConfig(std::string conffile);
	static myConfig & getConfig();

	bool flush();
	std::string match_name(std::string section, std::string name);
	bool exists(std::string name);
	bool exists(std::string section, std::string name);
	std::string getValue(std::string name);
	std::string getValue(std::string section, std::string name);
	std::string getValueOrDefault(std::string name, std::string defaultval);
	std::string getValueOrDefault(std::string section, std::string name, std::string defaultval);
	std::map<std::string, std::string> getSubset(std::string spec);
	std::map<std::string, std::string> getSubset(std::string section, std::string spec);
	void setValue(std::string section, std::string name, std::string value);
	void setValue(std::string name, std::string value);
	void deleteValue(std::string section, std::string name);
	void deleteValue(std::string name);
	std::map<std::string, std::string> getDefault();
	std::map<std::string, std::string> getSection(std::string section);
	
	//builds a list of variables to replace in property values where $(name) is found:
	void setVariable(std::string name, std::string value);
	void clearVariables();

	//TempConfig turns on a cache where subsequent setValues store to, and subsequent
	//getValues will query before going to the persistent data.  enableTempConfig(true); 
	//turns it on, enableTempConfig(false); turns it off and clears the cache.
	void enableTempConfig(bool e);
	bool getTempConfig();

private:
	std::string configfile;
	static std::map<std::string, std::string> defaultconfig;
	static std::map<std::string, std::map<std::string,std::string> > sectionconfig;
	
	std::string replace_variables(std::string str);
	std::map<std::string, std::string> variables;

	bool temp;
	std::map<std::string, std::string> tempconfig;


};


extern myConfig config;


#endif

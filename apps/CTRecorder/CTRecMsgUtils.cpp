#include "CTRecMsgUtils.h"

#include <sstream>

#include "log.h"


using std::istringstream;
using std::map;
using std::string;


string CTRecStringTrim(const string& str)
{
	const char * lpos = str.c_str();
	while (isspace(*lpos & 0xff))
		lpos++;

	if (*lpos == '\0')
		return string();

	const char * rpos = str.c_str()+str.length()-1;
	if (!isspace(*rpos & 0xff))
	{
		if (lpos == str.c_str())
			return str;
		else
			return string(lpos);
	}

	while (isspace(*rpos & 0xff))
		rpos--;

	return string(lpos, rpos - lpos + 1);
}

bool CTRecMIMEInfo::AddMIME(const string& line)
{
	size_t colonPos = line.find(':');
	if (colonPos == string::npos)
		return false;

	//DBG("line: '%s' - colonPos: %d\n", line.c_str(), colonPos);

	string fieldName = CTRecStringTrim(line.substr(0, colonPos));
	string fieldValue = CTRecStringTrim(line.substr(colonPos+1));

	//DBG("fieldName: '%s' - fieldValue: '%s'\n", fieldName.c_str(), fieldValue.c_str());

	return AddMIME(fieldName, fieldValue);
}

bool CTRecMIMEInfo::AddMIME(const string& fieldName, const string& _fieldValue)
{
	string fieldValue(_fieldValue);

	if (m_Map.count(fieldName) > 0)
		fieldValue = m_Map[fieldName] + '\n' + fieldValue;

	m_Map[fieldName] = fieldValue;

	return true;
}

void CTRecMIMEInfo::ReadFrom(const string& SIPMsg)
{
	istringstream strm(SIPMsg);

	m_Map.clear();

	string line;
	string lastLine;

	while (!strm.bad() && !strm.eof())
	{
		//strm >> line;
		getline(strm, line);
		if ((line.length() > 0) && (line[line.length()-1]=='\r'))
			line[line.length()-1] = '\0';

			if (line.empty())
			break;
		if (line[0] == ' ') 
			lastLine += line;
		else {
			AddMIME(lastLine);
			lastLine = line;
		}
	}

	if (!lastLine.empty()) {
		AddMIME(lastLine);
	}

	// Only for debugging purposes
	CTRecMIMEInfoMap::const_iterator mapiter = m_Map.begin();
	while (mapiter != m_Map.end())
	{
		DBG("got hdr %s: %s\n", (*mapiter).first.c_str(), (*mapiter).second.c_str());

		++mapiter;
	}
};

map<string, string> CTRecMIMEInfo::FindByRegex(const string& regexstr) const
{
	regex_t regex;

	if (regcomp(&regex, regexstr.c_str(), REG_EXTENDED|REG_NOSUB))
	{
		ERROR("Could not compile regexstr %s\n", regexstr.c_str());

		return map<string, string>();
	}

	return FindByRegex(regex);
}

map<string, string> CTRecMIMEInfo::FindByRegex(const regex_t& regex) const
{
	map<string, string> ret;

	CTRecMIMEInfoMap::const_iterator mapiter = m_Map.begin();
	while (mapiter != m_Map.end())
	{
		if (!regexec(&regex, (*mapiter).first.c_str(), 0,0,0))
		{	
//			DBG("got tag %s: %s\n", (*mapiter).first.c_str(), (*mapiter).second.c_str());

			ret[(*mapiter).first] = (*mapiter).second;
		}

		++mapiter;
	}

	return ret;
}

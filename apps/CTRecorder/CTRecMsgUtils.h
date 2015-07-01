#ifndef _CTRECMSGUTILS_H_
#define _CTRECMSGUTILS_H_

#include <map>
#include <string>
#include <vector>
#include <regex.h>


std::string CTRecStringTrim(const std::string& str);

class CTRecMIMEInfo
{
public:
	void ReadFrom(const std::string& SIPMsg);

	bool Contains(const std::string& key) const { return (m_Map.count(key) > 0); }
	const std::string& operator[] (const std::string& key) { return m_Map.operator[](key); }
	std::map<std::string, std::string> FindByRegex(const std::string& regexstr) const;
	std::map<std::string, std::string> FindByRegex(const regex_t& regex) const;

private:
	bool AddMIME(const std::string& line);
	bool AddMIME(const std::string& fieldName, const std::string& _fieldValue);

	struct CaseInsensitive
	{
		bool operator()(std::string const& left, std::string const& right) const
		{
			size_t const size = std::min(left.size(), right.size());

			for (size_t i = 0; i != size; ++i)
			{
				char const lowerLeft = std::tolower(left[i]);
				char const lowerRight = std::tolower(right[i]);

				if (lowerLeft < lowerRight) { return true; }
				if (lowerLeft > lowerRight) { return false; }

				// if equal? continue!
			}

			// same prefix? then we compare the length
			return left.size() < right.size();
		}
	};

	typedef std::map<std::string, std::string, CaseInsensitive> CTRecMIMEInfoMap;

	CTRecMIMEInfoMap m_Map;
};

#endif

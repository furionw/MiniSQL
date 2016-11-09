#pragma once

#include <list>
#include <string>
#include "global.h"

// 构造单态的，只公开run方法的Controler模块
class Controler
{
public:
	static Controler& instance();
	void run();
private:
	Controler();
	~Controler() {}
	Controler(const Controler& );
	Controler& operator=(const Controler& );

	std::list<std::string> m_cmd;
	QUERY_TYPE m_queryType;
	friend class Shell;
};

inline Controler& Controler::instance()
{
	static Controler obj;
	return obj;
}
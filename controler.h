#pragma once

#include <list>
#include <string>
#include "global.h"

// ���쵥̬�ģ�ֻ����run������Controlerģ��
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
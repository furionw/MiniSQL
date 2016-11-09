#include "controler.h"
#include "shell.h"
#include "interpreter.h"
#include <list>
#include <string>
#include <ctime>

using namespace std;

void Controler::run()
{
	while( true )
	{
		Shell::read(); // ��ԪShell��ȡ�û����룬������ƴ�ӵ�Controler::instance().m_cmd��β��
		while( m_cmd.empty() == false ) // ��m_cmd�ǿ�ʱ����ִ��������
		{
			try
			{
				string cmd = m_cmd.front();
				m_cmd.pop_front();
				m_queryType = Interpreter::parse( cmd ); // ��m_cmd��ͷԪ�ش��ε�Interpreterģ����
				Shell::printResult();                                          // ��Shell��ӡ���
			} catch (string e)
			{
				Shell::printException(e);                                 // �����쳣������쳣
			} catch ( ... )
			{}
		}
	}
}

// �������ʱ�ڵ�ǰĿ¼�´�������Ŀ¼���Դ������manager������
Controler::Controler()
{
	system("mkdir .\\index");
	system("mkdir .\\record");
	system("mkdir .\\catalog");
	system("cls");
}
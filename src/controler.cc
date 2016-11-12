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
		Shell::read(); // 友元Shell读取用户输入，将输入拼接到Controler::instance().m_cmd的尾部
		while( m_cmd.empty() == false ) // 当m_cmd非空时依次执行其内容
		{
			try
			{
				string cmd = m_cmd.front();
				m_cmd.pop_front();
				m_queryType = Interpreter::parse( cmd ); // 将m_cmd的头元素传参到Interpreter模块中
				Shell::printResult();                                          // 由Shell打印结果
			} catch (string e)
			{
				Shell::printException(e);                                 // 捕获到异常，输出异常
			} catch ( ... )
			{}
		}
	}
}

// 构造对象时在当前目录下创建三个目录用以存放三个manager的数据
Controler::Controler()
{
	system("mkdir .\\index");
	system("mkdir .\\record");
	system("mkdir .\\catalog");
	system("cls");
}
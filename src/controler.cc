#include <string>
#include "controler.h"
#include "shell.h"
#include "interpreter.h"

using std::string;

void Controler::run() {
  while (true) {
    // 友元Shell读取用户输入，将输入拼接到Controler::instance().m_cmd的尾部
    Shell::read();
    while (m_cmd.empty() == false) {
      // 当m_cmd非空时依次执行其内容
      try {
        string cmd = m_cmd.front();
        m_cmd.pop_front();
        // 将m_cmd的头元素传参到Interpreter模块中
        m_queryType = Interpreter::parse(cmd);
        // 由Shell打印结果
        Shell::printResult();
      } catch (string e) {
        // 捕获到异常，输出异常
        Shell::printException(e);
      } catch (...) {}
    }
  }
}

// 构造对象时在当前目录下创建三个目录用以存放三个manager的数据
Controler::Controler() {
  system("mkdir index");
  system("mkdir record");
  system("mkdir catalog");
  system("cls");
}

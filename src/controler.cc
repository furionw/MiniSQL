#include <string>
#include "controler.h"
#include "shell.h"
#include "interpreter.h"

using std::string;

void Controler::run() {
  while (true) {
    // ��ԪShell��ȡ�û����룬������ƴ�ӵ�Controler::instance().m_cmd��β��
    Shell::read();
    while (m_cmd.empty() == false) {
      // ��m_cmd�ǿ�ʱ����ִ��������
      try {
        string cmd = m_cmd.front();
        m_cmd.pop_front();
        // ��m_cmd��ͷԪ�ش��ε�Interpreterģ����
        m_queryType = Interpreter::parse(cmd);
        // ��Shell��ӡ���
        Shell::printResult();
      } catch (string e) {
        // �����쳣������쳣
        Shell::printException(e);
      } catch (...) {}
    }
  }
}

// �������ʱ�ڵ�ǰĿ¼�´�������Ŀ¼���Դ������manager������
Controler::Controler() {
  system("mkdir index");
  system("mkdir record");
  system("mkdir catalog");
  system("cls");
}

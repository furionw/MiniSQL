#ifndef SHELL_H_
#define SHELL_H_

#include <list>
#include <string>

class Shell
{
public:
  // ��ȡ�û����룬���û�����ƴ�ӵ�Controler::instance().m_cmdβ��
  static void read();  
  // ��ȡ�ļ�(execfile), ���ļ��е�queryָ��ƴ�ӵ�Controler::instance().m_cmdβ��
  static void readFromFile(const std::string& fileName); 
  // ��ӡ��ѯ���
  static void printResult();
  // ��ӡ�쳣��Ϣ
  static void printException(const std::string& exceptionStr);
private:
  static void onCreateTable();
  static void onDropTable();
  static void onCreateIndex();
  static void onDropIndex();
  static void onSelectTuple();
  static void onInsertTuple();
  static void onDeleteTuple();
  static void onQuit();
  static void onExecfile();
  static void onShowTables();
  static void onDescTable();
  static void onShowIndex();
  static void printHorizontal(const size_t size);
  static std::string m_tableName;
  friend class Interpreter;
};

#endif  // SHELL_H_

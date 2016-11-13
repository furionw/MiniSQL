#ifndef SHELL_H_
#define SHELL_H_

#include <list>
#include <string>

class Shell
{
public:
  // 读取用户输入，将用户输入拼接到Controler::instance().m_cmd尾部
  static void read();  
  // 读取文件(execfile), 将文件中的query指令拼接到Controler::instance().m_cmd尾部
  static void readFromFile(const std::string& fileName); 
  // 打印查询结果
  static void printResult();
  // 打印异常信息
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

#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <string>
#include <regex>
#include <ctime>
#include <vector>
#include "global.h"

class Interpreter {
 public:
  // 解析sql指令
  static QUERY_TYPE parse(const std::string& cmd);

 private:
  // onXXX由parse调用
  static QUERY_TYPE onCreateTable(const std::smatch& cmd);
  static QUERY_TYPE onDropTable(const std::smatch& cmd);
  static QUERY_TYPE onCreateIndex(const std::smatch& cmd);
  static QUERY_TYPE onDropIndex(const std::smatch& cmd);
  static QUERY_TYPE onSelectTuple(const std::smatch& cmd);
  static QUERY_TYPE onInsertTuple(const std::smatch& cmd);
  static QUERY_TYPE onDeleteTuple(const std::smatch& cmd);
  static QUERY_TYPE onQuit(const std::smatch& cmd);
  static QUERY_TYPE onExecfile(const std::smatch& cmd);
  // 将where子句的内容解析为vector<Condition>
  static std::vector<Condition> getVtCondition(const std::smatch& cmd);
  // 测试select的约束条件、insert的值是否与对应的attribute匹配，如：char类型的attribute要求输入的值有''符号
  static std::string inputTest(const Attribute& attr, const std::string& value);
};

// create table (..., ..., ...);
// drop table xxx;
// create index indexname on tablename(xxx)
// drop index indexname on tablename
// select * from tablename where
// insert into tableName values()
// delete from tableName where
// quit
// execfilename
// show tables
// desc tableName
// show index from tableName;

#endif  // INTERPRETER_H_

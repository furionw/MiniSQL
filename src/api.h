#ifndef API_H_
#define API_H_

#include <string>
#include <vector>
#include "record.h"
#include "catalog.h"
#include "index.h"
#include "global.h"
#include "buffer.h"

class API
{
public:
  static API& instance();
  // create table tableName(vector<Attribute>);
  void createTable(const std::string& tableName, const std::vector<Attribute>& vtAttr) const;
  // drop table tableName;
  void dropTable(const std::string& tableName) const;
  // create index indexName on tableName(attrName);
  void createIndex(const std::string& indexName, const std::string& tableName, const std::string& attrName) const;
  // drop index indexName on tableName;
  void dropIndex(const std::string& indexName, const std::string& tableName) const;
  // select * from query.m_tableName where query.m_condition( vector<Condition> )
  void selectTuple(const Query& query);
  // 供Shell::printResult() -> onSelectTuple() 调用
  bool hasNext();
  // 供Shell::printResult() -> onSelectTuple() 调用
  std::vector<Tuple> next();
  // insert into tableName(...);
  void insertTuple(const std::string& tableName, const std::vector<std::string>& tuple) const;
  // delete from query.m_tableName where query.m_condition
  void deleteTuple(const Query& query);
private:
  API() {}
  ~API() {}
  API(const API& );
  API& operator=(const API&);
  // called by deleteTuple to fix indice by calling IndexMan::deleteIndex() or IndexMan::insertIndex()
  void fixIndex(const Table& table, const std::vector<std::pair<Tuple, int>>& result, const bool isInsert) const;
  // called by next when m_flag = true
  std::vector<Tuple> selectViaIndex();
  // called by next when m_flag = false
  std::vector<Tuple> selectViaForeach();
  // initial m_flag and m_offset
  void initOffset();
  // update m_offset
  std::vector<int> setIntersection(std::vector<int>& a, std::vector<int>& b);
  std::vector<std::pair<std::string, int>> getInfo(const int locate, const std::vector<std::pair<Tuple, int>>& tuple) const;

  // 这都是为onSelectTuple, hasNext, next设计的成员变量
  Query m_query; // 查询
  Table m_table;   // 表
  std::vector<int> m_offset; // index查询而得的偏移
  bool m_flag;       // 是通过m_offset还是遍历record中的每一块输出查询结果
  size_t m_cnt;      // 当遍历record的每一块时，记录当前遍历的块号
  size_t m_blockBound; // 块号上界
  friend class Shell;
};

inline API& API::instance()
{
  static API obj;
  return obj;
}

#endif
//  bool updateTuple(const std::string& tableName, const std::vector<std::string>& tuple);
//  bool addAttribute(const std::string& tableName, const Attribute& attr) {}  // additional
//  bool dropAttribute(const std::string& tableName, const std::string& attrName) {} // additional
//  bool selectViaForeach(const Query& query, vector<string>& vtOut);
//  int getBlockNum(int offset) { return offset / BLOCK_SIZE; }
//  int getOffset(int offset) { return offset % BLOCK_SIZE; }
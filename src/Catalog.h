#ifndef CATALOG_H_
#define CATALOG_H_

#include <vector>
#include <string>
#include <map>
#include <cassert>
#include <iostream>

#include "global.h"

// enum TYPE;
class Table;
class Attribute;

class CatalogMan
{
public:
  // singleton
  static CatalogMan& instance(); 

  // throw string("table 'tableName' already exist");
  void createTableCheck(const std::string& tableName, const std::vector<Attribute>& vtAttr) const; 
  // throw string("table 'tableName' doesn't exist");
  void dropTableCheck(const std::string& tableName) const;
  // throw string("Duplicate index 'indexName'");
  void createIndexCheck(const std::string& indexName, const std::string& tableName, const std::string& attrName) const;
  // throw string("Index 'indexName' doesn't exist");
  void dropIndexCheck(const std::string& indexName, const std::string& tableName) const;
  // for the furture
  void insertTupleCheck(const std::string& tableName, const std::vector<std::string>& strTuple) const {}
  // for the furture
  void deleteTupleCheck(const Query& query) const {}
  // for the furture
  void selectTupleCheck(const Query& query) const {}

  // query
  void createTable(const std::string&, const std::vector<Attribute>&);
  void dropTable(const std::string&);
  void createIndex(const std::string&, const std::string&, const std::string&);
  bool alreadyHasBPlusTree(const std::string& tableName, const std::string& attrName) const { return  m_cnt.count(tableName + "&" + attrName)==1; }
  void dropIndex(const std::string&, const std::string&);
  bool gonnaRemoveBPlusTree(const std::string& tableName, const std::string& indexName);

  Table getTable(const std::string& tableName) const;
  std::string getAttrName(const std::string& tableName, const std::string& indexName) const;
private:
  CatalogMan();
  ~CatalogMan();
  CatalogMan(const CatalogMan&);
  CatalogMan& operator=(const CatalogMan&);
  void loadTable(); 
  void loadIndex(); 
  void initCnt();
  void flush();

  mutable std::map<std::string, Table> m_table;
  std::map<std::string, int> m_cnt;
  std::multimap<std::string, std::pair<std::string, std::string>> m_index;
  static const int M_UNIQUE;
  static const std::string M_DIR;
  friend class Shell;
};

inline CatalogMan& CatalogMan::instance()
{
  static CatalogMan obj;
  return obj;
}

#endif  // CATALOG_H_

#include "api.h"
#include <algorithm>
#include <vector>

using namespace std;

void API::selectTuple(const Query& query)
{
  CatalogMan::instance().selectTupleCheck(query);
  m_table = CatalogMan::instance().getTable(query.m_tableName);
  m_query = query;
  m_cnt = 0;
  m_flag = false;
  m_blockBound = RecordMan::instance().getBound(query.m_tableName); // 获得record文件的块号上界
  initOffset();
}

void API::initOffset()
{
  Table table = CatalogMan::instance().getTable(m_query.m_tableName);
  bool first = true;
  for (auto it = m_query.m_condition.begin(); it != m_query.m_condition.end(); )
  {
    if( table.getAttribute(it -> m_attrName).isIndex() ||
      table.getAttribute(it -> m_attrName).isUnique() ||
      table.getAttribute(it -> m_attrName).isPrimary() )
    {  
      m_flag = true;    
      vector<int> vtOffset = IndexMan::instance().find(table, *it);
      it = m_query.m_condition.erase( it ); // 剔除已在B+树中检索过的约束条件
      if( first )
      {
        m_offset = vtOffset;
        first = false;
      } else // 与此前得到的结果去交集
      {
        m_offset = setIntersection(m_offset, vtOffset);
      }
    } else
    {
      ++ it;
    }
  }
}

vector<int> API::setIntersection(vector<int>& a, vector<int>& b)
{
  vector<int> ret(a.size());
  sort(a.begin(), a.end());
  sort(b.begin(), b.end());
  auto it = set_intersection(a.begin(), a.end(), b.begin(), b.end(), ret.begin());
  ret.resize( it - ret.begin() );
  return ret;
}

bool API::hasNext()
{
  if( m_flag )
  {
    return (m_cnt < m_offset.size());
  } else
  {
    return (m_cnt <= m_blockBound);
  }
}

vector<Tuple> API::next()
{
  if( m_flag )
  {
    return selectViaIndex();
  } else
  {
    return selectViaForeach();
  }
}

vector<Tuple> API::selectViaIndex()
{
  vector<int> target;
  size_t start = m_offset[m_cnt];
  for( ; m_cnt < m_offset.size() && (m_offset[m_cnt] - start) < 100 * BLOCK_SIZE; m_cnt ++) // 100 is changeable
  {
    target.push_back( m_offset[m_cnt] );
  }
  return RecordMan::instance().selectTuple(m_table, m_query.m_condition, target);
}

vector<Tuple> API::selectViaForeach()
{
  return RecordMan::instance().selectTuple(m_table, m_query.m_condition, m_cnt ++);
}

void API::createTable(const string& tableName, const vector<Attribute>& vtAttr) const
{
  CatalogMan::instance().createTableCheck(tableName, vtAttr); 
  CatalogMan::instance().createTable(tableName, vtAttr);
  RecordMan::instance().createTable(tableName);
//  IndexMan::instance().createIndex(tableName, vtAttr);
}

void API::dropTable(const string& tableName) const
{
  CatalogMan::instance().dropTableCheck(tableName);
  Table table = CatalogMan::instance().getTable(tableName);
  vector<string> vtAttrName = table.getAttrNameIsIndex();
  CatalogMan::instance().dropTable(tableName); // dropIndex at the same time
  RecordMan::instance().dropTable(tableName); 
  IndexMan::instance().dropIndex(tableName, vtAttrName);
}


/******************* drop index indexName on tableName **********************************/
void API::dropIndex(const string& indexName, const string& tableName) const
{
  CatalogMan::instance().dropIndexCheck(indexName, tableName);
  string attrName = CatalogMan::instance().getAttrName(tableName, indexName);
  if ( CatalogMan::instance().gonnaRemoveBPlusTree(tableName, indexName) )
  {
    CatalogMan::instance().dropIndex(indexName, tableName);    
    IndexMan::instance().dropIndex(tableName, attrName);
  } else
  {
    CatalogMan::instance().dropIndex(indexName, tableName);
  }
}


/******************* create index indexName on tableName(attrName)  **********************/
void API::createIndex(const string& indexName, const string& tableName, const string& attrName) const
{
  CatalogMan::instance().createIndexCheck(indexName, tableName, attrName);

  if ( CatalogMan::instance().alreadyHasBPlusTree(tableName, attrName) )
  {
    CatalogMan::instance().createIndex(indexName, tableName, attrName);
    return ;
  }

  Table table = CatalogMan::instance().getTable( tableName );
  Attribute attr = table.getAttribute(attrName);
  CatalogMan::instance().createIndex(indexName, tableName, attrName);
//  IndexMan::instance().createIndex(tableName, attr);
  vector<pair<string, int>> info;
  int blockBound = RecordMan::instance().getBound( tableName );
  for ( int blockNum = 0; blockNum <= blockBound; blockNum ++ )
  {
    info.clear();
    info = RecordMan::instance().getOffsetInfo(table, attrName, blockNum);
    IndexMan::instance().insertIndex(tableName, attr, info);
  } 
}

void API::insertTuple(const string& tableName, const vector<string>& strTuple) const
{
  CatalogMan::instance().insertTupleCheck(tableName, strTuple); // auto check hasTable
  Table table = CatalogMan::instance().getTable(tableName);
  IndexMan::instance().insertTupleCheck(table, strTuple);
  int offset = RecordMan::instance().insertTuple(table, strTuple);
  vector<pair<Tuple, int>> result;
  result.push_back( make_pair(Tuple(strTuple), offset ));
  fixIndex(table, result, true);
}

void API::deleteTuple(const Query& query)
{
  CatalogMan::instance().deleteTupleCheck(query);
  Table table = CatalogMan::instance().getTable(query.m_tableName);
  m_query = query;
  initOffset();
  vector<pair<Tuple, int>> result;
  if( m_flag == true )
  {
    result = RecordMan::instance().deleteTuple(table, m_query.m_condition, m_offset);
  } else
  {
    result = RecordMan::instance().deleteTuple(table, query.m_condition);
  }
  fixIndex(table, result, false);
}

void API::fixIndex(const Table& table, const vector<pair<Tuple, int>>& result, const bool isInsert) const
{
  auto it = table.m_attr.begin();
  for( ; it != table.m_attr.end(); it ++)
  {
    if(it -> isIndex() || it -> isPrimary() || it -> isUnique())
    {
      Attribute attr = table.getAttribute(it -> m_name);
      auto info = getInfo( table.getAttrNum(it->m_name), result );
      if ( isInsert )
      {
        IndexMan::instance().insertIndex( table.m_name, attr, info );
      } else
      {
        IndexMan::instance().deleteIndex( table.m_name, attr, info );
      }
    }
  }
}

vector<pair<string, int>> API::getInfo(const int locate, const vector<pair<Tuple, int>>& tuple) const
{
  vector<pair<string, int>> ret;
  for(auto it = tuple.begin(); it != tuple.end(); it ++)
  {
    ret.push_back( make_pair( it -> first.m_content[locate], it -> second) );
  }
  return ret;
}
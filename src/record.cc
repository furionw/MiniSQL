#include "record.h"
#include <vector>
#include <string>
#include <iostream>

using namespace std;

const string RecordMan::M_DIR = "record/";
const int RecordMan::M_HEAD_SIZE = 4;
const char RecordMan::M_VALID = 1;

void RecordMan::createTable(const string& tableName) const
{
  BufferMan::instance().write( M_DIR + tableName, string(BLOCK_SIZE, 0) );
}

void RecordMan::dropTable(const std::string& tableName) const
{
  BufferMan::instance().remove( M_DIR + tableName );
}


vector<Tuple> RecordMan::selectTuple(const Table& table, const vector<Condition>& condition, const int blockNum) const
{
  string content;

  if ( BufferMan::instance().read( M_DIR + table.m_name, blockNum, content) == false )
  {
    return vector<Tuple>();
  }

  return unpackOnSelect( table, content, condition ); // is RVO guaranteed?
}

vector<Tuple> RecordMan::selectTuple(const Table& table, const vector<Condition>& condition, const vector<int>& vtOffset) const
{
  vector<Tuple> ret;

  for(auto it = vtOffset.begin(); it != vtOffset.end(); )
  {
    vector<int> offsetWithinBlock;
    int begin = *it;
    while( it != vtOffset.end() && *it / BLOCK_SIZE == begin / BLOCK_SIZE )
    {
      offsetWithinBlock.push_back( *it );
      it ++;
    }
    spliceOnSelect(ret, table, condition, offsetWithinBlock);
  }

  return ret;
}

vector<Tuple> RecordMan::unpackOnSelect(const Table& table, const string& content, const vector<Condition>& condition) const
{
  vector<Tuple> ret;
  int pos = M_HEAD_SIZE;
  int cnt = getCnt(content);
  size_t size = 1 + table.size();

  for(int i = 0; i < cnt; i ++, pos += size)
  {
    if ( isValid(content[pos]) == false )
    {
      continue;
    }
    Tuple cur(table, content, pos+1);
    if ( cur.isSatisfied(table, condition) )
    {
      ret.push_back( cur );
    }
  }

  return ret;  
}

void RecordMan::spliceOnSelect(vector<Tuple>& tupleOut, const Table& table, const vector<Condition>& condition, 
  const vector<int>& offsetWithinBlock) const
{
  string content;
  BufferMan::instance().read( M_DIR + table.m_name, offsetWithinBlock.front() / BLOCK_SIZE, content );

  for(auto it = offsetWithinBlock.begin(); it != offsetWithinBlock.end(); it ++)
  {
    Tuple cur(table, content, (*it+1)%BLOCK_SIZE);
    if( cur.isSatisfied(table, condition) )
    {
      tupleOut.push_back(cur);
    }
  }
}

vector<pair<string, int>> RecordMan::getOffsetInfo(const Table& table, const string& attrName, const int blockNum) const
{
  string content;
  if( BufferMan::instance().read(M_DIR + table.m_name, blockNum, content) == false )
  {
    return vector<pair<string, int>>();
  }
  
  vector<pair<string, int>> ret;
  int pos = M_HEAD_SIZE;
  int cnt = getCnt( content );
  size_t size = 1 + table.size();
  size_t attrBegin = table.getAttrBegin(attrName);
  size_t attrEnd = table.getAttrEnd(attrName);
  for(int i = 0; i < cnt; i ++, pos += size)
  {
    if( isValid(content[pos] == false) )
    {
      continue;
    }
    ret.push_back( make_pair( content.substr(pos+1+attrBegin, attrEnd-attrBegin), pos+blockNum*BLOCK_SIZE ) );
  } // bug fixed at 13-11-09

  return ret;
}

int RecordMan::insertTuple(const Table& table, const std::vector<std::string>& strTuple) const
{
  string content;
  int blockNum = BufferMan::instance().readLast( M_DIR + table.m_name, content );

  if( hasSpace(table, content) == false )
  {
    content = string(BLOCK_SIZE, 0);
    blockNum = BufferMan::instance().write( M_DIR + table.m_name, content );
  }

  return appendTupleAtRear(table, content, blockNum, strTuple);
}

int RecordMan::appendTupleAtRear(const Table& table, string& content, const int blockNum, const vector<string>& strTuple) const
{
  int cnt = getCnt(content);
  int pos = M_HEAD_SIZE + cnt * (table.size()+1);
  auto itAttr = table.m_attr.begin();
  auto itStr = strTuple.begin();

  content[pos ++] = M_VALID; // 标记成有效位
  for( ; itAttr != table.m_attr.end(); itAttr ++, itStr ++ )
  {
    cpy(content, pos, *itStr);
    pos += itAttr -> m_length;
  }
  incrementCnt( content );
  BufferMan::instance().write( M_DIR + table.m_name, content, blockNum );

  return blockNum * BLOCK_SIZE + M_HEAD_SIZE + cnt * (table.size()+1);
}

/*
void RecordMan::cpy(string& toBeWrite, const string& original) const
{
  for(int i = 0; i < original.size(); i ++)
  {
    toBeWrite[i] = original[i];
  }
}*/

void RecordMan::cpy(string& toBeWrite, const int begin, const string& original) const
{
  for(size_t i = 0; i < original.size(); i ++)
  {
    toBeWrite[begin + i] = original[i];
  }
}

bool RecordMan::hasSpace(const Table& table, const string& content) const
{
  int capcity = (BLOCK_SIZE - M_HEAD_SIZE) / (1 + table.size());
  return capcity > getCnt(content);
}

void RecordMan::incrementCnt(string& content) const
{
  size_t cnt = getCnt(content);
  intTo4Bytes(cnt+1, content, 0, M_HEAD_SIZE);
}

vector<pair<Tuple, int>> RecordMan::deleteTuple(const Table& table, const std::vector<Condition>& condition) const
{
  string content;
  vector<pair<Tuple, int>> ret;

  for(int blockNum = 0; BufferMan::instance().read(M_DIR + table.m_name, blockNum, content); blockNum ++)
  {
    spliceOnDelete(ret, table, content, condition, blockNum);
    BufferMan::instance().write(M_DIR + table.m_name, content, blockNum);
  }

  return ret;
}

// 在有索引的情况下被调用
vector<pair<Tuple, int>> RecordMan::deleteTuple(const Table& table, const std::vector<Condition>& condition, const std::vector<int>& vtOffset) const
{
  string content;
  vector<pair<Tuple, int>> ret;

  for(auto it = vtOffset.begin(); it != vtOffset.end(); )
  {
    vector<int> offsetWithinBlock;
    int begin = *it;
    while( it != vtOffset.end() && *it / BLOCK_SIZE ==  begin / BLOCK_SIZE )
    {
      offsetWithinBlock.push_back( *it );
      it ++;
    }
    BufferMan::instance().read(M_DIR + table.m_name, begin/BLOCK_SIZE, content);
    spliceOnDelete(ret, table, content, condition, offsetWithinBlock);
    BufferMan::instance().write(M_DIR + table.m_name, content, begin/BLOCK_SIZE);
  }

  return ret;
}

void RecordMan::spliceOnDelete(vector<pair<Tuple, int>>& ret, const Table& table, string& content, 
  const vector<Condition>& condition, const int blockNum) const
{
  int pos = M_HEAD_SIZE;
  int cnt = getCnt(content);
  size_t size = 1 + table.size();

  for(int i = 0; i < cnt; i ++, pos += size)
  {
    if ( isValid(content[pos]) == false )
    {
      continue;
    }
//    cout << "here is spliceOnDelete" << endl;
//    cout << content << endl;
    Tuple cur(table, content, pos+1);
    if ( cur.isSatisfied(table, condition) )
    {
      content[pos] = 0; // setUnvalid      
      ret.push_back( make_pair(cur, blockNum * BLOCK_SIZE + pos) );
    }
  }
}

void RecordMan::spliceOnDelete(vector<pair<Tuple, int>>& ret, const Table& table, string& content, 
  const vector<Condition>& condition, const vector<int> vtOffset) const
{
  for(auto it = vtOffset.begin(); it != vtOffset.end(); it ++)
  {
//    assert( isValid(content[(*it)%BLOCK_SIZE]) == true );
    Tuple cur(table, content, (*it+1)%BLOCK_SIZE);
    if ( cur.isSatisfied(table, condition) )
    {
      content[ (*it)%BLOCK_SIZE ] = 0; // setUnvalid
      ret.push_back( make_pair(cur, *it) );
    }
  }
}
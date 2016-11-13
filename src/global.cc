#include "global.h"
#include <iostream>
#include <cmath>

using namespace std;

Attribute Table::getAttribute(const string& attrName) const
{
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if(it -> m_name == attrName)
    {
      return *it;
    }
  }
  throw string("No such attribute '") + attrName + string("' in table '") + m_name + string("'.");
}

vector<string> Table::getAttrNameIsIndex() const
{
  vector<string> ret;
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if (it->isUnique() || it->isIndex() || it->isPrimary())
    {
      ret.push_back(it -> m_name);
    }
  }
  return ret;
}

size_t Table::size() const
{
  size_t size = 0;
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    size += it -> m_length;
  }
  return size;
}

size_t Table::getAttrNum(const string& attrName) const 
{
  size_t num = 0;
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if( attrName == it -> m_name )
    {
      return num;
    }
    num ++;
  }
  assert( false );
  return 0;
}

size_t Table::getAttrBegin(const string& attrName) const
{
  size_t begin = 0;
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if( attrName == it -> m_name )
    {
      return begin;
    } 
    begin += it -> m_length;
  }
  assert( false );
  return 0;
}

size_t Table::getAttrEnd(const string& attrName) const
{
  size_t end = 0;
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    end += it -> m_length;
    if( attrName == it -> m_name )
    {
      return end;
    } 
  }
  assert( false );
  return 0;
}

DATA_TYPE Table::getAttrType(const string& attrName) const
{
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if( attrName == it -> m_name )
    {
      return it -> m_type;
    } 
  }
  assert( false );  
  return DATA_TYPE::INT;
}

void Table::setIndex(const string& attrName)
{
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if(it -> m_name == attrName)
    {
      return it -> setIndex();
    }
  }
}

void Table::unsetIndex(const string& attrName)
{
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if(it -> m_name == attrName)
    {
      return it -> unsetIndex();
    }
  }
}

void Table::setPriamry(const string& attrName)
{
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if(it -> m_name == attrName)
    {
      return it -> setPrimary();
    }
  }
}

void Table::setUnique(const string& attrName)
{
  for(auto it = m_attr.begin(); it != m_attr.end(); it ++)
  {
    if(it -> m_name == attrName)
    {
      return it -> setUnique();
    }
  }
}

Tuple::Tuple(const Table& table, const string& content, int begin)
{
  for(auto it = table.m_attr.begin(); it != table.m_attr.end(); it ++)
  {
    m_content.push_back( content.substr(begin, it -> m_length) ); // substr的参数是pos, len!!!
    begin += it -> m_length;
  }
}

Tuple::Tuple(const vector<string>& strTuple)
{
  for(auto it = strTuple.begin(); it != strTuple.end(); it ++)
  {
    m_content.push_back(*it);
  }
}

bool Tuple::isSatisfied(const Table& table, const vector<Condition>& condition) const
{
  for(auto itCon = condition.begin(); itCon != condition.end(); itCon ++)
  {
    int attrNum = table.getAttrNum( itCon -> m_attrName );
    DATA_TYPE attrType = table.getAttrType( itCon -> m_attrName );
    if( judge(attrType, m_content[attrNum], *itCon) == false )
    {
      return false;
    }
  }
  return true; // 没返回值时居然返回true...
}

bool Tuple::judge(const DATA_TYPE operandType, const string& operandA, const Condition& condition) const
{
  switch( operandType )
  {
    case DATA_TYPE::INT:
      return intJudge(condition.m_opType, operandA, condition.m_operand);
    case DATA_TYPE::CHAR:
      return charJudge(condition.m_opType, operandA, condition.m_operand);
    case DATA_TYPE::FLOAT:
      return floatJudge(condition.m_opType, operandA, condition.m_operand);
    default:
      return false; // soft complior
  }  
}

// atoi将使用c_str(), 故在此并没有使用它
bool Tuple::intJudge(const OP_TYPE opType, const string& operandA, const string& operandB) const
{
  switch(opType)
  {
    case OP_TYPE::EQUAL:
      return stringToInt(operandA) == stringToInt(operandB);
    case OP_TYPE::NO_EQUAL:
      return stringToInt(operandA) != stringToInt(operandB);
    case OP_TYPE::LESS:
      return stringToInt(operandA) < stringToInt(operandB);
    case OP_TYPE::GREATER:
      return stringToInt(operandA) > stringToInt(operandB);
    case OP_TYPE::NO_GREATER:
      return stringToInt(operandA) <= stringToInt(operandB);
    case OP_TYPE::NO_LESS:
      return stringToInt(operandA) >= stringToInt(operandB);
    default:
      return false; // soft complior
  }
}

bool Tuple::charJudge(const OP_TYPE opType, const string& operandA, const string& operandB) const
{
  string opA(operandA.c_str()); // left out the 0s in the end
  switch(opType)
  {
    case OP_TYPE::EQUAL:
      return opA == operandB;
    case OP_TYPE::NO_EQUAL:
      return opA != operandB;
    case OP_TYPE::LESS:
      return opA < operandB;
    case OP_TYPE::GREATER:
      return opA > operandB;
    case OP_TYPE::NO_GREATER:
      return opA <= operandB;
    case OP_TYPE::NO_LESS:
      return opA >= operandB;
    default:
      return false; // soft complior
  }
}

// atof返回double, 也将使用c_str(), 故在此并没有使用它
bool Tuple::floatJudge(const OP_TYPE opType, const string& operandA, const string& operandB) const
{
  switch(opType)
  {
    case OP_TYPE::EQUAL:
      return fabs( stringToFloat(operandA) - stringToFloat(operandB) ) < EPS;
    case OP_TYPE::NO_EQUAL:
      return fabs( stringToFloat(operandA) - stringToFloat(operandB) ) >= EPS;
    case OP_TYPE::LESS:
      return stringToFloat(operandA) < stringToFloat(operandB);
    case OP_TYPE::GREATER:
      return stringToFloat(operandA) > stringToFloat(operandB);
    case OP_TYPE::NO_GREATER:
      return stringToFloat(operandA) <= stringToFloat(operandB);
    case OP_TYPE::NO_LESS:
      return stringToFloat(operandA) >= stringToFloat(operandB);
    default:
      return false; // soft complior
  }
}
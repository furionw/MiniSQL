#include "index.h"
#include "BPlusTreeV5.h"

using namespace std;

const string IndexMan::M_DIR = ".\\index\\";

void IndexMan::insertIndex(const string& tableName, 
						   const Attribute& attr, const vector<pair<string, int>>& info) const
{
	switch( attr.m_type )
	{
		case DATA_TYPE::INT:
		{
			BPlusTree<int> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return treeInsert<int>( tree, info );  
		}
		case DATA_TYPE::CHAR:
		{
			BPlusTree<string> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return treeInsert<string>( tree, info );
		}
		case DATA_TYPE::FLOAT:
		{
			BPlusTree<float> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return treeInsert<float>( tree, info );
		}
	}
}

void IndexMan::deleteIndex(const string& tableName, 
						   const Attribute& attr, const vector<pair<string, int>>& info) const
{
	switch( attr.m_type )
	{
		case DATA_TYPE::INT:
		{
			BPlusTree<int> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return treeRemove<int>(tree, info);
		}
		case DATA_TYPE::CHAR:
		{
			BPlusTree<string> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return treeRemove<string>(tree, info);
		}
		case DATA_TYPE::FLOAT:
		{
			BPlusTree<float> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return treeRemove<float>(tree, info);
		}
	}
}

void IndexMan::dropIndex(const string& tableName, const vector<string>& vtAttrName) const
{
	for(auto it = vtAttrName.begin(); it != vtAttrName.end(); it ++)
	{
		BufferMan::instance().remove(M_DIR + tableName + string("@") + *it);
	}
}

void IndexMan::dropIndex(const string& tableName, const string& attrName) const
{
	BufferMan::instance().remove(M_DIR + tableName + "@" + attrName);
}

vector<int> IndexMan::find(const Table& table, const Condition& condition) const
{
	Attribute attr = table.getAttribute(condition.m_attrName);
	switch( attr.m_type )
	{
		case DATA_TYPE::INT:
		{
			BPlusTree<int> tree(M_DIR + table.m_name + string("@") + attr.m_name, attr);
			return tree.find(condition);
		}
		case DATA_TYPE::CHAR:
		{
			BPlusTree<string> tree(M_DIR + table.m_name + string("@") + attr.m_name, attr);
			return tree.find(condition);
		}
		case DATA_TYPE::FLOAT:
		{
			BPlusTree<float> tree(M_DIR + table.m_name + string("@") + attr.m_name, attr);
			return tree.find(condition);
		}
		default: 
			return vector<int>(); // soft compilor
	}
}

void IndexMan::insertTupleCheck(const Table& table, const vector<string>& strTuple) const
{
	for ( size_t i = 0; i < strTuple.size(); i ++)
	{
		if ( table.m_attr[i].isUnique() || table.m_attr[i].isPrimary() )
		{
			Condition condition(table.m_attr[i].m_name, OP_TYPE::EQUAL, strTuple[i]);
			vector<int> ret = find(table, condition);
			if ( ret.empty() == false )
			{
				throw string("Duplicate values on primary or unique key '") + table.m_attr[i].m_name + string("'.");;
			}
		}
	}
}

void IndexMan::print(const std::string& tableName, const Attribute& attr) const
{
	switch( attr.m_type )
	{
		case DATA_TYPE::INT:
		{
			BPlusTree<int> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return tree.print();
		}
		case DATA_TYPE::CHAR:
		{
			BPlusTree<string> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return tree.print();
		}
		case DATA_TYPE::FLOAT:
		{
			BPlusTree<float> tree(M_DIR + tableName + string("@") + attr.m_name, attr);
			return tree.print();
		}
	}	
}

/*
void IndexMan::createIndex(const std::string& tableName, const vector<Attribute>& vtAttr) const
{
	for (auto it = vtAttr.begin(); it != vtAttr.end(); it ++)
	{
		if (it -> isUnique() || it -> isIndex() || it -> isPrimary() )
		{
			switch( it -> m_type )
			{
				case DATA_TYPE::INT:
				{
					BPlusTree<int> intTree(M_DIR + tableName + string("@") + it -> m_name, *it);
				}
				case DATA_TYPE::CHAR:
				{
					BPlusTree<string> charTree(M_DIR + tableName + string("@") + it -> m_name, *it);
				}
				case DATA_TYPE::FLOAT:
				{
					BPlusTree<float> floatTree(M_DIR + tableName + string("@") + it -> m_name, *it);
				}
			}
		}
	}
}

void IndexMan::createIndex(const std::string& tableName, Attribute& attr) const // call by API::createIndex
{
	vector<Attribute> arg;
	attr.setIndex();
	arg.push_back(attr);
	createIndex(tableName, arg);
}*/

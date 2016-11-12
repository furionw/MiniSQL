#include "Catalog.h"
#include <iostream>
#include <fstream>

using namespace std;

const std::string CatalogMan::M_DIR = ".\\catalog\\";
const int CatalogMan::M_UNIQUE = -1;

CatalogMan::CatalogMan()
{
	try
	{
		loadTable(); // init m_table
		loadIndex(); // init m_index
		initCnt();       // init m_cnt
	} catch (...)
	{
		cout << "CatalogMan() failed!" << endl;
		abort();
	}
}

void CatalogMan::loadTable()
{
	ifstream in( M_DIR + "table", ios::binary);
	Table newTable;
	while ( in >> newTable )
	{
		m_table.insert( make_pair(newTable.m_name, newTable) );
		newTable = Table(); // clear the table
	}
	in.close();
}

void CatalogMan::loadIndex() {
	ifstream in(M_DIR + "index", ios::binary);
	string indexName, tableName, attrName;
	while (true) {
		in >> tableName >> indexName >> attrName;
		if (!in.good()) break;
		m_index.insert( make_pair(tableName, make_pair(indexName, attrName)) );
	}
	in.close();
}

void CatalogMan::initCnt()
{
	for (auto it = m_index.begin(); it != m_index.end(); it ++)
	{
		string cntKey = it->first + "&" + it->second.second; // table&index
		if( m_cnt.count( cntKey ) == 0 )
		{
			m_cnt[cntKey] = 1;
		} else
		{
			m_cnt[cntKey] ++;
		}
	}
	for (auto it = m_table.begin(); it != m_table.end(); it ++)
	{
		for (auto itAttr = it->second.m_attr.begin(); itAttr != it->second.m_attr.end(); itAttr ++)
		{
			if (itAttr->isUnique() || itAttr->isPrimary())
			{
				m_cnt[it->first + "&" + itAttr->m_name] = M_UNIQUE;
			}
		}
	}
}

CatalogMan::~CatalogMan()
{
	try
	{
		flush();
	} catch(...)
	{
		// ...
	}
}

void CatalogMan::flush()
{
	ofstream out(".\\catalog\\table", ios::binary);
	for (auto it = m_table.begin(); it != m_table.end(); it ++)
		out << it -> second;
	out.close();
	out.clear(); // avoid the wrong stream?!

	out.open(".\\catalog\\index", ios::binary);
	for (auto it = m_index.begin(); it != m_index.end(); it ++)
		out << it -> first << " " << it -> second.first << " " 
			<< it -> second.second << "\n";
	out.close();
}

Table CatalogMan::getTable(const string& tableName) const
{
	if ( m_table.count(tableName) == 0 )
	{
		string toBeThrown("Table ");
		throw toBeThrown + tableName + string(" doesn't exist");
	} else
	{
		return m_table[tableName];
	}
}

void CatalogMan::createTableCheck(const string& tableName, const vector<Attribute>& vtAttr) const
{
	if ( m_table.count(tableName) != 0 )
	{
		string e("Table '");
		throw e + tableName + string("' already exists.");
	}
}

void CatalogMan::dropTableCheck(const string& tableName) const
{
	if ( m_table.count(tableName) == 0 )
	{
		string e("Table '");
		throw e + tableName + string("' doesn't exist."); // Unknown table 'c'
	}
}

void CatalogMan::createIndexCheck(const string& indexName, const string& tableName, const string& attrName) const
{
	int num = m_index.count(tableName);
	auto it = m_index.find(tableName);	
	for (int i = 0; i < num; i ++, it ++)
	{
		if( it -> second.first == indexName )
		{
			throw string("Duplicate index name '") + indexName + string("'.");
		}
	}
}

void CatalogMan::dropIndexCheck(const string& indexName, const string& tableName) const
{
	bool find = false;
	string attrName;
	int num = m_index.count(tableName);
	auto it = m_index.find(tableName);
	for (int i = 0; i < num; i ++, it ++)
	{
		if ( it -> second.first == indexName )
		{
			find = true;
			attrName = it -> second.second;
			break;
		}
	}
	if ( find == false )
	{
		string e("Index '");
		throw e + indexName + string("' doesn't exist");
	}

	if ( indexName == attrName )
	{
		Table table = m_table[tableName];
		Attribute attr = table.getAttribute(attrName);
		if( attr.isUnique() || attr.isPrimary() )
		{
			string e("Cannot drop '");
			throw e + indexName + string("' on unique or primary key '") + attrName + string("', since it's a index made by miniSQL");
		}
	}
}

/*
void CatalogMan::insertTupleCheck(const string& tableName, const vector<string>& vtStr) const
{}

void CatalogMan::deleteTupleCheck(const Query& query) const
{}

void CatalogMan::selectTupleCheck(const Query& query) const
{}*/

void CatalogMan::createTable(const string& tableName, const vector<Attribute>& vtAttr)
{
	Table table(tableName, vtAttr);
	m_table.insert( make_pair(tableName, table) );
	for (auto it = vtAttr.begin(); it != vtAttr.end(); it ++)
	{
		if (it->isUnique() || it->isPrimary())
		{
			m_index.insert( make_pair(tableName, make_pair(it->m_name, it->m_name)) );
			m_cnt.insert( make_pair(tableName+"&"+it->m_name, M_UNIQUE) );
		}
	}
}

void CatalogMan::dropTable(const string& tableName)
{
	Table table = m_table[ tableName ];
	for (auto it = table.m_attr.begin(); it != table.m_attr.end(); it++)
	{
		if (it->isIndex() || it->isUnique() || it->isPrimary())
		{
			m_cnt.erase( tableName + "&" + it->m_name );
		}
	}
	m_index.erase(tableName);
	m_table.erase(tableName);
}

void CatalogMan::createIndex(const string& indexName, const string& tableName, const string& attrName)
{
	m_index.insert( make_pair(tableName, make_pair(indexName, attrName)) );
	
	if (m_cnt.count(tableName + "&" + attrName) == 0)
	{
		m_cnt[tableName + "&" + attrName] = 1;
	} else if (m_cnt[tableName + "&" + attrName] != M_UNIQUE)
	{
		m_cnt[tableName + "&" + attrName] ++;
	}

	for (auto it = m_table.begin(); it != m_table.end(); it ++)
	{
		if (it -> first == tableName)
		{
			it -> second.setIndex(attrName);
		}
	}
}

void CatalogMan::dropIndex(const string& indexName, const string& tableName)
{
	string attrName;
	auto it = m_index.find(tableName);
	int num = m_index.count(tableName);
	for (int i = 0; i < num; i ++, it ++)
	{
		if (it -> second.first == indexName)
		{
			attrName = it -> second.second;
			m_index.erase(it);
			break;
		}		
	}
	if (m_cnt[tableName + "&" + attrName] != M_UNIQUE)
	{
		m_cnt[tableName + "&" + attrName] --;
	}

	if ( m_cnt[tableName + '&' + attrName] == 0 )
	{
		m_cnt.erase( tableName + '&' + attrName );
		for (auto it = m_table.begin(); it != m_table.end(); it ++)
		{
			if (it -> first == tableName)
			{
				it -> second.unsetIndex(attrName);
			}
		}
	}
}

bool CatalogMan::gonnaRemoveBPlusTree(const string& tableName, const string& indexName)
{
	string attrName;
	auto it = m_index.find(tableName);
	int num = m_index.count(tableName);
	for (int i = 0; i < num; i ++, it ++)
	{
		if (it -> second.first == indexName)
		{
			attrName = it -> second.second;
			break;
		}
	}
	return m_cnt[tableName + "&" + attrName] == 1;
}

string CatalogMan::getAttrName(const string& tableName, const string& indexName) const
{
	auto it = m_index.find(tableName);
	int num = m_index.count(tableName);
	for (int i = 0; i < num; i ++, it ++)
	{
		if (it -> second.first == indexName)
		{
			return it -> second.second;
		}
	}
	throw  string("In getAttrName. No such index name");
}

istream& operator>>(istream& is, Table& table)
{	
	size_t columnNum;
	Attribute attrTmp;

	if ( !(is >> table.m_name) )
		return is;
//	cout << "tableName: " << table.m_name << endl;
	is >> columnNum;
	for (size_t i = 0; i < columnNum; i ++)
	{
//		cout << "i" << i << endl;
		is >> attrTmp;
		table.m_attr.push_back( attrTmp );
	}

	return is;
}

istream& operator>>(istream& is, Attribute& attr)
{
	int tmp;

	is >> attr.m_name;
	is >> tmp;
	attr.m_type = (DATA_TYPE)tmp;
	is >> attr.m_length;
	is >> attr.m_flag;

	return is;
}

ostream& operator<<(ostream& os, const Table& table)
{
	os << table.m_name << " " << table.m_attr.size() << endl;
	for (auto it = table.m_attr.begin(); it != table.m_attr.end(); it ++)
	{
		os << *it;
	}
	return os;
}

ostream& operator<<(ostream& os, const Attribute& attr)
{
	int tmp = (int)attr.m_type;

	os << attr.m_name << " "
	  << tmp << " "
	  << attr.m_length << " "
	  << attr.m_flag << "\n";
	return os;
}
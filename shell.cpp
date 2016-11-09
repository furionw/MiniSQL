#include "shell.h"
#include "api.h"
#include "controler.h"
#include <iostream>
#include <algorithm>

using namespace std;

string Shell::m_tableName;

void Shell::read()
{
	string former;
	string cmd;
	bool find = false;
	bool first = true;
	while( true )
	{
		if (first)
		{
			cout << "miniSQL> ";
			first = false;
		} else
		{
			cout << "       > ";
		}
		getline(cin, cmd);
		cmd = former + cmd;
		size_t begin = 0;
		size_t end = 0;
		for ( ; end < cmd.size(); end ++)
		{
			if ( cmd[end] == ';' )
			{
				Controler::instance().m_cmd.push_back( cmd.substr(begin, end - begin) );
				find = true;
				begin = end + 1;
			}
		}
		former = cmd.substr(begin, end - begin);
		if ( find )
		{
			break;
		}
	}
}

void Shell::readFromFile(const string& fileName)
{
	ifstream in(fileName, ios::binary);
	if ( in == 0 )
	{
		throw string("No such file exists.");
	}
	string cmd;
	while ( getline(in, cmd, ';'))
	{
//		cout << cmd << endl;
		cmd.erase(remove(cmd.begin(), cmd.end(), '\r'), cmd.end());
		cmd.erase(remove(cmd.begin(), cmd.end(), '\n'), cmd.end());
//		cout << "CMD: " << endl;
//		cout << cmd << endl;
		Controler::instance().m_cmd.push_back(cmd);
	}
}

void Shell::printException(const string& exceptionStr)
{
	cout << exceptionStr << endl << endl;
}

void Shell::printResult()
{
	switch( Controler::instance().m_queryType )
	{
		case QUERY_TYPE::CREATE_TABLE:
			return onCreateTable();
		case QUERY_TYPE::DROP_TABLE:
			return onDropTable();
		case QUERY_TYPE::CREATE_INDEX:
			return onCreateIndex();
		case QUERY_TYPE::DROP_INDEX:
			return onDropIndex();
		case QUERY_TYPE::SELECT_TUPLE:
			return onSelectTuple();
		case QUERY_TYPE::INSERT_TUPLE:
			return onInsertTuple();
		case QUERY_TYPE::DELETE_TUPLE:
			return onDeleteTuple();
		case QUERY_TYPE::SHOW_TABLES:
			return onShowTables();
		case QUERY_TYPE::DESC:
			return onDescTable();	
		case QUERY_TYPE::SHOW_INDEX:
			return onShowIndex();
		// ...
	}
}

void Shell::onCreateTable()
{
	cout << "Query OK, 0 rows affected." << endl << endl;
}

void Shell::onDropTable()
{
	cout << "Query OK, drop table done." << endl << endl;
}

void Shell::onCreateIndex()
{
	cout << "Query OK, create index done." << endl << endl;	
}

void Shell::onDropIndex()
{
	cout << "Query OK, drop index done." << endl << endl;
}

void Shell::onInsertTuple()
{
	cout << "Query OK, 1 row afftected." << endl << endl;
}

void Shell::onDeleteTuple()
{
	cout << "Query OK, delete tuple done." << endl << endl;
}

void Shell::onSelectTuple()
{
	Table table = API::instance().m_table;
	vector<int> vtColumnSize;
	for( auto it = table.m_attr.begin(); it != table.m_attr.end(); it ++ )
		vtColumnSize.push_back( (it->m_name.size()>it->m_length)? it->m_name.size(): it->m_length );

	cout << "+";
	char prev = cout.fill('-');
	for ( auto it = vtColumnSize.begin(); it != vtColumnSize.end(); it ++)
	{
		cout.width( *it + 3 );
		cout << '+';
	}
	cout << endl;

	cout.fill(' ');
	for( size_t i = 0; i < vtColumnSize.size(); i ++ )
	{
		cout << "| ";
		cout.width( vtColumnSize[i] );
		cout << left << table.m_attr[i].m_name << " ";
	}
	cout << "|" << endl;

	cout << "+";
	prev = cout.fill('-');
	for ( auto it = vtColumnSize.begin(); it != vtColumnSize.end(); it ++)
	{
		cout.width( *it + 3 );
		cout << right << '+';
	}
	cout << endl;

	cout.fill(' ');
	size_t cnt = 0;
	while( API::instance().hasNext() )
	{
		auto vtTuple = API::instance().next();
		for (auto it = vtTuple.begin(); it != vtTuple.end(); it ++)
		{
			for ( size_t i = 0; i < vtColumnSize.size(); i ++ )
			{
				cout << "| ";
				cout.width( vtColumnSize[i] );
				cout << left << it -> m_content[i] << " ";
			}
			cout << "|" << endl;
			cnt ++;
		}
	}

	cout << "+";
	prev = cout.fill('-');
	for ( auto it = vtColumnSize.begin(); it != vtColumnSize.end(); it ++)
	{
		cout.width( *it + 3 );
		cout << left << right << '+';
	}
	cout << endl;

	cout << "Query OK, " << cnt << (cnt<=1? " row in set": " rows in set") << endl << endl;
}

void Shell::onShowTables()
{
	size_t maxSize = 12; // Tables_in_db
	for (auto it = CatalogMan::instance().m_table.begin(); it != CatalogMan::instance().m_table.end(); it ++)
		maxSize = (it->first.size())>maxSize? it->first.size(): maxSize;


	printHorizontal(maxSize);

	cout << "| ";
	for (size_t i = 0; i < (maxSize-12)/2; i ++)
		cout << " ";
	cout << "Tables_in_db";
	for (size_t i = 0; i < (maxSize-12)/2; i ++)
		cout << " ";
	cout << " |" << endl;

	printHorizontal(maxSize);

	for (auto it = CatalogMan::instance().m_table.begin(); it != CatalogMan::instance().m_table.end(); it ++)
	{
		cout << "| ";
		cout << it -> first ;
		for(size_t i = it -> first.size(); i < maxSize; i ++)
		{
			cout << " ";
		}
		cout << " |" << endl;
	}
	
	printHorizontal(maxSize);
	cout << endl;
}

void Shell::onDescTable()
{
	Table table = CatalogMan::instance().getTable(m_tableName);
	size_t fieldSize = 5;
	size_t typeSize = 6; 
	for (auto it = table.m_attr.begin(); it != table.m_attr.end(); it ++)
	{
		fieldSize =  (it -> m_name.size())>fieldSize?  (it -> m_name.size()): fieldSize;
			typeSize = (intToString(it->m_length).size()+6)>typeSize? (intToString(it->m_length).size()+6): typeSize;
	}
	size_t maxSize = 3+fieldSize + 3+typeSize + (3+7) + (3+8) + (3+6) + (3+5) +1;// primary,not null,unique,index

	cout << "+-";
	for (size_t i = 0; i <= fieldSize; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i <= typeSize; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 8; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 9; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 7; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 6; i ++)
		cout << "-";
	cout << "+" << endl;

	cout << "| Field";
	for(size_t i = 5; i < fieldSize; i ++)
		cout << " ";
	cout << " | Type";
	for(size_t i = 4; i < typeSize; i ++)
		cout << " ";
	cout << " | Primary | Not null | Unique | Index |" << endl;
	
	cout << "+-";
	for (size_t i = 0; i <= fieldSize; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i <= typeSize; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 8; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 9; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 7; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 6; i ++)
		cout << "-";
	cout << "+" << endl;

	for (auto it = table.m_attr.begin(); it != table.m_attr.end(); it ++)
	{
		cout << "| ";
		cout << it -> m_name;
		for(size_t i = it->m_name.size();  i < fieldSize; i ++)
		{
			cout << " ";
		}

		cout << " | ";
		switch(it -> m_type)
		{
			case DATA_TYPE::INT:
				cout << "int";
				for(size_t i = 3; i < typeSize; i ++)
					cout << " ";
				break;
			case DATA_TYPE::FLOAT:
				cout << "float";
				for(size_t i = 5; i < typeSize; i ++)
					cout << " ";
				break;
			case DATA_TYPE::CHAR:
				cout << "char(" << intToString(it->m_length) << ")";
				for(size_t i = 6+intToString(it->m_length).size(); i < typeSize; i ++)
					cout << " ";
				break;

		}
		
		if( it -> isPrimary() )
			cout << " | Yes    ";
		else
			cout << " | No     ";

		if ( it -> isNotNull() )
			cout << " | Yes     ";
		else
			cout << " | No      ";

		if ( it -> isUnique() )
			cout << " | Yes   ";
		else
			cout << " | No    ";

		if ( it -> isIndex() )
			cout << " | Yes   |" << endl;
		else
			cout << " | No    |" << endl;
	}

	cout << "+-";
	for (size_t i = 0; i <= fieldSize; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i <= typeSize; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 8; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 9; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 7; i ++)
		cout << "-";
	cout << "+-";
	for (size_t i = 0; i < 6; i ++)
		cout << "-";
	cout << "+" << endl << endl;

}

void Shell::printHorizontal(const size_t size)
{
	cout << "+-";
	for (size_t i = 0; i < size; i ++)
		cout << "-";
	cout << "-+" << endl;
}

void Shell::onShowIndex()
{
//	cout << "show Index" << endl;
	if ( CatalogMan::instance().m_table.count(m_tableName) == 0 )
	{
		throw string("Table ") + m_tableName + string(" doesn't exist");	
	}
	int tableLength = m_tableName.size()>5? m_tableName.size(): 5;
	auto it = CatalogMan::instance().m_index.find(m_tableName);
	size_t num = CatalogMan::instance().m_index.count(m_tableName);
	size_t indexLength = 10; // "Index Name"
	size_t columnLength = 11; // "Column Name
	for (size_t i = 0; i < num; i ++, it ++)
	{
		indexLength = it->second.first.size()>indexLength? it->second.first.size(): indexLength;
		columnLength = it->second.second.size()>columnLength? it->second.second.size(): columnLength;
	}
	cout.fill('-');
	cout.width(tableLength+3);
	cout << left << '+';
	cout.width(indexLength+3);
	cout << left << '+';
	cout.width(columnLength+3);
	cout << left << '+';
	cout << '+' << endl;

	cout << "| ";
	cout.fill(' ');
	cout.width(tableLength+1);
	cout << "Table" << "| ";
	cout.width(indexLength+1);
	cout << "Index Name" << "| ";
	cout.width(columnLength+1);
	cout << "Column Name" << "|" << endl;

	cout.fill('-');
	cout.width(tableLength+3);
	cout << left << '+';
	cout.width(indexLength+3);
	cout << left << '+';
	cout.width(columnLength+3);
	cout << left << '+';
	cout << '+' << endl;

	cout.fill(' ');
	it = CatalogMan::instance().m_index.find(m_tableName);
	for (size_t i = 0; i < num; i ++, it ++ )
	{
		cout << "| ";
		cout.width(tableLength);
		cout << m_tableName << " | ";
		cout.width(indexLength);
		cout << it -> second.first << " | ";
		cout.width(columnLength);
		cout << it -> second.second << " |" << endl;
	}

	cout.fill('-');
	cout.width(tableLength+3);
	cout << left << '+';
	cout.width(indexLength+3);
	cout << left << '+';
	cout.width(columnLength+3);
	cout << left << '+';
	cout << '+' << endl;		

}
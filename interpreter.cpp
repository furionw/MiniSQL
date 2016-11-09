#include <regex>
#include <set>
#include "global.h"
#include "interpreter.h"
#include "api.h"
#include "shell.h"

using namespace std;


// 都是体力活，用正则表达式描述query指令，当regex_match返回为真时
// 将参数smatch result传入onXXX进一步解析，返回QUERY_TYPE
QUERY_TYPE Interpreter::parse(const string& cmd)
{
/*	regex pattern; //("/|\"|\\[|\\]|\\||\\{|\\}|\\$|\\?|@");
	const sregex_token_iterator illegalTest(cmd.begin(), cmd.end(), pattern);
	if ( illegalTest != sregex_token_iterator() )
	{
		throw string("You have an error in your SQL syntax: illegal character");
	}*/

	regex pattern("[^\\s]");
	const sregex_token_iterator itToken(cmd.begin(), cmd.end(), pattern); 
	if (itToken == sregex_token_iterator())
	{
		throw string("Query was empty");
	} 

	smatch result;
	pattern = regex("\\s*create\\s+table\\s+(\\w+)\\s*\\((.*)\\)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onCreateTable( result );
	}

	pattern = regex("\\s*drop\\s+table\\s+(\\w+)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onDropTable( result );
	}

	pattern = regex("\\s*create\\s+index\\s+(\\w+)\\s+on\\s+(\\w+)\\s*\\(\\s*(\\w+)\\s*\\)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onCreateIndex( result );
	}

	pattern = regex("\\s*drop\\s+index\\s+(\\w+)\\s+on\\s+(\\w+)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onDropIndex( result );
	}

	pattern = regex("\\s*select\\s*\\*\\s*from\\s+(\\w+)\\s*(\\s+where\\s+.+)?", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onSelectTuple( result );
	}

	pattern = regex("\\s*insert\\s+into\\s+(\\w+)\\s+values\\s*\\((.*)\\)\\s*", regex::icase);	
	if ( regex_match(cmd, result, pattern) )
	{
		return onInsertTuple( result );
	}

	pattern = regex("\\s*delete\\s+from\\s+(\\w+)\\s*(\\s+where\\s+.+)*", regex::icase);	
	if ( regex_match(cmd, result, pattern) )
	{
		return onDeleteTuple( result );
	}

	pattern = regex("\\s*quit\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onQuit( result );
	}

	pattern = regex("\\s*execfile\\s+(.+)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return onExecfile( result );
	}

	pattern = regex("\\s*show\\s+tables\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		return QUERY_TYPE::SHOW_TABLES;
	}

	pattern = regex("\\s*desc\\s+(\\w+)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		Shell::m_tableName = result[1];
		return QUERY_TYPE::DESC;
	}

	pattern = regex("\\s*show\\s+index\\s+from\\s+(\\w+)\\s*", regex::icase);
	if ( regex_match(cmd, result, pattern) )
	{
		Shell::m_tableName = result[1];
		return QUERY_TYPE::SHOW_INDEX;
	}

	throw string("You have an error in your SQL syntax: no matching query");
}

QUERY_TYPE Interpreter::onDropTable(const smatch& cmd)
{
	API::instance().dropTable( cmd[1] );
	return QUERY_TYPE::DROP_TABLE;
}

QUERY_TYPE Interpreter::onCreateIndex(const smatch& cmd)
{
	API::instance().createIndex( cmd[1], cmd[2], cmd[3] );
	return QUERY_TYPE::CREATE_INDEX;
}

QUERY_TYPE Interpreter::onDropIndex(const smatch& cmd)
{
	API::instance().dropIndex(cmd[1], cmd[2]);
	return QUERY_TYPE::DROP_INDEX;
}

QUERY_TYPE Interpreter::onSelectTuple(const smatch& cmd)
{
	API::instance().selectTuple(Query(cmd[1], getVtCondition(cmd)));

	return QUERY_TYPE::SELECT_TUPLE;
}

QUERY_TYPE Interpreter::onDeleteTuple(const smatch& cmd)
{
	API::instance().deleteTuple(Query(cmd[1], getVtCondition(cmd)));
	return QUERY_TYPE::DELETE_TUPLE;
}

QUERY_TYPE Interpreter::onQuit(const smatch& noUse)
{
	time_t end = time(0);
	exit(0);
	return QUERY_TYPE::QUIT;  // soft compilor
}

QUERY_TYPE Interpreter::onExecfile(const smatch& cmd)
{
	Shell::readFromFile( cmd[1] );
	return QUERY_TYPE::EXECFILE;
}

QUERY_TYPE Interpreter::onCreateTable(const smatch& cmd)
{
	if ( cmd[2].length() == 0 )
	{
		throw string("You have an error in your SQL syntax: a table must have at least 1 column");
	}

	vector<Attribute> vtAttr;
	string str = cmd[2];
	vector<string> vtAttrStr;

	size_t begin = 0;
	size_t end = 0;
	while ( end < str.size() )
	{
		if ( str[end] == ',' )
		{
			vtAttrStr.push_back( str.substr(begin, end - begin) );
			begin = ++ end;
			continue;
		}
		++ end;
	}
	vtAttrStr.push_back( str.substr(begin, end - begin) );

	/*	char* str2 = new char[str.size()+1];
	strcpy(str2, str.c_str());
	char *p = strtok(str2, ",");  
	while (p != NULL)
	{
	vtAttrStr.push_back(p);
	p = strtok(NULL, ",");
	}*/

	set<string> duplicateTest;
	smatch result;
	regex pattern;// = regex("\\s*(\\w+\\s+int|\\w+\\s+float|\\w+\\s+char\\(\\d+\\))\\s*", regex::icase);
	for (auto it = vtAttrStr.begin(); it != vtAttrStr.end(); it ++)
	{
		pattern = regex("\\s*(\\w+)\\s+int(\\s+unique)?\\s*", regex::icase);
		if ( regex_match(*it, result, pattern) )
		{
			if( duplicateTest.count( result[1] ) == 1 )
			{
				throw string("You have an error in your SQL syntax: duplicate attribute name");
			} else
			{
				duplicateTest.insert( result[1] );
			}
			Attribute a(result[1], DATA_TYPE::INT, 11, 0x00);
			if ( result[2].matched == true ) // total, attrName, unique
			{
				a.setUnique();
			}
			vtAttr.push_back(a);
			continue; 
		}

		pattern = regex("\\s*(\\w+)\\s+float(\\s+unique)?\\s*", regex::icase);
		if ( regex_match(*it, result, pattern) )
		{
			if( duplicateTest.count( result[1] ) == 1 )
			{
				throw string("You have an error in your SQL syntax: duplicate attribute name");
			} else
			{
				duplicateTest.insert( result[1] );
			}
			Attribute a(result[1], DATA_TYPE::FLOAT, 11, 0x00);
			if ( result[2].matched == true ) // total, attrName, unique
			{
				a.setUnique();
			}
			vtAttr.push_back(a);
			continue; 
		}

		pattern = regex("\\s*(\\w+)\\s+char\\s*\\((\\d+)\\)\\s*(unique)?\\s*", regex::icase);
		if ( regex_match(*it, result, pattern) )
		{
			if( duplicateTest.count( result[1] ) == 1 )
			{
				throw string("You have an error in your SQL syntax: duplicate attribute name");
			} else
			{
				duplicateTest.insert( result[1] );
			}
			Attribute a(result[1], DATA_TYPE::CHAR, stringToInt(result[2]), 0x00);
			if ( result[3].matched == true ) // total, attrName, unique
			{
				a.setUnique();
			}
			vtAttr.push_back(a);
			continue; 
		}

		pattern = regex("\\s*primary\\s+key\\s*\\(\\s*(\\w+)\\s*\\)", regex::icase);
		if ( regex_match(*it, result, pattern) )
		{
			if ( duplicateTest.count( result[1] ) != 1 )
			{
				throw string("You have an error in your SQL syntax: no such attribute exist when trying to declare a primary key");
			}
			for (auto it = vtAttr.begin(); it != vtAttr.end(); it ++)
			{
				if ( it -> m_name == result[1] )
				{
					it -> setPrimary();
				}
			}
			continue;
		}

		throw string("You have an error in your SQL syntax: no such attribute description style");
	}

	//	delete str2;
	API::instance().createTable(cmd[1], vtAttr);
	return QUERY_TYPE::CREATE_TABLE;
}

QUERY_TYPE Interpreter::onInsertTuple(const smatch& cmd)
{
	vector<string> vtTupleStr;
	Table table = CatalogMan::instance().getTable(cmd[1]);
	string str = cmd[2];

	//	char* str2 = new char[str.size() + 1];
	//	strcpy( str2, str.c_str() );
	//	char* value = strtok( str2, "," );

	size_t cnt = 0;
	size_t begin = 0;
	size_t end = 0;
	while ( end < str.size() )
	{
		if ( str[end] == ',' )
		{
			if ( cnt >= table.m_attr.size() )
			{
				throw string("Unmatching attribute number when trying to insert: redundant insert columns."); // too many columns
			}
			Attribute a = table.m_attr[cnt];
			vtTupleStr.push_back( inputTest(a, str.substr(begin, end - begin)) );
			begin = ++ end;
			cnt ++;
			continue;
		}
		end ++;
	}
	Attribute a = table.m_attr[cnt];
	vtTupleStr.push_back( inputTest(a, str.substr(begin, end - begin)) );
	cnt ++;
	/*	while (value != NULL)
	{
	if ( cnt >= table.m_attr.size() )
	{
	throw string("Unmatching attribute number when trying to insert: redundant insert columns."); // too many columns
	}
	Attribute a = table.m_attr[cnt];
	vtTupleStr.push_back( inputTest(a, value) );
	value = strtok(NULL, ",");
	cnt ++;
	}*/
	if (cnt != table.m_attr.size() )
	{
		throw string("Unmatching attribute number when trying to insert: insufficient insert columns");		
	}
	API::instance().insertTuple(cmd[1], vtTupleStr);
	//	delete str2;

	return QUERY_TYPE::INSERT_TUPLE;	
}

// return input after parsing
string Interpreter::inputTest(const Attribute& attr, const string& input)
{
	if ( attr.m_type == DATA_TYPE::INT )
	{
		regex pattern("[^0-9 ]");
		if ( regex_search(input, pattern) )
		{
			throw string("Unmatching type in where clause: integer is required.");
		}
		return intToString( stringToInt(input) ); // 低效地把空格去掉= =...
	}

	if ( attr.m_type == DATA_TYPE::FLOAT )
	{
		regex pattern("[^0-9. ]");
		if ( regex_search(input, pattern) )
		{
			throw string("Unmatching type in where clause: float is required.");
		}
		return floatToString( stringToFloat(input) ); // 低效地把空格去掉= =...
	}

	if ( attr.m_type == DATA_TYPE::CHAR )
	{
		regex pattern("\\s*'(.*)'\\s*");
		smatch result;
		if ( regex_search(input, result, pattern) == false )
		{
			throw string("Unmatching type in where clause: string is required.");
		}
		if ( result[1].length() > attr.m_length ) // 两个分号
		{
			throw string("Length exceed in where clause.");
		}
		return result[1];
	}
	throw string("In inputTest to soft the compilor.");
}

vector<Condition> Interpreter::getVtCondition(const smatch& cmd) // called by select&deleteTuple to parse the where clause
{
	Table table = CatalogMan::instance().getTable( cmd[1] );
	string str = cmd[2];
	vector<Condition> vtCon;
	size_t index = str.find( "where" );
	if ( index == string::npos )	return vtCon;
	bool findAnd = true;
	bool findAttr = false;
	bool findCom = false;
	bool findOpB = false;
	string strA;
	OP_TYPE opType;
	string strB;
	for ( index += 5; index < str.size(); )
	{
		if ( str[index] == ' ' )
		{
			index ++;
			continue;
		}

		if ( findAnd == false ) 
		{
			if ( (index+2)>str.size() || str.substr(index, 3) != "and" )
			{
				throw string("and is needed in where clause.");
			}
			findAnd = true;
			index += 3;
			continue;
		}

		if ( findAttr == false )
		{
			size_t mark = index;
			while ( index < str.size() && str[index] != ' ' && str[index] != '='
				&& str[index] != '<' && str[index] != '>')
			{
				index ++;
			}
			if (mark == index)
			{
				throw string("Illegal attribute name.");
			}
			strA = str.substr(mark, index - mark);
			findAttr = true;
			continue;
		}

		if ( findCom == false )
		{
			const char first = str[index ++]; // 写成const就不会出现 if ( first =  )这种乌龙了..
			//			= != < > <= >=
			if ( first == '=' )
			{
				opType = OP_TYPE::EQUAL;
				findCom = true;
				continue;
			} /*else if ( first == '!' )
			  {
			  if ( index <str.size() && str[index] == '=' )
			  {
			  opType = OP_TYPE::NO_EQUAL;
			  findCom = true;
			  index ++;
			  continue;
			  }
			  }*/ else if ( first == '<' )
			{
				findCom = true;
				opType = OP_TYPE::LESS;
				if ( index < str.size() && str[index] == '=' )
				{
					opType = OP_TYPE::NO_GREATER;
					index ++;
				} else if ( index < str.size() && str[index] == '>' )
				{
					opType = OP_TYPE::NO_EQUAL;
					index ++;
				}
				continue;
			} else if  ( first == '>' )
			{
				findCom = true;
				opType = OP_TYPE::GREATER;
				if ( index < str.size() && str[index] == '=' )
				{
					opType = OP_TYPE::NO_LESS;
					index ++;
				}
				continue;
			}
			throw string("Illegal operator in where clause.");
		}

		if ( findOpB == false )
		{
			size_t mark = index;
			while(index < str.size() && str[index] != ' ')
			{
				index ++;
			}
			if (mark == index)
			{
				throw string("Lack of opearnd in where clause.");
			}
			Attribute a = table.getAttribute(strA);
			strB = inputTest(a,  str.substr(mark, index - mark));
			vtCon.push_back( Condition(strA, opType, strB) );
			findAnd = false;
			findAttr = false;
			findCom = false;
			continue;
		}
	}

	if ( findAnd )
	{
		throw string("Redundant 'and' in where clause.");
	}
	if ( findAttr || findCom )
	{
		throw string("Illegal constrain in where clause.");
	}

	return vtCon;
}
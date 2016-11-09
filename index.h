#pragma once

#include <vector>
#include <string>
#include <map>
#include "BPlusTreeV5.h"
#include "../miniSQL/global.h"

//class Table;
//class Condition;

class IndexMan
{
public:
	static IndexMan& instance();

	// called by API::dropTable
	void dropIndex(const std::string& tableName, const std::vector<std::string>& vtAttrName) const; 
	// called by API::dropIndex
	void dropIndex(const std::string& tableName, const std::string& attrName) const;
	// called by API::insertTuple
	void insertIndex(const std::string& tableName, const Attribute& attr, const std::vector<std::pair<std::string, int>>& info) const; 
	// called by API::insertTuple, throw string("Duplicate values on primary or unique key 'xxx'");
	void insertTupleCheck(const Table& table, const std::vector<std::string>& strTuple) const; 
	// called by API::deleteTuple
	void deleteIndex(const std::string& tableName, const Attribute& attr, const std::vector<std::pair<std::string, int>>& info) const; 
	// called on "select* from" or "delete from"
	std::vector<int> find(const Table& table, const Condition& condition) const;
	// for dbg
	void print(const std::string& tableName, const Attribute& attr) const;
private:
	IndexMan() {}
	IndexMan(const IndexMan&);
	IndexMan& operator=(const IndexMan&);
	~IndexMan() {}
	template<typename KEY> void treeInsert(const BPlusTree<KEY>& tree, const std::vector<std::pair<std::string, int>>& info)	const;
	template<typename KEY> void treeRemove(const BPlusTree<KEY>& tree, const std::vector<std::pair<std::string, int>>& info) const;
	const static std::string M_DIR;
};

inline IndexMan& IndexMan::instance()
{
	static IndexMan obj;
	return obj;
}

template<typename KEY>
void IndexMan::treeInsert(const BPlusTree<KEY>& tree, const std::vector<std::pair<std::string, int>>& info) const
{
	for( auto it = info.begin(); it != info.end(); it ++ )
	{	
		tree.insert( it -> first, it -> second );
//		tree.print();
	}
}

template<typename KEY>
void IndexMan::treeRemove(const BPlusTree<KEY>& tree, const std::vector<std::pair<std::string, int>>& info) const
{
	for( auto it = info.begin(); it != info.end(); it ++ )
	{
		tree.remove( it -> first, it -> second );
	}
//	tree.print();
}

// API::createTable
//	void createIndex(const std::string& tableName, const std::vector<Attribute>& attr) const; 
// API::createIndex
//	void createIndex(const std::string& tableName, Attribute& attr) const; 
#ifndef RECORD_H_
#define RECORD_H_

#include "global.h"
#include "buffer.h"
#include <vector>
#include <string>
#include <map>

class Tuple;

class RecordMan
{
public:
	static RecordMan& instance();
	// ������Ӧ��record���ļ�
	void createTable(const std::string& tableName) const;
	// ɾȥ��Ӧ��record�ļ�
	void dropTable(const std::string& tableName) const;
	// ��ָ�������ѡ�����Լ����tuple
	std::vector<Tuple> selectTuple(const Table& table, const std::vector<Condition>& condition, const int blockNum) const;
	// ����ָ��ƫ��ѡ�����Լ����tuple	
	std::vector<Tuple> selectTuple(const Table& table, const std::vector<Condition>& condition, const std::vector<int>& vtOffset) const;
	// ��create index xxx on tableName(attrName)ʱ������ָ������ڵ�<key,offset>
	std::vector<std::pair<std::string, int>> getOffsetInfo(const Table& table, const std::string& attrName, const int blockNum) const;
	// ��strTuple����record�ļ�β��������Ӧ��offset
	int insertTuple(const Table& table, const std::vector<std::string>& strTuple) const;
	// ��ָ�������ɾ������Լ����tuple
	std::vector<std::pair<Tuple,int>> deleteTuple(const Table& table, const std::vector<Condition>& condition, const std::vector<int>& vtOffset) const;
	// ����ָ��ƫ��ɾ������Լ����tuple	
	std::vector<std::pair<Tuple,int>> deleteTuple(const Table& table, const std::vector<Condition>& condition) const;

	int getBound(const std::string& tableName) const { std::string noUse; return BufferMan::instance().readLast(M_DIR + tableName, noUse); }
private:
	std::vector<Tuple> unpackOnSelect(const Table& table, const std::string& content, const std::vector<Condition>& condition) const;

	void spliceOnSelect(std::vector<Tuple>& tupleOut, const Table& table, const std::vector<Condition>& condition, const std::vector<int>& vtOffset) const;	

	void spliceOnDelete(std::vector<std::pair<Tuple, int>>& ret, const Table& table, std::string& content, const std::vector<Condition>& condition, const int blockNum) const;

	void spliceOnDelete(std::vector<std::pair<Tuple, int>>& ret, const Table& table, std::string& content, const std::vector<Condition>& condition, const std::vector<int> vtOffset) const;

	int appendTupleAtRear(const Table& table, std::string& content, const int blockNum, const std::vector<std::string>& strTuple) const;

	void incrementCnt(std::string& content);

	std::vector<Tuple> unpackContent(const Table& table, const std::string& content) const;

	std::vector<Tuple> unpackContent(const Table& table, const std::string& content, const std::vector<int>& offsetWithinBlock) const;

	void conditionTest(const Table& table, std::vector<Tuple>& vtTuple, const std::vector<Condition>& condition) const;

	int getCnt(const std::string& content) const { return fourBytesToInt(content, 0, M_HEAD_SIZE); }

	bool hasSpace(const Table& table, const std::string& content) const;

	bool isValid(char validBit) const { return validBit == M_VALID;}

	void cpy(std::string& content, const int begin, const std::string& original) const;

	void incrementCnt(std::string& content) const;

	RecordMan() {}
	RecordMan(const RecordMan&);
	RecordMan& operator=(const RecordMan&);
	~RecordMan() {}

	static const std::string M_DIR;
	static const char M_VALID;
	static const int M_HEAD_SIZE;
};

inline RecordMan& RecordMan::instance()
{
	static RecordMan obj;
	return obj;
}

#endif  // RECORD_H_

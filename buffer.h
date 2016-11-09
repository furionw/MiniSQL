#pragma once

#include <string>
#include <map>
#include <list>
//#include "../miniSQL/global.h"
#include "global.h"

// V1: 10-12
// V2: 10-19, ���readLast\ remove����(for RecordMan and so on); 
//                    �޸�write�ķ������ͣ�����д���blockNum(for indexMan)
// V3: 10-20, ����ofstream�Ĵ򿪷�ʽ��������ios::binary
//        10-23, ��replace, newBlock�����еĵ�����������char*��Ϊconst std::string&
// V4: 10-29, ���remove�ӿڣ�����API::dropIndex, API::dropTableʱɾ����Ӧ�ļ������m_used, m_map����Ӧ������

class BufferMan
{
	// nested classes
	class Block;
	class File;
public:
	// singleton
	static BufferMan& instance();
	// ��ָ����ŵ�����
	bool read(const std::string& fileName, const int blockNum, std::string& strOut);
	// blockNumΪ1ʱ��д׷�ӵ��ļ�β����֮д��ָ�����С�����д��Ŀ��
	int write(const std::string& fileName, const std::string& content, const int blockNum = -1);
	// ���ļ������һ�飬���ؿ��
	int readLast(const std::string& fileName, std::string& strOut);
	// �Ƴ�m_used, m_map����fileName�йص�����
	void remove(const std::string& fileName);
	// pin
	void pin(const std::string& fileName, const int blockNum);

private:
	BufferMan();
	~BufferMan();
	BufferMan(const BufferMan&);
	BufferMan& operator=(const BufferMan&);
	void init();
	bool isFull();
	std::string getKey(const std::string& fileName, const int blockNum) const;
	bool isInBuffer(const std::string& fileName, const int blockNum);
	bool readBlock(const std::string& fileName, const int blockNum, std::string& strOut);
	bool fileToBlock(const std::string& fileName, const int blockNum, std::string& strOut);
	bool writeBlock(const std::string& fileName, const int blockNum, const std::string& content);
	bool replace(const std::string& fileName, const int blockNum, const std::string& content);
	bool newBlock(const std::string& fileName, const int blockNum, const std::string& content);
	void removeInMap(const std::string& fileName);
	void removeInList(const std::string& fileName);

	std::list<Block*> m_used;
	std::map<std::string, Block*> m_map;

	class Block
	{
	public:
		Block();
		void setPin() { m_pin = true; }
		bool isDirty() const;
		bool isPin() const;
		bool writeToFile() const;
		void update(const std::string& fileName, const int blockNum, const std::string& content);
		std::string getKey() const;

		bool m_pin;
		bool m_dirty;
		int m_blockNum;
		std::string m_content;
		std::string m_fileName;
	};

	class File
	{
	public:
		static bool read(const std::string& fileName, const int blockNum, char*& dst);
		static bool write(const Block* cur);
		static bool write(const std::string& fileName, const std::string& content, int& num);
		static void createFile(const std::string& fileName);
		static int readLast(const std::string& fileName, std::string& strOut);
	};
};

inline BufferMan& BufferMan::instance()
{
	static BufferMan obj;
	return obj;
}




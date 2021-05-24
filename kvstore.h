#pragma once

#include "kvstore_api.h"
#include "skiplist.h"
#include "index.h"
#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"
#include "buffer.h"

class KVStore : public KVStoreAPI
{
	// You can add your implementation here
private:
	std::string root;
	Skiplist *memTable;
	Index *index;
	Buffer *buffer;
	vector<int> levelFilesNum; //每层文件数目
	uint64_t maxTimeStamp;

	void initialize();

	int memTableSize(); //返回若将当前MemTable转成SSTable，其数据量大小

	void saveToDisk();

	std::string findInSSTable(uint64_t key); //在SSTable中找key对应的value

	std::string getLevelPath(int level);
	std::string generateLevel(int level);
	std::string getSSTablePath(int level, int i);
	void compact(int level);				   //合并第level行（合并完会检查第level+1行，根据情况递归调用）
	bool SSTableFileExists(int level, int id); //检查对应层的对应id文件名的SSTable文件是否存在

public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;
};

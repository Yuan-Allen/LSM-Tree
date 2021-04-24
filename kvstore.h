#pragma once

#include "kvstore_api.h"
#include "skiplist.h"
#include "index.h"
#include <iostream>
#include <fstream>
#include <string>
#include "utils.h"

class KVStore : public KVStoreAPI
{
	// You can add your implementation here
private:
	std::string root;
	Skiplist *memTable;
	Index *index;
	vector<int> levelFilesNum; //每层文件数目
	uint64_t maxTimeStamp;

	void initialize();

	int memTableSize(); //返回若将当前MemTable转成SSTable，其数据量大小

	void saveToDisk();

	std::string findInSSTable(uint64_t key); //在SSTable中找key对应的value

	std::string getLevelPath(int level);
	std::string generateLevel(int level);
	std::string getSSTablePath(int level, int i);

public:
	KVStore(const std::string &dir);

	~KVStore();

	void put(uint64_t key, const std::string &s) override;

	std::string get(uint64_t key) override;

	bool del(uint64_t key) override;

	void reset() override;
};

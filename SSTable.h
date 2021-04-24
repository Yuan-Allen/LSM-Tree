#ifndef SSTABLE_H
#define SSTABLE_H

#include <cstdint>
#include <vector>

const int BFSIZE = 81920;

struct IndexNode
{
    uint64_t key;
    uint32_t offset;
    IndexNode(uint64_t k, uint32_t o) : key(k), offset(o) {}
    IndexNode() : key(0), offset(0) {}
};

class SSTable
{
private:
    int level;
    int id; //在该层的编号
    uint64_t timeStamp;
    std::vector<bool> bloomFilter;
    std::vector<IndexNode> index;

    uint32_t searchKey(uint64_t key); //二分查找key，若存在返回offset，不存在返回0

public:
    SSTable(int l, int i, uint64_t time, std::vector<bool> BF, std::vector<IndexNode> ind);
    ~SSTable();
    uint32_t hasKey(uint64_t key); //判断该SSTable中是否存在key，若存在返回offset，不存在返回0
    uint64_t getTimeStamp();
    int getLevel();
    int getId();
};

#endif
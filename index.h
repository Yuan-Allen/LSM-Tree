#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include "SSTable.h"
#include <fstream>

class Index
{
private:
    std::vector<SSTable *> index;

public:
    Index();
    ~Index();
    SSTable *readFile(int l, int i, std::fstream *in);
    SSTable *addToIndex(int l, int i, uint64_t time, std::vector<bool> BF, std::vector<IndexNode> ind);
    SSTable *search(uint64_t key, uint32_t &offset); //由key返回对应的SSTable的指针并且设置offset到参数
    void clear();
};

#endif
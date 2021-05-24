#ifndef INDEX_H
#define INDEX_H

#include <vector>
#include "SSTable.h"
#include <fstream>

class Index
{
private:
    std::vector<std::vector<SSTable *>> index;

public:
    Index();
    ~Index();
    SSTable *readFile(int l, int i, std::fstream *in);
    SSTable *addToIndex(int l, int i, uint64_t time, std::vector<bool> BF, std::vector<IndexNode> ind);
    SSTable *search(uint64_t key, uint32_t &offset);                                  //由key返回对应的SSTable的指针并且设置offset到参数
    void deleteFileIndex(int level, int i);                                           //删除文件后，也要在索引里删除
    std::vector<int> findIntersectionId(int level, uint64_t minKey, uint64_t maxKey); //返回level层键值与minKey到maxKey有交集的所有SSTable的id，并在index里删除这些索引，重排id
    void clear();
    void changeIndex(int level, int oldId, int id); //更新level层的oldId的文件id为id
};

#endif
#ifndef BUFFER_H
#define BUFFER_H

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include "SSTable.h"

struct dataNode
{
    uint64_t key;
    std::string value;
    dataNode(uint64_t k, std::string v) : key(k), value(v) {}
};

struct dataList
{
    std::vector<dataNode> data;
    uint64_t timeStamp;
    dataList(std::vector<dataNode> d, uint64_t t) : data(d), timeStamp(t) {}
};

class Buffer
{
private:
    std::vector<dataList> datas;  //要被合并的SSTable的数据暂存于此
    std::vector<dataNode> output; //归并排序后的输出数据
    uint64_t timeStamp;           //合并后新SSTable的时间戳（就是datas里最大的时间戳）

    std::vector<dataNode> get2MVector(std::vector<bool> &BF); //从output中获取生成的SSTable不超过2M的最大数据量的数据，并在参数中填入生成的BloomFilter，并从output中删除这部分数据
public:
    Buffer(/* args */);
    ~Buffer();
    void readFile(std::fstream *in); //把要合并的SSTable读入datas
    void compact(bool flag);         //flag为true表示下一层为空，合并时需要删除~DELETED~
    void clear();
    void write(std::fstream *out); //以SSTable形式输出2M数据
    uint64_t getMinKey();
    uint64_t getMaxKey();
    bool isOutputEmpty();
};

#endif
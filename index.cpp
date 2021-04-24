#include "index.h"

Index::Index()
{
}

Index::~Index()
{
}

SSTable *Index::readFile(int l, int i, std::fstream *in)
{
    //读取时间戳
    uint64_t timeStamp;
    in->read((char *)&timeStamp, sizeof(timeStamp));

    //读取键值对数量
    uint64_t size = 0;
    in->read((char *)&size, sizeof(size));

    //读取BloomFilter
    in->seekg(32, std::ios::beg); //移动读写位置到32字节处
    std::vector<bool> BF(BFSIZE, 0);
    char b;
    for (size_t i = 0; i < BFSIZE; i += 8) //一次读一个字节，所以i+=8;
    {
        in->read(&b, sizeof(b));
        for (size_t j = 0; j < 8; j++)
        {
            if (b & (1 << (7 - j)))
            {
                BF[i + j] = 1;
            }
        }
    }

    //读取索引区
    std::vector<IndexNode> ind;
    uint64_t key;
    uint32_t offset;
    for (size_t i = 0; i < size; i++)
    {
        in->read((char *)&key, sizeof(key));
        in->read((char *)&offset, sizeof(offset));
        ind.push_back(IndexNode(key, offset));
    }

    //记录到index
    return addToIndex(l, i, timeStamp, BF, ind);
}

SSTable *Index::addToIndex(int l, int i, uint64_t time, std::vector<bool> BF, std::vector<IndexNode> ind)
{
    SSTable *tmp = new SSTable(l, i, time, BF, ind);
    index.push_back(tmp);
    return tmp;
}

SSTable *Index::search(uint64_t key, uint32_t &offset)
{
    SSTable *result = nullptr;
    uint32_t tmpOffset;
    for (size_t i = 0; i < index.size(); i++)
    {
        if ((tmpOffset = index[i]->hasKey(key)) > 0)
        {
            if ((!result) || (result && (index[i]->getTimeStamp() > result->getTimeStamp()))) //选时间戳最大的
            {
                result = index[i];
                offset = tmpOffset;
            }
        }
    }
    return result;
}

void Index::clear()
{
    for (size_t i = 0; i < index.size(); i++)
    {
        delete index[i];
    }
}
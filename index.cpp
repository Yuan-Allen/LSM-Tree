#include "index.h"

Index::Index()
{
}

Index::~Index()
{
    clear();
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
    while (l > (int)(index.size() - 1))
    {
        index.push_back(std::vector<SSTable *>());
    }
    //index[l].push_back(tmp);
    bool flag = false;
    for (std::vector<SSTable *>::iterator it = index[l].begin(); it != index[l].end(); it++)
    {
        if ((*it)->getId() > i)
        {
            index[l].insert(it, tmp);
            flag = true;
            break;
        }
    }
    if (!flag)
        index[l].push_back(tmp);

    return tmp;
}

SSTable *Index::search(uint64_t key, uint32_t &offset)
{
    SSTable *result = nullptr;
    uint32_t tmpOffset;
    bool flag = false;
    for (size_t i = 0; i < index.size(); i++)
    {
        for (size_t j = 0; j < index[i].size(); j++)
        {
            if ((tmpOffset = index[i][j]->hasKey(key)) > 0)
            {
                if ((!result) || (result && (index[i][j]->getTimeStamp() > result->getTimeStamp()))) //选时间戳最大的
                {
                    result = index[i][j];
                    offset = tmpOffset;
                    flag = true;
                }
            }
        }
        if (flag)
            break;
    }
    return result;
}

void Index::clear()
{
    for (size_t i = 0; i < index.size(); i++)
    {
        for (size_t j = 0; j < index[i].size(); j++)
        {
            delete index[i][j];
        }
        index[i].clear();
    }
    index.clear();
}

void Index::deleteFileIndex(int level, int i)
{
    for (std::vector<SSTable *>::iterator it = index[level].begin(); it != index[level].end(); it++)
    {
        if ((*it)->getId() == i)
        {
            index[level].erase(it);
            break;
        }
    }
}

std::vector<int> Index::findIntersectionId(int level, uint64_t minKey, uint64_t maxKey)
{
    std::vector<int> result;
    for (std::vector<SSTable *>::iterator it = index[level].begin(); it != index[level].end(); it++)
    {
        if (!((*it)->getMaxKey() < minKey || (*it)->getMinKey() > maxKey))
        {
            result.push_back((*it)->getId());
            index[level].erase(it);
            it--;
        }
    }
    int size = index[level].size();
    for (int i = 0; i < size; ++i)
    {
        index[level][i]->changeId(i);
    }
    return result;
}

void Index::changeIndex(int level, int oldId, int id)
{
    for (std::vector<SSTable *>::iterator it = index[level].begin(); it != index[level].end(); ++it)
    {
        if ((*it)->getId() == oldId)
        {
            (*it)->changeId(id);
            break;
        }
    }
}
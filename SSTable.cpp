#include "SSTable.h"
#include "MurmurHash3.h"

SSTable::SSTable(int l, int i, uint64_t time, std::vector<bool> BF, std::vector<IndexNode> ind)
{
    level = l;
    id = i;
    timeStamp = time;
    bloomFilter = BF;
    index = ind;
}

SSTable::~SSTable()
{
}

uint32_t SSTable::hasKey(uint64_t key)
{
    //用BloomFilter判断是否存在key
    unsigned int hash[4] = {0};
    MurmurHash3_x64_128(&key, sizeof(key), 1, hash);
    for (size_t i = 0; i < 4; i++)
    {
        if (!bloomFilter[hash[i] % BFSIZE])
        {
            return 0;
        }
    }
    return searchKey(key);
}

uint32_t SSTable::searchKey(uint64_t key)
{
    int left = 0, right = index.size() - 1, mid;
    while (left <= right)
    {
        mid = (left + right) / 2;
        if (index[mid].key == key)
        {
            return index[mid].offset;
        }
        if (key > index[mid].key)
        {
            left = mid + 1;
        }
        else
        {
            right = mid - 1;
        }
    }
    return 0;
}

uint64_t SSTable::getTimeStamp()
{
    return timeStamp;
}

int SSTable::getLevel()
{
    return level;
}

int SSTable::getId()
{
    return id;
}
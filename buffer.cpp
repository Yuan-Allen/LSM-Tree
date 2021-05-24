#include "buffer.h"
#include "MurmurHash3.h"

Buffer::Buffer(/* args */)
{
    clear();
}

Buffer::~Buffer()
{
}

void Buffer::readFile(std::fstream *in)
{
    //读取时间戳
    uint64_t time;
    in->read((char *)&time, sizeof(time));

    //读取键值对数量
    uint64_t size = 0;
    in->read((char *)&size, sizeof(size));

    //读取索引区
    in->seekg(32 + 10240, std::ios::beg);
    std::vector<IndexNode> ind;
    uint64_t key;
    uint32_t offset;
    for (size_t i = 0; i < size; i++)
    {
        in->read((char *)&key, sizeof(key));
        in->read((char *)&offset, sizeof(offset));
        ind.push_back(IndexNode(key, offset));
    }

    //记录到dataList
    std::vector<dataNode> tmpDatas;
    char buf[200000] = {0};
    std::string value = "";
    for (size_t i = 0; i < size; i++)
    {
        in->seekg(ind[i].offset, std::ios::beg);
        in->get(buf, 200000, '\0');
        value = buf;
        tmpDatas.push_back(dataNode(ind[i].key, value));
    }
    dataList tmpList(tmpDatas, time);
    datas.push_back(tmpList);
}

void Buffer::compact(bool flag)
{
    output.clear();
    uint64_t minKey = UINT64_MAX; //归并排序时，每一轮的最小值
    int target = 0;               //归并排序时被取值的目标文件（在datas中的索引）
    uint64_t maxTime = 0;         //每一轮归并排序时，取值的目标文件的时间戳
    timeStamp = 0;
    for (size_t i = 0; i < datas.size(); i++)
    {
        if (datas[i].timeStamp > timeStamp)
            timeStamp = datas[i].timeStamp;
    }

    while (!datas.empty())
    {
        minKey = UINT64_MAX;
        target = 0;
        maxTime = 0;

        int fileNum = datas.size();
        for (int i = 0; i < fileNum; i++)
        {
            dataNode data = datas[i].data.front();
            if (data.key < minKey)
            {
                minKey = data.key;
                target = i;
                maxTime = datas[i].timeStamp;
            }
            else if (data.key == minKey)
            {
                if (datas[i].timeStamp > maxTime)
                {
                    maxTime = datas[i].timeStamp;
                    datas[target].data.erase(datas[target].data.begin()); //键相同选择时间戳大的，所以原先文件里的该键值对被抛弃，可以直接删掉
                    target = i;
                }
                else
                {
                    datas[i].data.erase(datas[i].data.begin()); //否则该文件时间戳小，该键值对被抛弃，可以删掉
                }
            }
        }
        dataNode tmp = datas[target].data.front();
        if (!(flag && tmp.value == "~DELETED~"))
            output.push_back(tmp);
        datas[target].data.erase(datas[target].data.begin());
        fileNum = datas.size();
        for (int i = 0; i < fileNum; i++) //检查已经被读完的文件
        {
            if (datas[i].data.empty())
            {
                datas.erase(datas.begin() + i);
                i -= 1; //由于是用下标访问，每次循环i++, 不减一的话会跳过下一个元素
                fileNum -= 1;
            }
        }
    }
}

void Buffer::clear()
{
    datas.clear();
    output.clear();
    timeStamp = 0;
}

std::vector<dataNode> Buffer::get2MVector(std::vector<bool> &BF)
{
    BF.assign(BFSIZE, 0);
    const int MAXSIZE = 2 * 1024 * 1024; //2M
    int tmpSize = 32 + 10240;            //Header加BloomFilter的大小（Header为32Byte，BF为10240Byte）
    std::vector<dataNode> result;
    while (true)
    {
        if (output.empty())
            break;
        tmpSize += (8 + 4); //一对key和offset的大小
        tmpSize += output.front().value.size() + 1;
        if (tmpSize >= MAXSIZE)
            break;
        result.push_back(output.front());
        output.erase(output.begin());
        //BloomFilter
        unsigned int hash[4] = {0};
        MurmurHash3_x64_128(&result.back().key, sizeof(result.back().key), 1, hash);
        for (size_t i = 0; i < 4; i++)
        {
            BF[hash[i] % BFSIZE] = 1;
        }
    }
    return result;
}

void Buffer::write(std::fstream *out)
{
    if (output.empty())
        return;

    std::vector<bool> BF;
    std::vector<dataNode> data = get2MVector(BF);
    //写入时间戳
    out->write((char *)&timeStamp, sizeof(timeStamp));

    //写入键值对数量
    uint64_t size = data.size();
    out->write((char *)&size, sizeof(size));

    //写入键最小值和最大值
    uint64_t minKey = data.front().key;
    uint64_t maxKey = data.back().key;
    out->write((char *)&minKey, sizeof(minKey));
    out->write((char *)&maxKey, sizeof(maxKey));

    //写入BloomFilter
    for (size_t i = 0; i < BFSIZE; i += 8)
    {
        char b = 0; //每次一个byte一个byte的写入文件
        for (int j = 0; j < 8; ++j)
        {
            if (BF[i + j])
            {
                b = b | (1 << (7 - j)); //设置byte的位
            }
        }
        out->write(&b, sizeof(b));
    }

    //写入索引区
    uint32_t offset = 32 + 10240 + size * (8 + 4);
    for (size_t i = 0; i < size; i++)
    {
        out->write((char *)&(data[i].key), sizeof(data[i].key));
        out->write((char *)&offset, sizeof(offset));
        offset += data[i].value.size() + 1;
    }

    //写入数据区
    for (size_t i = 0; i < size; i++)
    {
        out->write((char *)(data[i].value.c_str()), data[i].value.size() + 1);
    }
}

uint64_t Buffer::getMinKey()
{
    uint64_t minKey = UINT64_MAX;
    int size = datas.size();
    for (int i = 0; i < size; i++)
    {
        if (datas[i].data.front().key < minKey)
        {
            minKey = datas[i].data.front().key;
        }
    }
    return minKey;
}

uint64_t Buffer::getMaxKey()
{
    uint64_t maxKey = 0;
    int size = datas.size();
    for (int i = 0; i < size; i++)
    {
        if (datas[i].data.back().key > maxKey)
        {
            maxKey = datas[i].data.back().key;
        }
    }
    return maxKey;
}

bool Buffer::isOutputEmpty()
{
    return output.empty();
}
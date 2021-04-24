#include "kvstore.h"

KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	root = dir;
	memTable = new Skiplist();
	index = new Index();
	maxTimeStamp = 0;
	initialize();
}

KVStore::~KVStore()
{
	saveToDisk();
	delete memTable;
	delete index;
}

void KVStore::initialize()
{
	std::string dirPath;
	int level;
	for (level = 0, dirPath = getLevelPath(level); utils::dirExists(dirPath); dirPath = getLevelPath(++level))
	{
		levelFilesNum.push_back(0);
		int i = 0;
		while (true)
		{
			std::string filePath = getSSTablePath(level, i);
			fstream *in = new fstream(filePath.c_str(), ios::binary | ios::in);
			if (!in->is_open())
			{
				in->close();
				break;
			}
			levelFilesNum[level]++;
			SSTable *tmp = index->readFile(level, i, in);
			if (tmp->getTimeStamp() >= maxTimeStamp)
			{
				maxTimeStamp = tmp->getTimeStamp() + 1;
			}
			i += 1;
			in->close();
		}
	}
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	if (memTableSize() + 8 + 4 + s.size() > 2 * 1024 * 1024) //如果插入后的大小大于2M，先把memTable写入SSTable(8和4分别是key和offset的大小)
	{
		saveToDisk();
	}
	memTable->insert(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	std::string result = "";
	//在memTable中找
	result = memTable->getValue(key);
	if (result == "~DELETED~")
	{
		return "";
	}
	if (result != "")
		return result;

	//在SSTable中找
	result = findInSSTable(key);
	return result;
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	std::string value;
	value = memTable->getValue(key);
	if (value == "~DELETED~")
	{
		return false;
	}
	if (value != "")
	{
		memTable->remove(key);
		put(key, "~DELETED~");
		return true;
	}
	value = findInSSTable(key);
	if (value == "~DELETED~" || value == "")
	{
		return false;
	}
	put(key, "~DELETED~");
	return true;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
	memTable->clear();
	index->clear();

	std::string dirPath;
	int level;
	for (level = 0, dirPath = getLevelPath(level); utils::dirExists(dirPath); dirPath = getLevelPath(++level))
	{
		std::vector<string> files;
		utils::scanDir(dirPath, files);
		for (std::vector<string>::iterator it = files.begin(); it != files.end(); ++it)
		{
			std::string filePath = dirPath + *it;
			utils::rmfile(filePath.c_str());
		}
		utils::rmdir(dirPath.c_str());
	}
}

int KVStore::memTableSize()
{
	int infoSize = 32 + 10240;					//Header加BloomFilter的大小（Header为32Byte，BF为10240Byte）
	int indexSize = memTable->size() * (8 + 4); //索引区的大小（key为8Byte, offset为4Byte）
	int dataSize = memTable->dataSize();		//数据区的大小
	return infoSize + indexSize + dataSize;
}

void KVStore::saveToDisk()
{
	std::string foldPath, SSTablePath;
	foldPath = generateLevel(0); //生成第0层（内部自带判断是否已经存在某一层，若存在则不生成）
	SSTablePath = getSSTablePath(0, levelFilesNum[0]);
	levelFilesNum[0]++;
	fstream out(SSTablePath.c_str(), ios::binary | ios::out);
	//写入时间戳
	out.write((char *)&maxTimeStamp, sizeof(maxTimeStamp));
	maxTimeStamp++;

	//写入键值对数量
	uint64_t size = memTable->size();
	out.write((char *)&size, sizeof(size));

	//写入键最小值和最大值
	uint64_t minKey = memTable->getMinKey();
	uint64_t maxKey = memTable->getMaxKey();
	out.write((char *)&minKey, sizeof(minKey));
	out.write((char *)&maxKey, sizeof(maxKey));

	//写入BloomFilter
	vector<bool> BF = memTable->genBFVector();
	for (size_t i = 0; i < BF.size(); i += 8)
	{
		char b = 0; //每次一个byte一个byte的写入文件
		for (int j = 0; j < 8; ++j)
		{
			if (BF[i + j])
			{
				b = b | (1 << (7 - j)); //设置byte的位
			}
		}
		out.write(&b, sizeof(b));
	}

	//写入索引区
	uint32_t offset = 32 + 10240 + memTable->size() * (8 + 4);
	Node *p = memTable->getLowestHead();
	p = p->right;
	while (p)
	{
		out.write((char *)&(p->key), sizeof(p->key));
		out.write((char *)&offset, sizeof(offset));
		offset += p->val.size() + 1;
		p = p->right;
	}

	//写入数据区
	p = memTable->getLowestHead();
	p = p->right;
	while (p)
	{
		out.write((char *)(p->val.c_str()), p->val.size() + 1);
		p = p->right;
	}

	//关闭文件
	out.close();

	//读取到index
	fstream *in = new fstream(SSTablePath.c_str(), ios::binary | ios::in);
	index->readFile(0, levelFilesNum[0] - 1, in);
	in->close();
	delete in;
	memTable->clear(); //写入完成后，应清空memTable
}

std::string KVStore::getLevelPath(int level)
{
	std::string path = root + "/level-" + to_string(level) + "/";
	return path;
}

std::string KVStore::generateLevel(int level)
{
	std::string path = getLevelPath(level);
	if (!utils::dirExists(path))
	{
		utils::mkdir(path.c_str());
		levelFilesNum.push_back(0); //新的一层内初始有0个文件
	}
	return path;
}

std::string KVStore::getSSTablePath(int level, int i)
{
	std::string path = getLevelPath(level);
	path += "/SSTable" + to_string(i) + ".sst";
	return path;
}

std::string KVStore::findInSSTable(uint64_t key)
{
	std::string result;
	char buf[200000] = {0};
	uint32_t offset = 0;
	SSTable *SSTp = index->search(key, offset);
	if (!SSTp)
		return "";
	fstream in(getSSTablePath(SSTp->getLevel(), SSTp->getId()), ios::binary | ios::in);
	in.seekg(offset, ios::beg);
	in.get(buf, 200000, '\0');
	in.close();

	result = buf;
	if (result == "~DELETED~")
		return "";
	return result;
}

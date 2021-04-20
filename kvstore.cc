#include "kvstore.h"
#include <string>

KVStore::KVStore(const std::string &dir) : KVStoreAPI(dir)
{
	memTable = new Skiplist();
}

KVStore::~KVStore()
{
}

/**
 * Insert/Update the key-value pair.
 * No return values for simplicity.
 */
void KVStore::put(uint64_t key, const std::string &s)
{
	memTable->insert(key, s);
}
/**
 * Returns the (string) value of the given key.
 * An empty string indicates not found.
 */
std::string KVStore::get(uint64_t key)
{
	std::string result;
	result = memTable->getValue(key);
	if (result == "~DELETED~")
	{
		return "";
	}
	return result;
}
/**
 * Delete the given key-value pair if it exists.
 * Returns false iff the key is not found.
 */
bool KVStore::del(uint64_t key)
{
	bool flag = false;
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
	return flag;
}

/**
 * This resets the kvstore. All key-value pairs should be removed,
 * including memtable and all sstables files.
 */
void KVStore::reset()
{
	delete memTable;
}

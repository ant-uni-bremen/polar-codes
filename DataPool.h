#ifndef MULTIPATHTREE_H
#define MULTIPATHTREE_H

#include <set>
#include <unordered_map>

struct Block
{
	unsigned useCount, size;
	float *data;
};

class DataPool
{
	std::unordered_multimap<unsigned, Block*> freeBlocks;

public:
	DataPool();
	~DataPool();

	Block* allocate(unsigned size);
	Block* lazyDuplicate(Block *block);
	Block* duplicate(Block* other);
	void release(Block* block);
};

#endif

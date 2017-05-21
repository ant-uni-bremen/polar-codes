#ifndef MULTIPATHTREE_H
#define MULTIPATHTREE_H

#include <vector>
#include <stack>

struct Block
{
	unsigned useCount, stage;
	float *data;
};

class DataPool
{
	std::vector<std::stack<Block*>> freeBlocks;

public:
	DataPool(int stages);
	~DataPool();

	Block* allocate(unsigned stage);
	Block* lazyDuplicate(Block *block);
	Block* duplicate(Block* other);
	void release(Block* block);
};

#endif

#include <cstring>
#include <iostream>

#include "DataPool.h"
#include "Parameters.h"

DataPool::DataPool()
{

}

DataPool::~DataPool()
{
	for(auto block : freeBlocks)
	{
		_mm_free(block.second->data);
	}
}

Block* DataPool::allocate(unsigned size)
{
	Block *block;
	auto it = freeBlocks.find(size);
	if(it == freeBlocks.end())
	{
		block = new Block();
		block->data = (float*)_mm_malloc(size<<2, sizeof(vec));
		block->useCount = 1;
		block->size = size;
	}
	else
	{
		block = it->second;
		block->useCount = 1;
		freeBlocks.erase(it);
	}
	return block;
}

Block* DataPool::lazyDuplicate(Block *block)
{
	block->useCount++;
	return block;
}

Block* DataPool::duplicate(Block* other)
{
	Block *block = allocate(other->size);
	memcpy(block->data, other->data, other->size<<2);
	other->useCount--;
	return block;
}

void DataPool::release(Block* block)
{
	block->useCount--;
	if(block->useCount == 0)
	{
		freeBlocks.insert(std::pair<unsigned, Block*>(block->size, block));
	}
}

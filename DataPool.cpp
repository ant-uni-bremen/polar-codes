#include <cstring>
#include <iostream>

#include "DataPool.h"
#include "Parameters.h"

DataPool::DataPool(int stages)
{
	freeBlocks.resize(stages);
}

DataPool::~DataPool()
{
	for(auto &stack : freeBlocks)
	{
		while(!stack.empty())
		{
			_mm_free(stack.top()->data);
			stack.pop();
		}
	}
}

Block* DataPool::allocate(unsigned stage)
{
	Block *block;
	if(freeBlocks[stage].empty())
	{
		block = new Block();
		block->data = (float*)_mm_malloc(1<<(stage+2), sizeof(vec));
		block->useCount = 1;
		block->stage = stage;
	}
	else
	{
		block = freeBlocks[stage].top();
		block->useCount = 1;
		freeBlocks[stage].pop();
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
	Block *block = allocate(other->stage);
	memcpy(block->data, other->data, 1<<(other->stage+2));
	other->useCount--;
	return block;
}

void DataPool::release(Block* block)
{
	if(--block->useCount == 0)
	{
		freeBlocks[block->stage].push(block);
	}
}

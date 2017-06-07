#include <cstring>
#include <iostream>

#include "DataPool.h"
#include "Parameters.h"

DataPool::DataPool(int stages)
{
	freeBlocks.resize(stages+1);
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
		block->data = (float*)_mm_malloc(4<<stage, sizeof(vec));
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
	memcpy(block->data, other->data, 4<<(other->stage));
	other->useCount--;
	return block;
}

void DataPool::prepareForWrite(Block*& block)
{
	if(block->useCount > 1)
	{
		block = duplicate(block);
	}
}

void DataPool::release(Block* block)
{
	if(--block->useCount == 0)
	{
		freeBlocks[block->stage].push(block);
	}
}

/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DATAPOOL_TXX
#define PC_DATAPOOL_TXX

#include <cstddef>
#include <cstring>
#include <map>
#include <stack>
#include <iostream>

#include <polarcode/avxconvenience.h>

namespace PolarCode {

template <typename mT>
struct Block {
	size_t useCount;
	size_t size;
	mT *data;
};

/*!
 * \brief A class that saves complexity in memory allocation, if blocks of few
 * distinct sizes are heavily re-used.
 */
template <typename T, size_t alignment>
class DataPool {
	std::map<size_t, std::stack<Block<T>*>> freeBlocks;

public:
	DataPool() {
	}

	~DataPool() {
		for(auto &stack : freeBlocks) {
			while(!stack.second.empty()) {
				_mm_free(stack.second.top()->data);
				delete stack.second.top();
				stack.second.pop();
			}
		}
	}

	/*!
	 * \brief Get a pointer to a data block of _size_ elements.
	 *
	 * This function either allocates a new block or recalls a previously freed
	 * block from the stack of available blocks.
	 *
	 * \param size Number of elements to allocate.
	 * \return Pointer to a new data block.
	 */
	Block<T>* allocate(size_t size) {
		Block<T> *block;

		size = std::max(alignment/sizeof(T), size);

/* This change improves performance only slightly, but looks nice:
 * Instead of searching the key, we simply try to access it and when failing,
 * create a new element.
 */
		try {
			(void)freeBlocks[size];
		} catch(...) {
			freeBlocks.insert({size, std::stack<Block<T>*>()});
		}

		if(freeBlocks[size].empty()) {
			block = new Block<T>();
			void* ptr = _mm_malloc(sizeof(T)*size, alignment);
			if(ptr == nullptr) {
				std::cerr << "Can't allocate aligned memory." << std::endl;
			}
			memset(ptr, 0, sizeof(T)*size);
			block->data = reinterpret_cast<T*>(ptr);
			block->useCount = 1;
			block->size = size;
		} else {
			block = freeBlocks[size].top();
			block->useCount = 1;
			freeBlocks[size].pop();
		}
		return block;
	}

	/*!
	 * \brief Create a virtual copy of the given block by increasing the reference counter.
	 * \param block The block (not) to copy.
	 * \return The same pointer to imitate a copy operation.
	 */
	Block<T>* lazyDuplicate(Block<T> *block) {
		block->useCount++;
		return block;
	}

	/*!
	 * \brief Create a physical copy of the given block.
	 * \param other The block to duplicate.
	 * \return Pointer to the newly created block that contains a copy of the data.
	 */
	Block<T>* duplicate(Block<T> *other) {
		Block<T> *block = allocate(other->size);
		memcpy(block->data, other->data, sizeof(T)*(other->size));
		other->useCount--;
		return block;
	}

	/*!
	 * \brief Create a physical copy, only if neccessary, in order to gain valid
	 *        write access.
	 *
	 * \param block The block that might be copied.
	 */
	void prepareForWrite(Block<T>*& block) {
		if(block->useCount > 1) {
			block = duplicate(block);
		}
	}

	/*!
	 * \brief Decrease the reference counter, eventually marking it as _unused_.
	 * \param block The block that is no longer in use by the caller.
	 */
	void release(Block<T>* &block) {
		if(block == nullptr) {
			return;
		}
		if(--block->useCount == 0) {
			freeBlocks[block->size].push(block);
		}
		block = nullptr;
	}
};

}// namespace PolarCode
#endif

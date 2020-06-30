/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef ALIGNEDNEW_H
#define ALIGNEDNEW_H

/* Source: https://stackoverflow.com/a/41418690 */

#include <cstdlib>
#include <new>

using size_t = ::std::size_t;

template <size_t ALIGNMENT>
struct alignas(ALIGNMENT) AlignedNew {
  static_assert(ALIGNMENT > 0, "ALIGNMENT must be positive");
  static_assert((ALIGNMENT & (ALIGNMENT - 1)) == 0,
	  "ALIGNMENT must be a power of 2");
  static_assert((ALIGNMENT % sizeof(void*)) == 0,
	  "ALIGNMENT must be a multiple of sizeof(void *)");
  static void* operator new(size_t count) { return Allocate(count); }
  static void* operator new[](size_t count) { return Allocate(count); }
  static void operator delete(void* ptr) { free(ptr); }
  static void operator delete[](void* ptr) { free(ptr); }

 private:
  static void* Allocate(size_t count) {
	void* result = nullptr;
	const auto alloc_failed = posix_memalign(&result, ALIGNMENT, count);
	if (alloc_failed)  throw ::std::bad_alloc();
	return result;
  }
};

#endif //ALIGNEDNEW_H

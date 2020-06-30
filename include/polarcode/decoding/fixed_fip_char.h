/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifndef PC_DEC_FIXED_CHAR_H
#define PC_DEC_FIXED_CHAR_H

#include <polarcode/encoding/encoder.h>
#include <polarcode/decoding/decoder.h>

namespace PolarCode {
namespace Decoding {


namespace FixedDecoding {

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

class FixedDecoder : public AlignedNew<32> {
public:
    FixedDecoder();
    virtual ~FixedDecoder();
    virtual void decode(void* LlrIn, void* BitsOut) = 0;

};

FixedDecoder *createFixedDecoder(unsigned int scheme);

}// namespace FixedDecoding

class FixedChar : public Decoder {
    FixedDecoding::FixedDecoder *mDecoder;
	Encoding::Encoder *mEncoder;

public:
    FixedChar(unsigned int scheme);
    ~FixedChar();

    bool decode();
};

}// namespace Decoding
}// namespace PolarCode

#endif

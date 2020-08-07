/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "errordetectiontest.h"

#include <cstring>

CPPUNIT_TEST_SUITE_REGISTRATION(ErrorDetectionTest);

void ErrorDetectionTest::setUp() {
	using namespace PolarCode::ErrorDetection;

	mDummy = new Dummy();
	mCrc8  = new CRC8();
	mCrc32 = new CRC32();

  const unsigned char e_key[] = { 0x2b,0x7e,0x15,0x16,
                                  0x28,0xae,0xd2,0xa6,
                                  0xab,0xf7,0x15,0x88,
                                  0x09,0xcf,0x4f,0x3c};
  std::vector<unsigned char> my_key(e_key, e_key + 16);
  mCmac  = new cmac(my_key);

	mDataLength = 32;
	mData = new char[mDataLength];
	mTestInput = new unsigned char[mDataLength];
	strcpy(reinterpret_cast<char*>(mData), "Hello, this is some test text..");
}

void ErrorDetectionTest::tearDown() {
	delete mDummy;
	delete mCrc8;
	delete mCrc32;
  delete mCmac;
	delete [] mData;
	delete [] mTestInput;
}

void ErrorDetectionTest::testDummy() {
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mDummy->generate(mTestInput, mDataLength);
	CPPUNIT_ASSERT_EQUAL(true, mDummy->check(mTestInput, mDataLength));

	mTestInput[0] ^= 0xFF;
	CPPUNIT_ASSERT_EQUAL(true, mDummy->check(mTestInput, mDataLength));
}

void ErrorDetectionTest::testCrc8() {
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mCrc8->generate(mTestInput, mDataLength);
	CPPUNIT_ASSERT_EQUAL(true, mCrc8->check(mTestInput, mDataLength));

	mTestInput[0] ^= 0xFF;
	CPPUNIT_ASSERT_EQUAL(false, mCrc8->check(mTestInput, mDataLength));
}

void ErrorDetectionTest::testCrc32() {
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mCrc32->generate(mTestInput, mDataLength);
	CPPUNIT_ASSERT_EQUAL(true, mCrc32->check(mTestInput, mDataLength));

	mTestInput[0] ^= 0xFF;
	CPPUNIT_ASSERT_EQUAL(false, mCrc32->check(mTestInput, mDataLength));
}

void ErrorDetectionTest::testCmac() {
  // K: 2b7e1516 28aed2a6 abf71588 09cf4f3c
//  const unsigned char e_key[] = { 0x2b,0x7e,0x15,0x16,
//                                  0x28,0xae,0xd2,0xa6,
//                                  0xab,0xf7,0x15,0x88,
//                                  0x09,0xcf,0x4f,0x3c};

  // M: 6bc1bee2 2e409f96 e93d7e11 7393172a Mlen: 128
  unsigned char e_message[] = { 0x6b,0xc1,0xbe,0xe2,
                                      0x2e,0x40,0x9f,0x96,
                                      0xe9,0x3d,0x7e,0x11,
                                      0x73,0x93,0x17,0x2a, 0x00 };

  // expected result T = 070a16b4 6b4d4144 f79bdd9d d04a287c
  const unsigned char e_cmac[] = {0x07, 0x0a, 0x16, 0xb4,
                                  0x6b, 0x4d, 0x41, 0x44,
                                  0xf7, 0x9b, 0xdd, 0x9d,
                                  0xd0, 0x4a, 0x28, 0x7c};
	strcpy(reinterpret_cast<char*>(mTestInput), mData);

	mCmac->generate(e_message, 17);
	CPPUNIT_ASSERT_EQUAL(true, e_message[16] == e_cmac[0]);
  for(int i = 0; i < 17; ++i) {
    printf("%.2X ", e_message[i]);
  }
  std::cout << std::endl;

  CPPUNIT_ASSERT_EQUAL(true, mCmac->check(e_message, 17));

//	mTestInput[0] ^= 0xFF;
//	CPPUNIT_ASSERT_EQUAL(false, mCrc32->check(mTestInput, mDataLength));
}

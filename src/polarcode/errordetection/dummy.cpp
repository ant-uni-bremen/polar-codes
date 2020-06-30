/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <polarcode/errordetection/dummy.h>

namespace PolarCode
{
	namespace ErrorDetection
	{

		Dummy globalDummyDetector;

		Dummy::Dummy()
		{
		}

		Dummy::~Dummy()
		{
		}

		unsigned Dummy::getCheckBitCount()
		{
			return 0;
		}

		std::string Dummy::getType()
		{
			return std::string("DUMMY");
		}

		//Null check believes the code was okay
		bool Dummy::check(void *dataPtr, int bytes)
		{
			return true;
		}

		void Dummy::generate(void *dataPtr, int bytes)
		{
			//Nothing to do
		}

		//Whatever we have to check, the first array is okay
		int Dummy::multiCheck(void **dataPtr, int nArrays, int nBytes)
		{
			return 0;
		}

	} //namespace ErrorDetection
} //namespace PolarCode

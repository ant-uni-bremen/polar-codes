#include <polarcode/construction/bhattacharrya.h>
#include "constructiontest.h"

CPPUNIT_TEST_SUITE_REGISTRATION(ConstructionTest);

void ConstructionTest::setUp() {

}

void ConstructionTest::tearDown() {

}

void ConstructionTest::testBhattacharrya() {
	std::set<unsigned> output;
	std::set<unsigned> expectedOutput({0,1,2,4});

	mConstructor = new PolarCode::Construction::Bhattacharrya(8, 4);
	output = mConstructor->construct();
	delete mConstructor;

	CPPUNIT_ASSERT(output == expectedOutput);
}

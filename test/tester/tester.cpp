/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <cppunit/CompilerOutputter.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <fmt/core.h>

int main(int argc, char* argv[])
{
    // Get registry
    CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();

    // Get the top level suite from the registry
    CppUnit::Test* suite = registry.makeTest();

    // std::cout << "Test cases: " << suite->countTestCases() << std::endl;
    // std::cout << "Child test count: " << suite->getChildTestCount() << std::endl;
    // std::cout << "Name: " << suite->getName() << std::endl;

    CppUnit::TextUi::TestRunner runner;
    // runner.addTest(suite);
    if (argc > 1) {
        std::string pattern(argv[1]);

        for (int iTestIndex = 0; iTestIndex < suite->getChildTestCount(); ++iTestIndex) {
            auto test = suite->getChildTestAt(iTestIndex);
            fmt::print("Test No. {:>2}:\t{}\n", iTestIndex, test->getName());
            if (test->getName().find(pattern) != std::string::npos) {
                fmt::print("Add test -> {}\n", test->getName());
                runner.addTest(test);
            }
        }
    } else {
        runner.addTest(suite);
    }

    // Adds the test to the list of test to run


    // Change the default outputter to a compiler error format outputter
    runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(), std::cerr));
    // Run the tests.
    bool wasSucessful = runner.run();

    // Return error code 1 if the one of test failed.
    return wasSucessful ? 0 : 1;
}

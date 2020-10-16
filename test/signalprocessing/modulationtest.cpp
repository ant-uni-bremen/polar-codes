/* -*- c++ -*- */
/*
 * Copyright 2018 Florian Lotze
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "modulationtest.h"

#include <signalprocessing/modulation/ask.h>
#include <signalprocessing/modulation/bpsk.h>

#include <iostream>

CPPUNIT_TEST_SUITE_REGISTRATION(ModulationTest);

void ModulationTest::setUp() {}

void ModulationTest::tearDown() {}

void ModulationTest::testBpsk()
{
    SignalProcessing::Modulation::Modem* Modem = new SignalProcessing::Modulation::Bpsk();

    std::vector<float> shortTestInput = { 0.0, -0.0, -0.0, 0.0 };
    std::vector<float> shortTestExpectedOutput = { 1.0, -1.0, -1.0, 1.0 };

    Modem->setInputSignal(&shortTestInput);
    Modem->modulate();
    CPPUNIT_ASSERT(shortTestExpectedOutput == *Modem->outputSignal());

    delete Modem;
}

void ModulationTest::testAsk()
{
    SignalProcessing::Modulation::Modem* Modem =
        new SignalProcessing::Modulation::Ask(2, false);

    std::vector<float> modulationInput = { 0.0, 0.0, 0.0, -0.0, -0.0, 0.0, -0.0, -0.0 };
    std::vector<float> demodulationInput;

    Modem->setInputSignal(&modulationInput);
    Modem->modulate();
    std::vector<float>* modulationOutput = Modem->outputSignal();

    // Test the Gray-mapping
    bool modulationSuccess =
        modulationOutput->at(0) == 3 && modulationOutput->at(1) == 1 &&
        modulationOutput->at(2) == -3 && modulationOutput->at(3) == -1;

    CPPUNIT_ASSERT(modulationSuccess);

    demodulationInput.resize(modulationOutput->size());
    std::copy(
        modulationOutput->begin(), modulationOutput->end(), demodulationInput.begin());

    Modem->setInputSignal(&demodulationInput);
    Modem->demodulate();

    // std::vector<float> *demodulationOutput = Modem->outputSignal();
    PolarCode::BitContainer* detectedData = new PolarCode::PackedContainer(8);
    Modem->getDataOutput(detectedData);

    char data, expectedData = 0b00011011;
    detectedData->getPackedBits(&data);

    CPPUNIT_ASSERT_EQUAL(expectedData, data);


    delete detectedData;
    delete Modem;
}

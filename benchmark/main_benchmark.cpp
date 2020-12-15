/* -*- c++ -*- */
/*
 * Copyright 2020 Johannes Demel
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include "test_functions.txx"
#include <benchmark/benchmark.h>
#include <iostream>
#include <memory>

#include <polarcode/construction/constructor.h>
#include <polarcode/decoding/decoder.h>
#include <polarcode/encoding/butterfly_fip_packed.h>
#include <polarcode/encoding/encoder.h>
#include <polarcode/errordetection/errordetector.h>


static void BM_calculate_parity_std(benchmark::State& state)
{
    constexpr unsigned size = (0x1 << 21);
    auto vec = initialize_random_vector(size);
    unsigned result = 16;
    for (auto _ : state) {
        result = calculate_bit_parity<size>(vec.data());
    }
    if (result != 0 && result != 1) {
        std::cout << "The function failed!\n";
    }
}

BENCHMARK(BM_calculate_parity_std);


static void BM_calculate_parity_simd(benchmark::State& state)
{
    constexpr unsigned size = (0x1 << 21);
    auto vec = initialize_random_vector(size);
    unsigned result = 16;
    for (auto _ : state) {
        result = (calculate_parity<size>(vec.data()) < 0) ? 0x1 : 0x0;
    }
    if (result != 0 && result != 1) {
        std::cout << "The function failed!\n";
    }
}

BENCHMARK(BM_calculate_parity_simd);


static void BM_argabsmin_std(benchmark::State& state)
{
    constexpr unsigned size = (0x1 << 21);
    auto vec = initialize_random_vector(size);
    unsigned result = size + 4;
    for (auto _ : state) {
        result = argabsmin<size>(vec.data());
    }
    if (result > size) {
        std::cout << "The function failed!\n";
    }
}

BENCHMARK(BM_argabsmin_std);


static void BM_argabsmin_simd(benchmark::State& state)
{
    constexpr unsigned size = (0x1 << 21);
    auto vec = initialize_random_vector(size);
    unsigned result = size + 4;
    for (auto _ : state) {
        result = argabsmin_avx2<size>(vec.data());
    }
    if (result > size) {
        std::cout << "The function failed!\n";
    }
}

BENCHMARK(BM_argabsmin_simd);

static void BM_calculate_spc_seq(benchmark::State& state)
{
    constexpr unsigned size = (0x1 << 21);
    auto vec = initialize_random_vector(size);
    std::vector<float> result(size);

    for (auto _ : state) {
        calculate_spc_seq<size>(result.data(), vec.data());
    }

    unsigned nflipped = 0;
    for (unsigned i = 0; i < vec.size(); ++i) {
        if (vec[i] != result[i]) {
            nflipped++;
        }
    }
    if (nflipped > 1) {
        std::cout << "The function failed!\n";
    }
}

BENCHMARK(BM_calculate_spc_seq);


static void BM_calculate_spc_opt(benchmark::State& state)
{
    constexpr unsigned size = (0x1 << 21);
    auto vec = initialize_random_vector(size);
    std::vector<float> result(size);

    for (auto _ : state) {
        calculate_spc<size>(result.data(), vec.data());
    }

    unsigned nflipped = 0;
    for (unsigned i = 0; i < vec.size(); ++i) {
        if (vec[i] != result[i]) {
            nflipped++;
        }
    }
    if (nflipped > 1) {
        std::cout << "The function failed!\n";
    }
}

BENCHMARK(BM_calculate_spc_opt);


std::unique_ptr<PolarCode::Encoding::ButterflyFipPacked>
create_polar_encoder(const size_t block_length,
                     const std::vector<unsigned>& frozen_bit_positions,
                     const unsigned detector_size,
                     const std::string detector_type = std::string("crc"),
                     const bool isSystematic = false)
{
    auto encoder = std::make_unique<PolarCode::Encoding::ButterflyFipPacked>(
        block_length, frozen_bit_positions);
    encoder->setSystematic(isSystematic);
    encoder->setErrorDetection(
        PolarCode::ErrorDetection::create(detector_size, detector_type));
    return encoder;
}


std::unique_ptr<PolarCode::Decoding::Decoder>
create_polar_decoder(const size_t block_length,
                     const size_t list_size,
                     const std::vector<unsigned>& frozen_bit_positions,
                     const std::string decoder_type,
                     const unsigned detector_size,
                     const std::string detector_type = std::string("crc"),
                     const bool isSystematic = false)
{
    auto decoder =
        std::unique_ptr<PolarCode::Decoding::Decoder>(PolarCode::Decoding::create(
            block_length, list_size, frozen_bit_positions, decoder_type));
    decoder->setSystematic(isSystematic);
    decoder->setErrorDetection(
        PolarCode::ErrorDetection::create(detector_size, detector_type));
    return decoder;
}


static void benchmark_polar_encoder(benchmark::State& state,
                                    const size_t block_length,
                                    const size_t info_length,
                                    const float dsnr,
                                    const unsigned parity_size,
                                    const std::string& detector_type,
                                    const bool is_systematic)
{
    const auto frozen_bit_positions =
        PolarCode::Construction::frozen_bits(block_length, info_length, dsnr);

    auto encoder = create_polar_encoder(
        block_length, frozen_bit_positions, parity_size, detector_type, is_systematic);

    auto vec = initialize_random_bit_vector(info_length / 8);
    std::vector<uint8_t> result(block_length / 8);

    for (auto _ : state) {
        benchmark::DoNotOptimize(vec);
        benchmark::DoNotOptimize(result);
        encoder->encode_vector((void*)vec.data(), (void*)result.data());
        benchmark::DoNotOptimize(vec);
        benchmark::DoNotOptimize(result);
    }

    // state.SetBytesProcessed(block_length * state.iterations());

    state.counters["CodeThr"] = benchmark::Counter(block_length * state.iterations(),
                                                   benchmark::Counter::kIsRate,
                                                   benchmark::Counter::OneK::kIs1024);
    state.counters["InfoThr"] = benchmark::Counter(info_length * state.iterations(),
                                                   benchmark::Counter::kIsRate,
                                                   benchmark::Counter::OneK::kIs1024);
}

static void benchmark_polar_decoder(benchmark::State& state,
                                    const size_t block_length,
                                    const size_t info_length,
                                    const size_t list_size,
                                    const float dsnr,
                                    const std::string& decoder_type,
                                    const unsigned parity_size,
                                    const std::string& detector_type,
                                    const bool is_systematic,
                                    const std::string& constructor_type)
{
    const auto frozen_bit_positions = PolarCode::Construction::frozen_bits(
        block_length, info_length, dsnr, constructor_type);
    auto decoder = create_polar_decoder(block_length,
                                        list_size,
                                        frozen_bit_positions,
                                        decoder_type,
                                        parity_size,
                                        detector_type,
                                        is_systematic);


    auto vec = initialize_random_vector(block_length, 0.5);
    std::vector<uint8_t> result(info_length / 8);

    for (auto _ : state) {
        benchmark::DoNotOptimize(vec);
        benchmark::DoNotOptimize(result);
        decoder->decode_vector(vec.data(), (void*)result.data());
        benchmark::DoNotOptimize(vec);
        benchmark::DoNotOptimize(result);
    }

    state.counters["CodeThr"] = benchmark::Counter(block_length * state.iterations(),
                                                   benchmark::Counter::kIsRate,
                                                   benchmark::Counter::OneK::kIs1024);
    state.counters["InfoThr"] = benchmark::Counter(info_length * state.iterations(),
                                                   benchmark::Counter::kIsRate,
                                                   benchmark::Counter::OneK::kIs1024);
}

static void BM_polar_encode(benchmark::State& state, const std::string& detector_type)
{
    const size_t block_length = static_cast<size_t>(state.range(0));
    const size_t info_length = static_cast<size_t>(state.range(1));
    const unsigned parity_size = static_cast<unsigned>(state.range(2));
    const bool is_systematic = static_cast<size_t>(state.range(3)) > 0;
    const float dsnr = static_cast<float>(state.range(4)) / 100.0f;
    // std::cout << block_length << "\t" << info_length << "\t" << detector_type
    //           << parity_size << "\t" << dsnr << "\t" << is_systematic << std::endl;
    if (not(block_length > info_length)) {
        std::string msg("Invalid code (" + std::to_string(block_length) + ", " +
                        std::to_string(info_length) + ")");
        state.SkipWithError(msg.c_str());
        return;
    }

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}


BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        64,
                    },
                    { 16, 24, 32, 40, 48, 56 },
                    {
                        0,
                        8,
                    },
                    {
                        0,
                        1,
                    },
                    { 100, 400 } });

BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        128,
                    },
                    { 16, 32, 48, 64, 80, 96, 112 },
                    {
                        0,
                        8,
                        16,
                    },
                    { 0, 1 },
                    { -100, 0, 100, 400 } });

BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        256,
                    },
                    { 32, 64, 96, 128, 160, 192, 224 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 400 } });

BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        512,
                    },
                    { 64, 128, 192, 256, 320, 384, 448 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 400 } });

BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        1024,
                    },
                    { 128, 256, 384, 512, 640, 768, 896 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });

BENCHMARK_CAPTURE(BM_polar_encode, CMAC, "cmac")
    ->ArgsProduct({ {
                        1024,
                    },
                    { 128, 256, 384, 512, 640, 768, 896 },
                    { 8, 16, 32, 64 },
                    { 0, 1 },
                    {
                        100,
                    } });


BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        2048,
                    },
                    { 256, 384, 512, 768, 1024, 1280, 1536, 1792 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });


BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        32768,
                    },
                    { 4096, 8192, 12288, 16384, 20480, 24576, 28672 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });


BENCHMARK_CAPTURE(BM_polar_encode, CRC, "crc")
    ->ArgsProduct({ {
                        131072,
                    },
                    { 16384, 32768, 49152, 65536, 81920, 98304, 114688 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });


static void BM_polar_decode(benchmark::State& state,
                            const std::string& detector_type,
                            const std::string& decoder_type,
                            const std::string& constructor_type = std::string("BB"))
{
    const size_t block_length = static_cast<size_t>(state.range(0));
    const size_t info_length = static_cast<size_t>(state.range(1));
    const size_t list_size = static_cast<size_t>(state.range(2));
    const unsigned parity_size = static_cast<unsigned>(state.range(3));
    const bool is_systematic = static_cast<size_t>(state.range(4)) > 0;
    const float dsnr = static_cast<float>(state.range(5)) / 100.0f;
    // std::cout << block_length << "\t" << info_length << "\t" << list_size << "\t"
    //           << detector_type << parity_size << "\t" << dsnr << "\t" << is_systematic
    //           << std::endl;
    if (not(block_length > info_length)) {
        std::string msg("Invalid code (" + std::to_string(block_length) + ", " +
                        std::to_string(info_length) + ")");
        state.SkipWithError(msg.c_str());
        return;
    }

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic,
                            constructor_type);
}


BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        64,
                    },
                    { 16, 24, 32, 40, 48, 56 },
                    { 1, 2, 4, 8, 16, 32 },
                    {
                        0,
                        8,
                    },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });

BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        128,
                    },
                    { 16, 32, 48, 64, 80, 96, 112 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });


BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        256,
                    },
                    { 32, 64, 96, 128, 160, 192, 224 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });

BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        512,
                    },
                    { 64, 128, 192, 256, 320, 384, 448 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });


BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        1024,
                    },
                    { 128, 256, 384, 512, 640, 768, 896 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });

BENCHMARK_CAPTURE(BM_polar_decode, CRC_BE_float, "crc", "float", "BE")
    ->ArgsProduct({ {
                        1024,
                    },
                    { 128, 256, 384, 512, 640, 768, 896 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    {
                        100,
                    } });

BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_char, "crc", "char")
    ->ArgsProduct({ {
                        1024,
                    },
                    { 128, 256, 384, 512, 640, 768, 896 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });

BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        2048,
                    },
                    { 256, 512, 768, 1024, 1280, 1536, 1792 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });

BENCHMARK_CAPTURE(BM_polar_decode, CRC_BB_float, "crc", "float")
    ->ArgsProduct({ {
                        8192,
                    },
                    { 1024, 2048, 3072, 4096, 5120, 6144, 7168 },
                    { 1, 2, 4, 8, 16, 32 },
                    { 0, 8, 16, 32 },
                    { 0, 1 },
                    { -100, 0, 100, 200, 300, 400 } });


BENCHMARK_MAIN();

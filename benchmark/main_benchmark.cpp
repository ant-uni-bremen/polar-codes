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

static void BM_polar_encode(benchmark::State& state,
                            const float dsnr,
                            const unsigned parity_size,
                            const std::string& detector_type,
                            const bool is_systematic)
{
    const size_t block_length = static_cast<size_t>(state.range(0));
    const size_t info_length = static_cast<size_t>(state.range(1));
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

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC8_nonsystematic, 1.0, 8, "crc", false)
    ->ArgsProduct({ {
                        64,
                    },
                    { 16, 24, 32, 40, 48 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC8_nonsystematic, 1.0, 8, "crc", false)
    ->ArgsProduct({ {
                        128,
                    },
                    { 16, 24, 32, 40, 48, 64, 92 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC8_nonsystematic, 1.0, 8, "crc", false)
    ->ArgsProduct({ {
                        256,
                    },
                    { 16, 24, 32, 40, 48, 64, 92, 128, 192 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC8_nonsystematic, 1.0, 8, "crc", false)
    ->ArgsProduct({ {
                        512,
                    },
                    { 32, 64, 128, 256, 384 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dm1d0_CRC8_nonsystematic, -1.0, 8, "crc", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp0d0_CRC8_nonsystematic, 0.0, 8, "crc", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC8_nonsystematic, 1.0, 8, "crc", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC16_nonsystematic, 1.0, 16, "crc", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC32_nonsystematic, 1.0, 32, "crc", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CMAC8_nonsystematic, 1.0, 8, "cmac", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CMAC16_nonsystematic, 1.0, 16, "cmac", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CMAC32_nonsystematic, 1.0, 32, "cmac", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 64, 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CMAC64_nonsystematic, 1.0, 64, "cmac", false)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 128, 256, 512, 786 } });

BENCHMARK_CAPTURE(BM_polar_encode, Dp1d0_CRC8_systematic, 1.0, 8, "crc", true)
    ->ArgsProduct({ {
                        1024,
                    },
                    { 32, 64, 128, 256, 512, 786 } });

static void
BM_polar_encoder_fip_packed_1024_512_nonsystematic_crc8(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const float dsnr = 1.0;
    const unsigned parity_size = 8;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_1024_512_nonsystematic_crc8);


static void
BM_polar_encoder_fip_packed_1024_512_nonsystematic_crc16(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const float dsnr = 1.0;
    const unsigned parity_size = 16;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_1024_512_nonsystematic_crc16);


static void
BM_polar_encoder_fip_packed_1024_512_nonsystematic_crc32(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const float dsnr = 1.0;
    const unsigned parity_size = 32;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_1024_512_nonsystematic_crc32);


static void
BM_polar_encoder_fip_packed_1024_512_nonsystematic_cmac32(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const float dsnr = 1.0;
    const unsigned parity_size = 32;
    const auto detector_type = std::string("cmac");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_1024_512_nonsystematic_cmac32);


static void
BM_polar_encoder_fip_packed_1024_512_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const float dsnr = 1.0;
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_1024_512_nonsystematic_dummy0);


static void
BM_polar_encoder_fip_packed_1024_512_systematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const float dsnr = 1.0;
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = true;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_1024_512_systematic_dummy0);


static void
BM_polar_encoder_fip_packed_512_256_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 512;
    const size_t info_length = 256;
    const float dsnr = 1.0;
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_512_256_nonsystematic_dummy0);


static void
BM_polar_encoder_fip_packed_2048_1024_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 2048;
    const size_t info_length = 1024;
    const float dsnr = 1.0;
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_2048_1024_nonsystematic_dummy0);


static void
BM_polar_encoder_fip_packed_32768_16384_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 32768;
    const size_t info_length = 16384;
    const float dsnr = 1.0;
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_32768_16384_nonsystematic_dummy0);


static void
BM_polar_encoder_fip_packed_131072_65536_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 131072;
    const size_t info_length = 65536;
    const float dsnr = 1.0;
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_encoder(state,
                            block_length,
                            info_length,
                            dsnr,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_encoder_fip_packed_131072_65536_nonsystematic_dummy0);


static void benchmark_polar_decoder(benchmark::State& state,
                                    const size_t block_length,
                                    const size_t info_length,
                                    const size_t list_size,
                                    const float dsnr,
                                    const std::string& decoder_type,
                                    const unsigned parity_size,
                                    const std::string& detector_type,
                                    const bool is_systematic)
{
    const auto frozen_bit_positions =
        PolarCode::Construction::frozen_bits(block_length, info_length, dsnr);
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
        decoder->decode_vector(vec.data(), (void*)result.data());
    }
}


static void
BM_polar_decoder_float_512_256_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 512;
    const size_t info_length = 256;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_512_256_l1_nonsystematic_dummy0);


static void
BM_polar_decoder_float_1024_256_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 256;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_1024_256_l1_nonsystematic_dummy0);


static void
BM_polar_decoder_float_1024_512_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_1024_512_l1_nonsystematic_dummy0);


static void
BM_polar_decoder_float_2048_1024_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 2048;
    const size_t info_length = 1024;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_2048_1024_l1_nonsystematic_dummy0);


static void
BM_polar_decoder_float_8192_4096_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 8192;
    const size_t info_length = 4096;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_8192_4096_l1_nonsystematic_dummy0);


static void
BM_polar_decoder_float_8192_1024_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 8192;
    const size_t info_length = 1024;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_8192_1024_l1_nonsystematic_dummy0);


static void
BM_polar_decoder_char_1024_512_l1_nonsystematic_dummy0(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 1;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("char");
    const unsigned parity_size = 0;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_char_1024_512_l1_nonsystematic_dummy0);


static void BM_polar_decoder_float_1024_512_l2_nonsystematic_crc8(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 2;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 8;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_1024_512_l2_nonsystematic_crc8);


static void
BM_polar_decoder_float_1024_512_l32_nonsystematic_crc8(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 32;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 8;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_1024_512_l32_nonsystematic_crc8);

static void BM_polar_decoder_char_1024_512_l32_nonsystematic_crc8(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 32;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("char");
    const unsigned parity_size = 8;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_char_1024_512_l32_nonsystematic_crc8);


static void BM_polar_decoder_float_1024_512_l8_nonsystematic_crc8(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 8;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("float");
    const unsigned parity_size = 8;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_float_1024_512_l8_nonsystematic_crc8);

static void BM_polar_decoder_char_1024_512_l8_nonsystematic_crc8(benchmark::State& state)
{
    const size_t block_length = 1024;
    const size_t info_length = 512;
    const size_t list_size = 8;
    const float dsnr = 1.0;
    const auto decoder_type = std::string("char");
    const unsigned parity_size = 8;
    const auto detector_type = std::string("crc");
    const bool is_systematic = false;

    benchmark_polar_decoder(state,
                            block_length,
                            info_length,
                            list_size,
                            dsnr,
                            decoder_type,
                            parity_size,
                            detector_type,
                            is_systematic);
}

BENCHMARK(BM_polar_decoder_char_1024_512_l8_nonsystematic_crc8);


BENCHMARK_MAIN();

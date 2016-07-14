// Copyright (c) 2015-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <iostream>
#include <vector>
#include <complex>

POTHOS_TEST_BLOCK("/comms/tests", test_fft_float)
{
    /*
    numpy.fft.fft([(0.4+0.6j), (-0.7+0.6j), (-0.2+0.8j), (0.9+0.2j)])
    array([  4.00000000e-01+2.2j,   1.00000000e+00+1.4j,  -5.55111512e-17+0.6j,
             2.00000000e-01-1.8j])
    */

    std::vector<std::complex<float>> input;
    input.emplace_back(0.4, 0.6);
    input.emplace_back(-0.7, 0.6);
    input.emplace_back(-0.2, 0.8);
    input.emplace_back(0.9, 0.2);

    std::vector<std::complex<float>> result;
    result.emplace_back(0.4, 2.2);
    result.emplace_back(1.0, 1.4);
    result.emplace_back(0.0, 0.6);
    result.emplace_back(0.2, -1.8);

    //create blocks
    const auto dtype = Pothos::DType(typeid(std::complex<float>));
    auto source = Pothos::BlockRegistry::make("/blocks/vector_source", dtype);
    source.callVoid("setElements", input);
    source.callVoid("setMode", "ONCE");
    auto collector = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto fft = Pothos::BlockRegistry::make("/comms/fft", dtype, input.size(), false);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(source, 0, fft, 0);
        topology.connect(fft, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), result.size());
    auto pb = buff.as<const std::complex<float> *>();
    for (size_t i = 0; i < buff.elements(); i++)
    {
        std::cout << i << " FFT expected " << result[i] << " actual " << pb[i] << std::endl;
        POTHOS_TEST_TRUE(std::abs(pb[i].real()-result[i].real()) < 0.01);
        POTHOS_TEST_TRUE(std::abs(pb[i].imag()-result[i].imag()) < 0.01);
    }

    //perform the ifft and check the result
    source.callVoid("setElements", result);
    source.callVoid("setMode", "ONCE");
    collector.callVoid("clear");
    auto ifft = Pothos::BlockRegistry::make("/comms/fft", dtype, result.size(), true);
    {
        Pothos::Topology topology;
        topology.connect(source, 0, ifft, 0);
        topology.connect(ifft, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the buffer
    buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), input.size());
    pb = buff.as<const std::complex<float> *>();
    for (size_t i = 0; i < buff.elements(); i++)
    {
        std::cout << i << " IFFT expected " << input[i] << " actual " << pb[i] << std::endl;
        POTHOS_TEST_TRUE(std::abs(pb[i].real()-input[i].real()*input.size()) < 0.01);
        POTHOS_TEST_TRUE(std::abs(pb[i].imag()-input[i].imag()*input.size()) < 0.01);
    }
}

POTHOS_TEST_BLOCK("/comms/tests", test_fft_short)
{
    auto env = Pothos::ProxyEnvironment::make("managed");
    auto registry = env->findProxy("Pothos/BlockRegistry");

    /*
    numpy.fft.fft([(0.4+0.6j), (-0.7+0.6j), (-0.2+0.8j), (0.9+0.2j)])
    array([  4.00000000e-01+2.2j,   1.00000000e+00+1.4j,  -5.55111512e-17+0.6j,
             2.00000000e-01-1.8j])
    */

    std::vector<std::complex<float>> input;
    input.emplace_back(0.4*1000, 0.6*1000);
    input.emplace_back(-0.7*1000, 0.6*1000);
    input.emplace_back(-0.2*1000, 0.8*1000);
    input.emplace_back(0.9*1000, 0.2*1000);

    std::vector<std::complex<float>> result;
    result.emplace_back(0.4*1000, 2.2*1000);
    result.emplace_back(1.0*1000, 1.4*1000);
    result.emplace_back(0.0*1000, 0.6*1000);
    result.emplace_back(0.2*1000, -1.8*1000);

    //create blocks
    const auto dtype = Pothos::DType(typeid(std::complex<short>));
    auto source = Pothos::BlockRegistry::make("/blocks/vector_source", dtype);
    source.callVoid("setElements", input);
    source.callVoid("setMode", "ONCE");
    auto collector = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);
    auto fft = Pothos::BlockRegistry::make("/comms/fft", dtype, input.size(), false);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(source, 0, fft, 0);
        topology.connect(fft, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the buffer
    auto buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), result.size());
    auto pb = buff.as<const std::complex<short> *>();
    for (size_t i = 0; i < buff.elements(); i++)
    {
        std::cout << i << " FFT expected " << result[i] << " actual " << pb[i] << std::endl;
        POTHOS_TEST_TRUE(std::abs(pb[i].real()-result[i].real()/result.size()) < 0.01);
        POTHOS_TEST_TRUE(std::abs(pb[i].imag()-result[i].imag()/result.size()) < 0.01);
    }

    //perform the ifft and check the result
    source.callVoid("setElements", result);
    source.callVoid("setMode", "ONCE");
    collector.callVoid("clear");
    auto ifft = Pothos::BlockRegistry::make("/comms/fft", dtype, result.size(), true);
    {
        Pothos::Topology topology;
        topology.connect(source, 0, ifft, 0);
        topology.connect(ifft, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive());
    }

    //check the buffer
    buff = collector.call<Pothos::BufferChunk>("getBuffer");
    POTHOS_TEST_EQUAL(buff.elements(), input.size());
    pb = buff.as<const std::complex<short> *>();
    for (size_t i = 0; i < buff.elements(); i++)
    {
        std::cout << i << " IFFT expected " << input[i] << " actual " << pb[i] << std::endl;
        POTHOS_TEST_TRUE(std::abs(pb[i].real()-input[i].real()) < 0.01);
        POTHOS_TEST_TRUE(std::abs(pb[i].imag()-input[i].imag()) < 0.01);
    }
}

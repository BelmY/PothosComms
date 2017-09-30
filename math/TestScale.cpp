// Copyright (c) 2015-2016 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#include <Pothos/Testing.hpp>
#include <Pothos/Framework.hpp>
#include <Pothos/Proxy.hpp>
#include <cstdint>
#include <complex>
#include <cmath>
#include <iostream>

static const size_t NUM_POINTS = 13;

template <typename Type>
void testScaleTmpl(const double factor)
{
    auto dtype = Pothos::DType(typeid(Type));
    std::cout << "Testing scale with type " << dtype.toString() << ", factor " << factor << std::endl;

    auto feeder = Pothos::BlockRegistry::make("/blocks/feeder_source", dtype);
    auto scale = Pothos::BlockRegistry::make("/comms/scale", dtype);
    scale.call("setFactor", factor);
    auto collector = Pothos::BlockRegistry::make("/blocks/collector_sink", dtype);

    //load the feeder
    auto buffIn = Pothos::BufferChunk(typeid(Type), NUM_POINTS);
    auto pIn = buffIn.as<Type *>();
    for (size_t i = 0; i < buffIn.elements(); i++)
    {
        pIn[i] = Type(10*i);
    }
    feeder.call("feedBuffer", buffIn);

    //run the topology
    {
        Pothos::Topology topology;
        topology.connect(feeder, 0, scale, 0);
        topology.connect(scale, 0, collector, 0);
        topology.commit();
        POTHOS_TEST_TRUE(topology.waitInactive(0.01));
    }

    //check the collector
    Pothos::BufferChunk buffOut = collector.call("getBuffer");
    POTHOS_TEST_EQUAL(buffOut.elements(), buffIn.elements());
    auto pOut = buffOut.as<const Type *>();
    for (size_t i = 0; i < buffOut.elements(); i++)
    {
        const auto input = double(pIn[i]);
        const auto expected = Type(input * factor);
        //allow up to an error of 1 because of fixed point truncation rounding
        POTHOS_TEST_CLOSE(pOut[i], expected, 1);
    }
}

POTHOS_TEST_BLOCK("/comms/tests", test_scale)
{
    for (size_t i = 0; i <= 4; i++)
    {
        const double factor = i/2.0-1.0;
        testScaleTmpl<double>(factor);
        testScaleTmpl<float>(factor);
        testScaleTmpl<int64_t>(factor);
        testScaleTmpl<int32_t>(factor);
        testScaleTmpl<int16_t>(factor);
        testScaleTmpl<int8_t>(factor);
    }
}

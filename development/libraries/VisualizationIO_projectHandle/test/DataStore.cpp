//
// Copyright (C) 2012-2016 Shell International Exploration & Production.
// All rights reserved.
//
// Developed under license for Shell by Shell India Markets Pvt. Ltd.
//
// Confidential and proprietary source code of Shell.
// Do not distribute without written permission from Shell.
//

#include <gtest/gtest.h>
#include "../src/DataStore.h"

using namespace CauldronIO;

TEST( DataStore, Compression )
{
    const char hello[5] = { 'h', 'e', 'l', 'l', 'o' };
    size_t size = 5;

    char* compressed = DataStoreSave::compress(hello, size);
    char* decompressed = DataStoreLoad::decompress(compressed, size);

    EXPECT_EQ(size, sizeof(hello));
    for (size_t i = 0; i < size; ++i)
        EXPECT_EQ(decompressed[i], hello[i]);

    delete[] compressed;
    delete[] decompressed;

    float data[6] = { 1e-5, 0, 1e3, 3.14521, 1e12, 99999 };
    size = 6 * sizeof(float);
    
    compressed = DataStoreSave::compress((char*)data, size);
    decompressed = DataStoreLoad::decompress(compressed, size);
    float* result = (float*)decompressed;

    EXPECT_EQ(size, 6 * sizeof(float));
    for (size_t i = 0; i < 6; ++i)
        EXPECT_FLOAT_EQ(result[i], data[i]);

    delete[] compressed;
    delete[] decompressed;
}
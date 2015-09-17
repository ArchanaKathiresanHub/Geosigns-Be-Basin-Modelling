#include <gtest/gtest.h>
#include "../src/DataStore.h"

using namespace CauldronIO;

TEST( DataStore, Compression )
{
    const char hello[5] = { 'h', 'e', 'l', 'l', 'o' };
    const char hello_gz[25] = { 
        '\x1f', '\x8b', '\x08', '\x00', '\x00', '\x00', '\x00', '\x00',
        '\x00', '\xff', '\xcb', '\x48', '\xcd', '\xc9', '\xc9', '\x07',
        '\x00', '\x86', '\xa6', '\x10', '\x36', '\x05', '\x00', '\x00',
        '\x00', };
#ifdef _WIN32
    size_t size = 5;
    char* compressed = DataStore::compress(hello, size);

    EXPECT_EQ(size, sizeof(hello_gz));
    for (size_t i = 0; i < size; ++i)
        EXPECT_EQ(compressed[i], hello_gz[i]);

    char* decompressed = DataStore::decompress(compressed, size);
    EXPECT_EQ(size, sizeof(hello));
    for (size_t i = 0; i < size; ++i)
        EXPECT_EQ(decompressed[i], hello[i]);

    delete[] compressed;
    delete[] decompressed;

    float data[6] = { 1e-5, 0, 1e3, 3.14521, 1e12, 99999 };
    size = 6 * sizeof(float);
    
    compressed = DataStore::compress((char*)data, size);
    decompressed = DataStore::decompress(compressed, size);
    float* result = (float*)decompressed;

    EXPECT_EQ(size, 6 * sizeof(float));
    for (size_t i = 0; i < 6; ++i)
        EXPECT_FLOAT_EQ(result[i], data[i]);

    delete[] compressed;
    delete[] decompressed;
#endif
}
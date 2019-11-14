#include <gtest/gtest.h>
#include <bulk_impl.hpp>
#include <memory>

// ------------------------------------------------------------------
TEST(bulk_test, bulk_base_functionality_test) {
    ASSERT_EQ(1, 1);
}

// ------------------------------------------------------------------
int main( int argc, char* argv[] ) {
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS();
}
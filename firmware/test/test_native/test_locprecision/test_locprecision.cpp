#include <unity.h>
#include "util/locprecision.h"
#include <math.h>

using namespace mclite;

void test_exact_passthrough() {
    double c = 52.123456;
    TEST_ASSERT_TRUE(obfuscateCoord(c, 32) == c);   // precision 32 = no change
}

void test_within_half_cell() {
    double c = 52.123456;
    double step = 65536.0 / 1e7;                     // precision 16 grid step (deg)
    double o = obfuscateCoord(c, 16);
    TEST_ASSERT_TRUE(fabs(o - c) <= step / 2 + 1e-7);
}

void test_centered_in_cell() {
    // The result must sit at the cell centre: scaled value mod 65536 == 32768.
    double o = obfuscateCoord(52.123456, 16);
    long scaled = lround(o * 1e7);
    int rem = (int)(((scaled % 65536) + 65536) % 65536);
    TEST_ASSERT_EQUAL_INT(32768, rem);
}

void test_same_cell_same_output() {
    double a = obfuscateCoord(52.123456, 16);
    double b = obfuscateCoord(52.123457, 16);        // a hair away → same cell
    TEST_ASSERT_TRUE(fabs(a - b) < 1e-9);
}

void test_negative_coord() {
    double c = -33.868800;                            // Sydney latitude
    double step = 262144.0 / 1e7;                     // precision 14
    double o = obfuscateCoord(c, 14);
    TEST_ASSERT_TRUE(fabs(o - c) <= step / 2 + 1e-6);
    TEST_ASSERT_TRUE(o < 0.0 && o > -90.0);
}

void test_precision_meters() {
    TEST_ASSERT_EQUAL_UINT32(0, locPrecisionMeters(32));       // exact
    uint32_t m16 = locPrecisionMeters(16);
    TEST_ASSERT_TRUE(m16 > 700 && m16 < 760);                 // ~730 m
    uint32_t m10 = locPrecisionMeters(10);
    TEST_ASSERT_TRUE(m10 > 45000 && m10 < 48000);            // ~46.7 km
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_exact_passthrough);
    RUN_TEST(test_within_half_cell);
    RUN_TEST(test_centered_in_cell);
    RUN_TEST(test_same_cell_same_output);
    RUN_TEST(test_negative_coord);
    RUN_TEST(test_precision_meters);
    return UNITY_END();
}

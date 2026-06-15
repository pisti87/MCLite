#include <unity.h>
#include "util/coordparse.h"

using namespace mclite;

// ---- Decimal lat/lon ----

void test_decimal_basic() {
    GeoCoord gc = parseFirstGeoCoord("@ 52.123456, 13.654321");
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(fabs(gc.lat - 52.123456) < 1e-5);
    TEST_ASSERT_TRUE(fabs(gc.lon - 13.654321) < 1e-5);
}

void test_decimal_bare() {
    GeoCoord gc = parseFirstGeoCoord("48.8566, 2.3522");
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(fabs(gc.lat - 48.8566) < 1e-4);
    TEST_ASSERT_TRUE(fabs(gc.lon - 2.3522) < 1e-4);
}

void test_decimal_negative_sos() {
    GeoCoord gc = parseFirstGeoCoord("SOS @ -33.868800, 151.209500");
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(fabs(gc.lat - (-33.8688)) < 1e-4);
    TEST_ASSERT_TRUE(fabs(gc.lon - 151.2095) < 1e-4);
}

void test_decimal_in_sentence() {
    GeoCoord gc = parseFirstGeoCoord("meet me at 52.5200, 13.4050 ok?");
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(fabs(gc.lat - 52.52) < 1e-3);
}

void test_reject_no_decimal_point() {
    // "5, 6" must NOT be treated as a coordinate (no decimal point in lat)
    GeoCoord gc = parseFirstGeoCoord("see you at 5, 6");
    TEST_ASSERT_FALSE(gc.valid);
}

void test_reject_out_of_range() {
    GeoCoord gc = parseFirstGeoCoord("200.0, 500.0");
    TEST_ASSERT_FALSE(gc.valid);
}

void test_reject_plain_text() {
    GeoCoord gc = parseFirstGeoCoord("hello world, how are you");
    TEST_ASSERT_FALSE(gc.valid);
}

// ---- "both" format → exactly one coordinate (the decimal) ----

void test_both_format_returns_decimal() {
    GeoCoord gc = parseFirstGeoCoord("52.123456, 13.654321 (18T WL 8084 0656)");
    TEST_ASSERT_TRUE(gc.valid);
    // Decimal is found first → its value, not the (truncated) MGRS.
    TEST_ASSERT_TRUE(fabs(gc.lat - 52.123456) < 1e-5);
    TEST_ASSERT_TRUE(fabs(gc.lon - 13.654321) < 1e-5);
}

// ---- MGRS / UTMREF ----

void test_mgrs_spaced() {
    // "@ " + real MGRS for the Statue of Liberty; detect + reverse near truth.
    String m = "@ " + latLonToMGRS(40.6892, -74.0445, 4);
    GeoCoord gc = parseFirstGeoCoord(m);
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(fabs(gc.lat - 40.6892) < 2e-3);
    TEST_ASSERT_TRUE(fabs(gc.lon - (-74.0445)) < 2e-3);
}

void test_mgrs_compact() {
    // Compact (no spaces) MGRS must parse and land in the right region (NYC).
    GeoCoord gc = parseFirstGeoCoord("18TWL80840656");
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(gc.lat > 40.0 && gc.lat < 41.0);
    TEST_ASSERT_TRUE(gc.lon > -75.0 && gc.lon < -73.0);
}

void test_mgrs_in_sentence() {
    String m = "im at " + latLonToMGRS(48.8566, 2.3522, 4) + " right now";
    GeoCoord gc = parseFirstGeoCoord(m);
    TEST_ASSERT_TRUE(gc.valid);
    TEST_ASSERT_TRUE(fabs(gc.lat - 48.8566) < 2e-3);
    TEST_ASSERT_TRUE(fabs(gc.lon - 2.3522) < 2e-3);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_decimal_basic);
    RUN_TEST(test_decimal_bare);
    RUN_TEST(test_decimal_negative_sos);
    RUN_TEST(test_decimal_in_sentence);
    RUN_TEST(test_reject_no_decimal_point);
    RUN_TEST(test_reject_out_of_range);
    RUN_TEST(test_reject_plain_text);
    RUN_TEST(test_both_format_returns_decimal);
    RUN_TEST(test_mgrs_spaced);
    RUN_TEST(test_mgrs_compact);
    RUN_TEST(test_mgrs_in_sentence);
    return UNITY_END();
}

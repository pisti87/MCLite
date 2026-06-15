#include <unity.h>
#include "util/mgrs.h"

using namespace mclite;

// Reference points verified against external MGRS converters

void test_mgrs_statue_of_liberty() {
    // 40.6892 N, -74.0445 W -> 18T WL 8084 0656 (precision 4)
    String mgrs = latLonToMGRS(40.6892, -74.0445, 4);
    TEST_ASSERT_TRUE(mgrs.startsWith("18T WL"));
}

void test_mgrs_london_eye() {
    // 51.5033 N, -0.1195 W -> 30U XC
    String mgrs = latLonToMGRS(51.5033, -0.1195, 4);
    TEST_ASSERT_TRUE(mgrs.startsWith("30U"));
}

void test_mgrs_equator_prime_meridian() {
    // 0, 0 -> 31N AA
    String mgrs = latLonToMGRS(0.0, 0.0, 4);
    TEST_ASSERT_TRUE(mgrs.startsWith("31N"));
}

void test_mgrs_southern_hemisphere() {
    // Sydney: -33.8688, 151.2093 -> 56H
    String mgrs = latLonToMGRS(-33.8688, 151.2093, 4);
    TEST_ASSERT_TRUE(mgrs.startsWith("56H"));
}

void test_mgrs_norway_exception() {
    // Bergen: 60.39, 5.32 -> should be zone 32 (Norway exception)
    int zone = utmZoneNumber(60.39, 5.32);
    TEST_ASSERT_EQUAL_INT(32, zone);
}

void test_mgrs_svalbard_exception() {
    // Svalbard: 78.0, 16.0 -> should be zone 33
    int zone = utmZoneNumber(78.0, 16.0);
    TEST_ASSERT_EQUAL_INT(33, zone);
}

void test_mgrs_outside_utm() {
    String mgrs = latLonToMGRS(-85.0, 0.0, 4);
    TEST_ASSERT_EQUAL_STRING("Outside UTM", mgrs.c_str());
}

void test_mgrs_band_letter_equator() {
    TEST_ASSERT_EQUAL_CHAR('N', utmBandLetter(0.0));
}

void test_mgrs_band_letter_south() {
    TEST_ASSERT_EQUAL_CHAR('C', utmBandLetter(-79.0));
}

void test_mgrs_precision_1() {
    // Low precision — just check it doesn't crash and format looks right
    String mgrs = latLonToMGRS(48.8566, 2.3522, 1);
    TEST_ASSERT_TRUE(mgrs.length() > 0);
    TEST_ASSERT_TRUE(mgrs.indexOf(" ") > 0);
}

// ---- Reverse parser (mgrsToLatLon) ----

// Round-trip a lat/lon through forward + reverse; must come back within ~tol deg.
// precision-4 truncates to 10m, so the reverse lands on the cell's SW corner —
// allow a comfortable margin (~55m).
static void roundTrip(double lat, double lon) {
    String s = latLonToMGRS(lat, lon, 4);
    double rlat = 0, rlon = 0;
    TEST_ASSERT_TRUE(mgrsToLatLon(s.c_str(), rlat, rlon));
    TEST_ASSERT_TRUE(fabs(rlat - lat) < 0.0005);
    TEST_ASSERT_TRUE(fabs(rlon - lon) < 0.0005);
}

void test_mgrs_reverse_roundtrip_liberty() { roundTrip(40.6892, -74.0445); }
void test_mgrs_reverse_roundtrip_london()  { roundTrip(51.5033, -0.1195); }
void test_mgrs_reverse_roundtrip_sydney()  { roundTrip(-33.8688, 151.2093); }
void test_mgrs_reverse_roundtrip_paris()   { roundTrip(48.8566, 2.3522); }
void test_mgrs_reverse_roundtrip_equator() { roundTrip(0.0, 0.0); }

void test_mgrs_reverse_literal() {
    // Forward-encode a known point (Statue of Liberty) then decode the spaced
    // MGRS string back — truth anchor is the real coordinate, not a hand value.
    String m = latLonToMGRS(40.6892, -74.0445, 4);
    double lat = 0, lon = 0;
    TEST_ASSERT_TRUE(mgrsToLatLon(m.c_str(), lat, lon));
    TEST_ASSERT_TRUE(fabs(lat - 40.6892) < 0.001);
    TEST_ASSERT_TRUE(fabs(lon - (-74.0445)) < 0.001);
}

void test_mgrs_reverse_nospaces() {
    // Spaced vs compact (no spaces) must decode to the same point.
    double sLat = 0, sLon = 0, cLat = 0, cLon = 0;
    TEST_ASSERT_TRUE(mgrsToLatLon("18T WL 8084 0656", sLat, sLon));
    TEST_ASSERT_TRUE(mgrsToLatLon("18TWL80840656",    cLat, cLon));
    TEST_ASSERT_TRUE(fabs(sLat - cLat) < 1e-9);
    TEST_ASSERT_TRUE(fabs(sLon - cLon) < 1e-9);
}

void test_mgrs_reverse_malformed() {
    double lat = 0, lon = 0;
    TEST_ASSERT_FALSE(mgrsToLatLon("not a coord", lat, lon));
    TEST_ASSERT_FALSE(mgrsToLatLon("18T WL 808", lat, lon));   // odd digit count
    TEST_ASSERT_FALSE(mgrsToLatLon("", lat, lon));
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_mgrs_statue_of_liberty);
    RUN_TEST(test_mgrs_london_eye);
    RUN_TEST(test_mgrs_equator_prime_meridian);
    RUN_TEST(test_mgrs_southern_hemisphere);
    RUN_TEST(test_mgrs_norway_exception);
    RUN_TEST(test_mgrs_svalbard_exception);
    RUN_TEST(test_mgrs_outside_utm);
    RUN_TEST(test_mgrs_band_letter_equator);
    RUN_TEST(test_mgrs_band_letter_south);
    RUN_TEST(test_mgrs_precision_1);
    RUN_TEST(test_mgrs_reverse_roundtrip_liberty);
    RUN_TEST(test_mgrs_reverse_roundtrip_london);
    RUN_TEST(test_mgrs_reverse_roundtrip_sydney);
    RUN_TEST(test_mgrs_reverse_roundtrip_paris);
    RUN_TEST(test_mgrs_reverse_roundtrip_equator);
    RUN_TEST(test_mgrs_reverse_literal);
    RUN_TEST(test_mgrs_reverse_nospaces);
    RUN_TEST(test_mgrs_reverse_malformed);
    return UNITY_END();
}

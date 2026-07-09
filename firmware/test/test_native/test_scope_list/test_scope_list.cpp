#include <unity.h>
#include "mesh/ScopeListParser.h"

#include <cstring>

using namespace mclite;

void setUp() {}
void tearDown() {}

// Build a repeater regions reply: [clock:4][names blob]. `blob` is copied
// verbatim (include a trailing '\0' in the literal to exercise NUL handling).
static std::vector<uint8_t> reply(const char* blob, size_t blobLen) {
    std::vector<uint8_t> v = {0xDE, 0xAD, 0xBE, 0xEF};  // 4-byte clock (ignored)
    for (size_t i = 0; i < blobLen; i++) v.push_back((uint8_t)blob[i]);
    return v;
}

// Typical reply: wildcard plus a few named regions.
void test_typical_list() {
    const char* b = "*,roi,ni,scotland";
    auto v = reply(b, strlen(b));
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(4, names.size());
    TEST_ASSERT_EQUAL_STRING("*", names[0].c_str());
    TEST_ASSERT_EQUAL_STRING("roi", names[1].c_str());
    TEST_ASSERT_EQUAL_STRING("ni", names[2].c_str());
    TEST_ASSERT_EQUAL_STRING("scotland", names[3].c_str());
}

// A NUL terminator ends the blob; trailing garbage after it is ignored.
void test_nul_terminates() {
    const char* b = "roi,ni\0GARBAGE";
    auto v = reply(b, 6 + 1 + 7);   // include the NUL and the garbage tail
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(2, names.size());
    TEST_ASSERT_EQUAL_STRING("roi", names[0].c_str());
    TEST_ASSERT_EQUAL_STRING("ni", names[1].c_str());
}

// Single region, no commas.
void test_single_region() {
    const char* b = "roi";
    auto v = reply(b, strlen(b));
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(1, names.size());
    TEST_ASSERT_EQUAL_STRING("roi", names[0].c_str());
}

// '#'/'$'-prefixed names pass through unchanged (scope-field syntax).
void test_prefixed_names_preserved() {
    const char* b = "#roi,$local";
    auto v = reply(b, strlen(b));
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(2, names.size());
    TEST_ASSERT_EQUAL_STRING("#roi", names[0].c_str());
    TEST_ASSERT_EQUAL_STRING("$local", names[1].c_str());
}

// Empty/duplicate commas produce no empty entries.
void test_skips_empty_fields() {
    const char* b = ",roi,,ni,";
    auto v = reply(b, strlen(b));
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(2, names.size());
    TEST_ASSERT_EQUAL_STRING("roi", names[0].c_str());
    TEST_ASSERT_EQUAL_STRING("ni", names[1].c_str());
}

// Empty name blob (clock only) -> no names.
void test_clock_only() {
    std::vector<uint8_t> v = {0x01, 0x02, 0x03, 0x04};
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(0, names.size());
}

// Runt reply shorter than the 4-byte clock -> no names, no crash.
void test_runt_reply() {
    std::vector<uint8_t> v = {0x01, 0x02};
    auto names = parseScopeList(v.data(), (uint8_t)v.size());
    TEST_ASSERT_EQUAL_UINT32(0, names.size());
}

// Null pointer -> no names, no crash.
void test_null_data() {
    auto names = parseScopeList(nullptr, 10);
    TEST_ASSERT_EQUAL_UINT32(0, names.size());
}

int main(int, char**) {
    UNITY_BEGIN();
    RUN_TEST(test_typical_list);
    RUN_TEST(test_nul_terminates);
    RUN_TEST(test_single_region);
    RUN_TEST(test_prefixed_names_preserved);
    RUN_TEST(test_skips_empty_fields);
    RUN_TEST(test_clock_only);
    RUN_TEST(test_runt_reply);
    RUN_TEST(test_null_data);
    return UNITY_END();
}

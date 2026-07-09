#pragma once

#include <Arduino.h>
#include <vector>

// Parser for a repeater's ANON_REQ_TYPE_REGIONS reply — the "valid scope list"
// query (issue #45). A repeater answers an anonymous regions request with its
// RegionMap names, comma-separated, via RegionMap::exportNamesTo(). See the
// MeshCore simple_repeater handleAnonRegionsReq() for the wire format.
//
// Reply payload as delivered to MCLiteMesh::onAnonResponse (which has already
// stripped the leading 4-byte request tag):
//   [repeater_clock : 4][names : comma-separated, NUL-terminated]
// e.g. bytes after the clock: "*,roi,ni,scotland\0"
//
// The wildcard "*" is a legitimate selectable scope (== no scope), so it is kept
// in the returned list when present. Names mirror the scope-field syntax MCLite
// already uses (see scopeToTransportKey): a bare name or a '#'/'$'-prefixed one.

namespace mclite {

// Split a repeater regions reply into ordered scope names.
// Robust to: len < 4 (no clock -> empty), empty name blob, an absent NUL
// terminator (bounded by len), and empty/trailing comma fields (skipped).
inline std::vector<String> parseScopeList(const uint8_t* data, uint8_t len) {
    std::vector<String> out;
    if (!data || len < 4) return out;   // need at least the 4-byte clock prefix

    const uint8_t* p = data + 4;
    const uint8_t  n = (uint8_t)(len - 4);
    String cur;
    for (uint8_t i = 0; i < n; i++) {
        const char c = (char)p[i];
        if (c == '\0') break;           // NUL terminates the name blob
        if (c == ',') {
            if (cur.length() > 0) out.push_back(cur);
            cur = "";
        } else {
            cur += c;
        }
    }
    if (cur.length() > 0) out.push_back(cur);
    return out;
}

}  // namespace mclite

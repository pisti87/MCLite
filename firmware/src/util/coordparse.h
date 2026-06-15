#pragma once

// coordparse — detect the FIRST GPS coordinate inside a free-text message and
// return it as decimal lat/lon. Supports decimal "lat, lon" pairs and
// MGRS / UTMREF strings (via mgrs.h reverse parser). Used to turn shared
// positions in chat into a tap-to-open-map link.
//
// Decimal is tried first, so a "both" formatted message
//   "52.123456, 13.654321 (33U UP 9140 7180)"
// yields a single coordinate (the decimal) — the trailing MGRS is the same
// position and is ignored. No <regex> (heavy on ESP32): plain pointer scanning.

#include <Arduino.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "mgrs.h"

namespace mclite {

struct GeoCoord {
    bool   valid = false;
    double lat   = 0.0;
    double lon   = 0.0;
};

inline bool cp_isNumChar(char ch) {
    return isdigit((unsigned char)ch) || ch == '.' || ch == '-' || ch == '+';
}

// Try to parse a decimal "lat, lon" pair anywhere in `s`. Requires the latitude
// token to contain a '.' (rejects "see you at 5, 6") and both values in range.
inline bool cp_tryDecimal(const char* s, double& lat, double& lon) {
    int len = (int)strlen(s);
    for (int k = 0; k < len; k++) {
        if (s[k] != ',') continue;

        // ---- left token (latitude) ----
        int j = k - 1;
        while (j >= 0 && (s[j] == ' ' || s[j] == '\t')) j--;
        int end = j + 1;                         // one past last char
        while (j >= 0 && cp_isNumChar(s[j])) j--;
        int start = j + 1;
        int llen = end - start;
        if (llen <= 0 || llen >= 31) continue;

        bool hasDot = false;
        for (int t = start; t < end; t++) if (s[t] == '.') hasDot = true;
        if (!hasDot) continue;                   // require a decimal point

        char latBuf[32];
        memcpy(latBuf, s + start, llen);
        latBuf[llen] = '\0';
        char* ep = nullptr;
        double la = strtod(latBuf, &ep);
        if (ep == latBuf || *ep != '\0') continue;

        // ---- right token (longitude) ----
        int r = k + 1;
        while (s[r] == ' ' || s[r] == '\t') r++;
        char* ep2 = nullptr;
        double lo = strtod(s + r, &ep2);
        if (ep2 == s + r) continue;              // nothing parsed

        if (la < -90.0 || la > 90.0 || lo < -180.0 || lo > 180.0) continue;
        lat = la;
        lon = lo;
        return true;
    }
    return false;
}

// Try to find an MGRS / UTMREF token (possibly split across spaces) anywhere in
// the text and convert it to lat/lon. Tries the longest token-combo first so a
// spaced "33U UP 9140 7180" wins over a truncated "33U UP 9140".
inline bool cp_tryMgrs(const String& text, double& lat, double& lon) {
    const char* s = text.c_str();
    int len = (int)text.length();

    // Tokenize on whitespace.
    static const int MAXTOK = 64;
    int ts[MAXTOK], te[MAXTOK], nt = 0;
    int i = 0;
    while (i < len && nt < MAXTOK) {
        while (i < len && (s[i] == ' ' || s[i] == '\t')) i++;
        if (i >= len) break;
        int start = i;
        while (i < len && s[i] != ' ' && s[i] != '\t') i++;
        ts[nt] = start; te[nt] = i; nt++;
    }

    char buf[48];
    for (int a = 0; a < nt; a++) {
        // Start token must begin (after any leading punctuation like '@') with a digit.
        int p = ts[a];
        while (p < te[a] && !isalnum((unsigned char)s[p])) p++;
        if (p >= te[a] || !isdigit((unsigned char)s[p])) continue;

        // Try the longest feasible combo first (up to 4 tokens: zone-band / square
        // / east / north). Clamp the start so a single compact token (m=0) is tried.
        int maxM = nt - 1 - a;
        if (maxM > 3) maxM = 3;
        for (int m = maxM; m >= 0; m--) {
            int bl = 0;
            for (int q = p; q < te[a] && bl < (int)sizeof(buf) - 1; q++) buf[bl++] = s[q];
            for (int x = a + 1; x <= a + m && bl < (int)sizeof(buf) - 1; x++)
                for (int q = ts[x]; q < te[x] && bl < (int)sizeof(buf) - 1; q++) buf[bl++] = s[q];
            buf[bl] = '\0';
            // Trim trailing punctuation (e.g. a closing paren in "(...)").
            while (bl > 0 && !isalnum((unsigned char)buf[bl - 1])) buf[--bl] = '\0';
            if (mgrsToLatLon(buf, lat, lon)) return true;
        }
    }
    return false;
}

// Return the first coordinate found in the message (decimal preferred, then MGRS).
inline GeoCoord parseFirstGeoCoord(const String& text) {
    GeoCoord gc;
    if (cp_tryDecimal(text.c_str(), gc.lat, gc.lon)) { gc.valid = true; return gc; }
    if (cp_tryMgrs(text, gc.lat, gc.lon))            { gc.valid = true; return gc; }
    return gc;
}

} // namespace mclite

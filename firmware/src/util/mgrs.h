#pragma once

#include <Arduino.h>
#include <math.h>
#include <ctype.h>

namespace mclite {

// WGS84 ellipsoid constants
static constexpr double MGRS_A  = 6378137.0;           // Semi-major axis
static constexpr double MGRS_F  = 1.0 / 298.257223563; // Flattening
static constexpr double MGRS_E2 = 2 * MGRS_F - MGRS_F * MGRS_F; // First eccentricity squared
static constexpr double MGRS_K0 = 0.9996;              // UTM scale factor

// Latitude band letters (C-X, excluding I and O)
static const char MGRS_BAND_LETTERS[] = "CDEFGHJKLMNPQRSTUVWX";

// 100km column letters cycle (per zone set 1-6)
static const char MGRS_COL_LETTERS_1[] = "ABCDEFGH";  // Sets 1,4
static const char MGRS_COL_LETTERS_2[] = "JKLMNPQR";  // Sets 2,5
static const char MGRS_COL_LETTERS_3[] = "STUVWXYZ";  // Sets 3,6

// 100km row letters cycle (alternating for odd/even zones)
static const char MGRS_ROW_LETTERS_ODD[]  = "ABCDEFGHJKLMNPQRSTUV"; // 20 letters
static const char MGRS_ROW_LETTERS_EVEN[] = "FGHJKLMNPQRSTUVABCDE"; // 20 letters

inline int utmZoneNumber(double lat, double lon) {
    int zone = (int)((lon + 180.0) / 6.0) + 1;
    // Norway exception
    if (lat >= 56.0 && lat < 64.0 && lon >= 3.0 && lon < 12.0) zone = 32;
    // Svalbard exceptions
    if (lat >= 72.0 && lat < 84.0) {
        if (lon >= 0.0  && lon <  9.0) zone = 31;
        else if (lon >= 9.0  && lon < 21.0) zone = 33;
        else if (lon >= 21.0 && lon < 33.0) zone = 35;
        else if (lon >= 33.0 && lon < 42.0) zone = 37;
    }
    return zone;
}

inline char utmBandLetter(double lat) {
    if (lat < -80.0 || lat > 84.0) return 'Z'; // Outside UTM
    int idx = (int)((lat + 80.0) / 8.0);
    if (idx > 19) idx = 19;
    return MGRS_BAND_LETTERS[idx];
}

// Lat/lon (WGS84) to UTM easting/northing
inline void latLonToUTM(double lat, double lon, int zone,
                        double& easting, double& northing) {
    double latRad = lat * M_PI / 180.0;
    double lonRad = lon * M_PI / 180.0;
    double lonOrigin = (zone - 1) * 6.0 - 180.0 + 3.0;
    double lonOriginRad = lonOrigin * M_PI / 180.0;

    double e2 = MGRS_E2;
    double ep2 = e2 / (1.0 - e2); // Second eccentricity squared

    double sinLat = sin(latRad);
    double cosLat = cos(latRad);
    double tanLat = tan(latRad);

    double N = MGRS_A / sqrt(1.0 - e2 * sinLat * sinLat);
    double T = tanLat * tanLat;
    double C = ep2 * cosLat * cosLat;
    double A = cosLat * (lonRad - lonOriginRad);

    // Meridional arc (M)
    double e4 = e2 * e2;
    double e6 = e4 * e2;
    double M = MGRS_A * ((1.0 - e2/4.0 - 3.0*e4/64.0 - 5.0*e6/256.0) * latRad
             - (3.0*e2/8.0 + 3.0*e4/32.0 + 45.0*e6/1024.0) * sin(2.0*latRad)
             + (15.0*e4/256.0 + 45.0*e6/1024.0) * sin(4.0*latRad)
             - (35.0*e6/3072.0) * sin(6.0*latRad));

    double A2 = A * A;
    double A3 = A2 * A;
    double A4 = A3 * A;
    double A5 = A4 * A;
    double A6 = A5 * A;

    easting = MGRS_K0 * N * (A + (1.0-T+C)*A3/6.0
              + (5.0-18.0*T+T*T+72.0*C-58.0*ep2)*A5/120.0) + 500000.0;

    northing = MGRS_K0 * (M + N * tanLat * (A2/2.0 + (5.0-T+9.0*C+4.0*C*C)*A4/24.0
               + (61.0-58.0*T+T*T+600.0*C-330.0*ep2)*A6/720.0));

    if (lat < 0.0) northing += 10000000.0; // Southern hemisphere offset
}

// Convert lat/lon to MGRS string
// precision: 1=10km, 2=1km, 3=100m, 4=10m, 5=1m
inline String latLonToMGRS(double lat, double lon, int precision = 4) {
    // Clamp to UTM bounds (skip polar UPS)
    if (lat < -80.0 || lat > 84.0) return "Outside UTM";

    int zone = utmZoneNumber(lat, lon);
    char band = utmBandLetter(lat);

    double easting, northing;
    latLonToUTM(lat, lon, zone, easting, northing);

    // 100km square identification
    int setNumber = ((zone - 1) % 6) + 1;

    // Column letter
    int col100k = (int)(easting / 100000.0);
    if (col100k < 1) col100k = 1;  // Defensive: easting always >= 100km in valid UTM
    const char* colLetters;
    switch (((setNumber - 1) % 3)) {
        case 0: colLetters = MGRS_COL_LETTERS_1; break;
        case 1: colLetters = MGRS_COL_LETTERS_2; break;
        default: colLetters = MGRS_COL_LETTERS_3; break;
    }
    char colLetter = colLetters[(col100k - 1) % 8];

    // Row letter
    int row100k = (int)(fmod(northing, 2000000.0) / 100000.0);
    const char* rowLetters = (setNumber % 2 != 0)
        ? MGRS_ROW_LETTERS_ODD : MGRS_ROW_LETTERS_EVEN;
    char rowLetter = rowLetters[row100k % 20];

    // Grid coordinates within 100km square
    int eastGrid  = (int)fmod(easting, 100000.0);
    int northGrid = (int)fmod(northing, 100000.0);

    // Truncate to requested precision
    int divisor = 1;
    for (int i = precision; i < 5; i++) divisor *= 10;
    eastGrid  /= divisor;
    northGrid /= divisor;

    // Format: "33U UP 9140 7180" (for precision=4)
    char buf[32];
    snprintf(buf, sizeof(buf), "%d%c %c%c %0*d %0*d",
             zone, band, colLetter, rowLetter,
             precision, eastGrid, precision, northGrid);
    return String(buf);
}

// ---- Reverse: MGRS / UTMREF → lat/lon ----------------------------------------
// (Inverse of the forward functions above; uses the same WGS84 constants and the
// same 100km lettering scheme so a forward→reverse round-trip is consistent.)

// Inverse UTM projection: easting/northing (meters) in a zone → lat/lon (deg).
// `north` selects the hemisphere (northing measured from the equator either way;
// the caller must already have removed the 10,000,000 m southern false-northing).
inline bool utmToLatLon(double easting, double northing, int zone, bool north,
                        double& lat, double& lon) {
    if (zone < 1 || zone > 60) return false;

    double e2 = MGRS_E2;
    double e1 = (1.0 - sqrt(1.0 - e2)) / (1.0 + sqrt(1.0 - e2));
    double ep2 = e2 / (1.0 - e2);

    double x = easting - 500000.0;          // remove false easting
    double y = northing;
    if (!north) y -= 10000000.0;            // remove southern false-northing → equator-relative

    double M = y / MGRS_K0;
    double mu = M / (MGRS_A * (1.0 - e2/4.0 - 3.0*e2*e2/64.0 - 5.0*e2*e2*e2/256.0));

    double e1_2 = e1 * e1, e1_3 = e1_2 * e1, e1_4 = e1_3 * e1;
    double phi1 = mu
        + (3.0*e1/2.0 - 27.0*e1_3/32.0) * sin(2.0*mu)
        + (21.0*e1_2/16.0 - 55.0*e1_4/32.0) * sin(4.0*mu)
        + (151.0*e1_3/96.0) * sin(6.0*mu)
        + (1097.0*e1_4/512.0) * sin(8.0*mu);

    double sinPhi1 = sin(phi1), cosPhi1 = cos(phi1), tanPhi1 = tan(phi1);
    double N1 = MGRS_A / sqrt(1.0 - e2 * sinPhi1 * sinPhi1);
    double T1 = tanPhi1 * tanPhi1;
    double C1 = ep2 * cosPhi1 * cosPhi1;
    double R1 = MGRS_A * (1.0 - e2) / pow(1.0 - e2 * sinPhi1 * sinPhi1, 1.5);
    double D = x / (N1 * MGRS_K0);

    double D2 = D*D, D3 = D2*D, D4 = D3*D, D5 = D4*D, D6 = D5*D;

    double latRad = phi1 - (N1 * tanPhi1 / R1) *
        (D2/2.0
         - (5.0 + 3.0*T1 + 10.0*C1 - 4.0*C1*C1 - 9.0*ep2) * D4/24.0
         + (61.0 + 90.0*T1 + 298.0*C1 + 45.0*T1*T1 - 252.0*ep2 - 3.0*C1*C1) * D6/720.0);

    double lonOrigin = (zone - 1) * 6.0 - 180.0 + 3.0;
    double lonRad =
        (D
         - (1.0 + 2.0*T1 + C1) * D3/6.0
         + (5.0 - 2.0*C1 + 28.0*T1 - 3.0*C1*C1 + 8.0*ep2 + 24.0*T1*T1) * D5/120.0)
        / cosPhi1;

    lat = latRad * 180.0 / M_PI;
    lon = lonOrigin + lonRad * 180.0 / M_PI;
    return true;
}

// Find a letter in a NUL-terminated set; returns index or -1.
inline int mgrsFindLetter(const char* set, char c) {
    for (int i = 0; set[i]; i++) if (set[i] == c) return i;
    return -1;
}

// Parse an MGRS / UTMREF string (e.g. "33U UP 9140 7180", spaces optional) into
// lat/lon. Handles any precision (1–5 digit pairs). Returns false if malformed.
inline bool mgrsToLatLon(const char* s, double& lat, double& lon) {
    if (!s) return false;

    // Collect alnum chars (strip spaces) into a compact buffer.
    char c[40];
    int n = 0;
    for (const char* p = s; *p && n < (int)sizeof(c) - 1; p++) {
        if (*p == ' ' || *p == '\t') continue;
        char ch = *p;
        if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';  // upper-case
        c[n++] = ch;
    }
    c[n] = '\0';
    if (n < 5) return false;

    int i = 0;
    // Zone: 1–2 digits
    if (!isdigit((unsigned char)c[0])) return false;
    int zone = c[i++] - '0';
    if (isdigit((unsigned char)c[i])) zone = zone * 10 + (c[i++] - '0');
    if (zone < 1 || zone > 60) return false;

    // Band letter (C–X excl. I,O)
    char band = c[i++];
    int bandIdx = mgrsFindLetter(MGRS_BAND_LETTERS, band);
    if (bandIdx < 0) return false;
    bool north = band >= 'N';

    // Two 100km square letters
    if (i + 2 > n) return false;
    char colLetter = c[i++];
    char rowLetter = c[i++];

    // Remaining must be an even count of digits (easting/northing halves)
    int digitsStart = i;
    int digitCount = 0;
    while (c[i] && isdigit((unsigned char)c[i])) { i++; digitCount++; }
    if (c[i] != '\0' || digitCount == 0 || (digitCount & 1)) return false;
    int half = digitCount / 2;
    if (half > 5) return false;

    long eVal = 0, nVal = 0;
    for (int k = 0; k < half; k++)        eVal = eVal * 10 + (c[digitsStart + k] - '0');
    for (int k = 0; k < half; k++)        nVal = nVal * 10 + (c[digitsStart + half + k] - '0');
    // Scale to meters (precision `half`: half==5 → 1m, half==4 → 10m, …)
    double scale = 1.0;
    for (int k = half; k < 5; k++) scale *= 10.0;
    double eastGrid  = eVal * scale;
    double northGrid = nVal * scale;

    // Reconstruct 100km column easting (same scheme as forward)
    int setNumber = ((zone - 1) % 6) + 1;
    const char* colLetters;
    switch (((setNumber - 1) % 3)) {
        case 0: colLetters = MGRS_COL_LETTERS_1; break;
        case 1: colLetters = MGRS_COL_LETTERS_2; break;
        default: colLetters = MGRS_COL_LETTERS_3; break;
    }
    int colIdx = mgrsFindLetter(colLetters, colLetter);
    if (colIdx < 0) return false;
    double easting = (colIdx + 1) * 100000.0 + eastGrid;  // col100k = colIdx+1

    // Reconstruct 100km row northing (mod 2,000,000) then resolve the band.
    const char* rowLetters = (setNumber % 2 != 0)
        ? MGRS_ROW_LETTERS_ODD : MGRS_ROW_LETTERS_EVEN;
    int rowIdx = mgrsFindLetter(rowLetters, rowLetter);
    if (rowIdx < 0) return false;
    double northBase = rowIdx * 100000.0 + northGrid;  // northing mod 2,000,000

    // Resolve the 2,000,000 m ambiguity: pick the candidate nearest the band's
    // approximate northing. latLonToUTM returns the *stored* northing (it already
    // adds the 10,000,000 m southern offset), matching northBase's convention, so
    // utmToLatLon then removes that offset itself.
    double bandLat = -80.0 + bandIdx * 8.0 + 4.0;  // center of the 8° band
    double approxE, approxN;
    latLonToUTM(bandLat, (zone - 1) * 6.0 - 180.0 + 3.0, zone, approxE, approxN);

    double northing = northBase;
    while (northing + 1000000.0 < approxN) northing += 2000000.0;
    while (northing - 1000000.0 > approxN) northing -= 2000000.0;

    return utmToLatLon(easting, northing, zone, north, lat, lon);
}

} // namespace mclite

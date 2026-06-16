#pragma once

// Location-advert precision (privacy obfuscation).
//
// Meshtastic-style grid snapping: a coordinate scaled to MeshCore's 1e7
// fixed-point integer has its low (32 - precision) bits cleared, snapping it to
// a coarse grid, then re-centred in the cell. `precision` is in bits:
//   32      = exact (no change)
//   10..31  = coarsened (smaller = coarser; see locPrecisionMeters)
//   (0 means "don't advertise location at all" — handled by the caller)
//
// ONLY used for the broadcast location advert. Telemetry responses to authorized
// contacts and the in-chat GPS insert always use exact coordinates.
//
// Pure (no Arduino) so it's unit-testable on the host.

#include <stdint.h>
#include <math.h>

namespace mclite {

// Snap a WGS84 coordinate (degrees) to the precision grid, centred in the cell.
inline double obfuscateCoord(double coord, uint8_t precision) {
    if (precision >= 32) return coord;                 // exact — no change
    uint8_t shift = (uint8_t)(32 - precision);         // 1..22 for precision 10..31
    int32_t scaled = (int32_t)lround(coord * 1e7);     // MeshCore fixed-point scale
    int32_t mask   = (int32_t)(0xFFFFFFFFu << shift);  // clear the low `shift` bits
    int32_t cell   = (scaled & mask) + (1 << (shift - 1));  // snap to grid + centre in cell
    return cell / 1e7;
}

// Approximate grid step (metres) for a precision level — for UI labels.
// One latitude degree ~= 111320 m; longitude varies with latitude, so this is a
// nominal figure for display only.
inline uint32_t locPrecisionMeters(uint8_t precision) {
    if (precision >= 32) return 0;                     // exact
    uint8_t shift = (uint8_t)(32 - precision);
    double stepDeg = (double)(1u << shift) / 1e7;
    return (uint32_t)(stepDeg * 111320.0 + 0.5);
}

}  // namespace mclite

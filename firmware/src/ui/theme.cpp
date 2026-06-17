#include "theme.h"
#include "../config/ConfigManager.h"
#include <string.h>

namespace mclite {
namespace theme {

// The live palette. Defaults to DARK until applyThemeFromConfig() runs at boot.
Palette ACTIVE = PALETTE_DARK;

const Palette* builtinPaletteByName(const char* name) {
    if (!name) return nullptr;
    if (!strcmp(name, "dark"))          return &PALETTE_DARK;
    if (!strcmp(name, "light"))         return &PALETTE_LIGHT;
    if (!strcmp(name, "amber"))         return &PALETTE_AMBER;
    if (!strcmp(name, "high_contrast")) return &PALETTE_HIGHCON;
    return nullptr;
}

void applyThemeFromConfig() {
    const auto& cfg = ConfigManager::instance().config();
    const Palette* p = builtinPaletteByName(cfg.display.theme.c_str());
    // (Custom themes from cfg.display.customThemes are resolved in a later phase.)
    ACTIVE = p ? *p : PALETTE_DARK;
}

}  // namespace theme
}  // namespace mclite

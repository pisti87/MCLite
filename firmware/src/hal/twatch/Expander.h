#pragma once

#include <cstdint>

namespace mclite {

// XL9555 I/O expander wrapper. Used to control display power, touch reset,
// LoRa antenna RF switch, haptic enable, and SD card detect on T-Watch Ultra.
// Wire.begin() must be called before init().
class Expander {
public:
    static Expander& instance();

    bool init();
    void writePin(uint8_t pin, bool high);
    bool readPin(uint8_t pin);

    bool isReady() const { return _ready; }

private:
    Expander() = default;
    bool _ready = false;
};

}  // namespace mclite

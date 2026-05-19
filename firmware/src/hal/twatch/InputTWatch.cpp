#include "hal/twatch/InputTWatch.h"
#include "hal/twatch/TouchTWatch.h"
#include "hal/Display.h"
#include "hal/boards/board.h"
#include <Arduino.h>

namespace mclite {

void InputTWatch::init() {
    Display::instance().setBootStatus("Touchscreen...");
    TouchTWatch::instance().init();

    // Boot button on GPIO 0. Strapping pin: ESP32-S3 enters download mode if
    // held during reset, so we use the _seenRelease guard (copied from
    // Trackball::updatePress) to ignore the pin until we see it go HIGH once.
    pinMode(TWATCH_BOOT_BUTTON, INPUT_PULLUP);
}

void InputTWatch::updatePress() {
    bool pinDown = (digitalRead(TWATCH_BOOT_BUTTON) == LOW);

    if (!_seenRelease) {
        if (!pinDown) _seenRelease = true;
        return;
    }

    if (pinDown && !_pressing) {
        _pressing = true;
        _pressStartMs = millis();
    } else if (!pinDown && _pressing) {
        _lastHoldMs = millis() - _pressStartMs;
        _pressing = false;
    }
}

uint32_t InputTWatch::holdDurationMs() {
    if (!_pressing) return 0;
    return millis() - _pressStartMs;
}

bool InputTWatch::isTouched() {
    return TouchTWatch::instance().isTouched();
}

bool InputTWatch::has(InputCapability /*cap*/) const {
    // Only Keyboard is queried today (main.cpp:handleKeyShortcuts). T-Watch
    // has no QWERTY — handleKeyShortcuts becomes a no-op on this board.
    return false;
}

IInput& IInput::instance() {
    static InputTWatch inst;
    return inst;
}

}  // namespace mclite

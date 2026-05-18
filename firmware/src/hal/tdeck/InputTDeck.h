#pragma once

#include "hal/IInput.h"

namespace mclite {

class InputTDeck : public IInput {
public:
    void init() override;

    char pollKey() override;
    void clearKey() override;

    void updatePress() override;
    bool isPressed() override;
    bool hasMoved() override;
    uint32_t holdDurationMs() override;
    uint32_t lastHoldDuration() override;

    bool isTouched() override;

    void setBacklight(uint8_t level) override;

    bool has(InputCapability cap) const override;

    void attachToGroup(lv_group_t* group) override;
};

}  // namespace mclite

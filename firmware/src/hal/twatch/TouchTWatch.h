#pragma once

#include <lvgl.h>

namespace mclite {

// CST9217 capacitive touch wrapper for T-Watch Ultra. The CST9217 sits on the
// shared I2C bus (SDA=3, SCL=2). Touch reset is driven via the XL9555
// expander pin (TWATCH_EXP_TOUCH_RST), not a direct GPIO — so the expander
// must be initialised before init() is called.
class TouchTWatch {
public:
    static TouchTWatch& instance();

    bool init();
    bool isTouched() const { return _touched; }
    lv_indev_t* indev() { return _indev; }

private:
    TouchTWatch() = default;
    lv_indev_t*    _indev = nullptr;
    lv_indev_drv_t _drv;
    bool           _touched = false;

    static void readCb(lv_indev_drv_t* drv, lv_indev_data_t* data);
};

}  // namespace mclite

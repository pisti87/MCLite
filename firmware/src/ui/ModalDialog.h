#pragma once

#include <lvgl.h>
#include <Arduino.h>
#include <functional>
#include <vector>

namespace mclite {

// Shared modal dialog — one consistent look for every confirm / chooser / info
// popup. A centered MODAL_TEXT_WIDTH card with an optional wrapped title, an
// optional scrollable body, and full-width buttons stacked one per row (with a
// proper encoder group for trackball). Mirrors lv_msgbox usage: show() returns
// the dialog handle and the caller closes it from the callback via close().
class ModalDialog {
public:
    // cb(dlg, idx): idx = pressed button (0-based). The dialog stays open until
    // the caller calls ModalDialog::close(dlg) (so "Refresh"-style buttons can
    // keep it up). Safe to call close() from inside cb.
    using Callback = std::function<void(lv_obj_t* dlg, int idx)>;

    // smallTail (optional): extra text rendered below the body in a smaller,
    // secondary-color font inside the same scrollable area — used for raw
    // public keys so they stay compact and don't dominate the dialog.
    static lv_obj_t* show(const String& title, const String& body,
                          const std::vector<String>& buttons, Callback cb,
                          const String& smallTail = "");
    static void close(lv_obj_t* dlg);
    static void setBody(lv_obj_t* dlg, const String& body);  // update scrollable body text
};

}  // namespace mclite

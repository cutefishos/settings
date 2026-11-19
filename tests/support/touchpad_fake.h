// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <mutex>

#include "base/dbus.h"

namespace settings::testing {

// The compositor's org.cutefish.Touchpad1 as a stand-in, for a pad with
// physical buttons: no choice of secondary click.
struct FakeTouchpad
{
    std::mutex mutex;
    bool tapToClick = false;
    double pointerAcceleration = 0;
    double scrollSpeed = 0.5;
    cutefish::base::dbus::Object* object = nullptr;
};

inline std::shared_ptr<void> serveTouchpad(cutefish::base::dbus::Connection& bus, FakeTouchpad& fake)
{
    auto object = std::make_shared<cutefish::base::dbus::Object>(&bus, "/org/cutefish/Touchpad1",
                                                                 "org.cutefish.Touchpad1");
    object->addProperty("available", [] { return true; });
    object->addProperty("enabled", [] { return true; });
    object->addProperty("disableWithMouse", [] { return false; });
    object->addProperty("disableWhileTyping", [] { return true; });
    object->addProperty("tapToClick", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.tapToClick;
    });
    object->addProperty("tapDragging", [] { return false; });
    object->addProperty("naturalScroll", [] { return true; });
    object->addProperty("pointerAcceleration", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.pointerAcceleration;
    });
    object->addProperty("scrollSpeed", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.scrollSpeed;
    });
    object->addProperty("secondaryClickConfigurable", [] { return false; });
    object->addProperty("twoFingerSecondaryClick", [] { return true; });
    object->addSignal("tapToClickChanged", "");
    object->addSignal("scrollSpeedChanged", "");
    object->addMethod("setTapToClick", [&fake, object = object.get()](bool enabled) {
        {
            std::lock_guard guard(fake.mutex);
            fake.tapToClick = enabled;
        }
        object->emitSignal("tapToClickChanged");
    });
    object->addMethod("setScrollSpeed", [&fake, object = object.get()](double speed) {
        {
            std::lock_guard guard(fake.mutex);
            fake.scrollSpeed = speed;
        }
        object->emitSignal("scrollSpeedChanged");
    });
    object->publish();
    fake.object = object.get();
    return object;
}

} // namespace settings::testing

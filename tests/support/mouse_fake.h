// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <mutex>
#include <string>

#include "base/dbus.h"

namespace settings::testing {

// The compositor's org.cutefish.Mouse1 as a stand-in: its settings outlive a
// restart of the service, as the compositor keeps them in a file.
struct FakeMouse
{
    std::mutex mutex;
    bool leftHanded = false;
    bool naturalScroll = true;
    bool acceleration = true;
    double pointerAcceleration = 0;
    // On the service thread only.
    cutefish::base::dbus::Object* object = nullptr;
};

template<typename T>
void addMouseSetting(cutefish::base::dbus::Object& object, FakeMouse& fake, const std::string& name,
                     const std::string& setter, T FakeMouse::*field)
{
    object.addProperty(name, [&fake, field] {
        std::lock_guard guard(fake.mutex);
        return fake.*field;
    });
    object.addSignal(name + "Changed", "");
    object.addMethod(setter, [&fake, &object, field, name](T value) {
        {
            std::lock_guard guard(fake.mutex);
            fake.*field = value;
        }
        object.emitSignal(name + "Changed");
    });
}

inline std::shared_ptr<void> serveMouse(cutefish::base::dbus::Connection& bus, FakeMouse& fake)
{
    auto object =
            std::make_shared<cutefish::base::dbus::Object>(&bus, "/org/cutefish/Mouse1", "org.cutefish.Mouse1");
    object->addProperty("available", [] { return true; });
    object->addSignal("availableChanged", "");
    addMouseSetting(*object, fake, "leftHanded", "setLeftHanded", &FakeMouse::leftHanded);
    addMouseSetting(*object, fake, "naturalScroll", "setNaturalScroll", &FakeMouse::naturalScroll);
    addMouseSetting(*object, fake, "acceleration", "setAcceleration", &FakeMouse::acceleration);
    addMouseSetting(*object, fake, "pointerAcceleration", "setPointerAcceleration", &FakeMouse::pointerAcceleration);
    object->publish();
    fake.object = object.get();
    return object;
}

} // namespace settings::testing

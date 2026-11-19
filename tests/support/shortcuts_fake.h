// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

// A stand-in for the compositor's org.cutefish.Shortcuts1 service, answering
// in the JSON the real one produces.

#pragma once

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base/dbus.h"

namespace settings::testing {

inline std::string accelerator(int keysym, const std::string& keyName, const std::string& keyText, int modifiers,
                               const std::string& type = "default")
{
    return "{\"keysym\":" + std::to_string(keysym) + ",\"keyName\":\"" + keyName + "\",\"keyText\":\"" + keyText
            + "\",\"modifiers\":" + std::to_string(modifiers) + ",\"keyState\":\"pressed\",\"type\":\"" + type
            + "\"}";
}

// What the stand-in serves; shared with the test thread under the mutex.
struct FakeShortcuts
{
    std::mutex mutex;
    std::string convention = "pc";
    std::string maximize = "[" + accelerator(0x3d, "equal", "=", 4) + "," + accelerator(0xff52, "Up", "", 8) + "]";
    std::string maximizeDefaults = maximize;
    std::string lock = "[" + accelerator(0x6c, "l", "L", 8) + "]";
    std::vector<std::string> calls;
    std::string nextResult = "{\"result\":\"success\"}";
    // On the service thread only.
    cutefish::base::dbus::Object* object = nullptr;

    std::vector<std::string> takeCalls()
    {
        std::lock_guard guard(mutex);
        return calls;
    }

    void record(std::string call)
    {
        std::lock_guard guard(mutex);
        calls.push_back(std::move(call));
    }

    std::string accelerators()
    {
        std::lock_guard guard(mutex);
        return "{\"actions\":[{\"name\":\"lock-screen\",\"accelerators\":" + lock + ",\"defaults\":" + lock
                + "},{\"name\":\"toggle-maximized\",\"accelerators\":" + maximize
                + ",\"defaults\":" + maximizeDefaults
                + "},{\"name\":\"not-a-cutefish-action\",\"accelerators\":[],\"defaults\":[]}]}";
    }
};

inline std::shared_ptr<void> serveShortcuts(cutefish::base::dbus::Connection& bus, FakeShortcuts& fake)
{
    auto object = std::make_shared<cutefish::base::dbus::Object>(&bus, "/org/cutefish/Shortcuts1",
                                                                 "org.cutefish.Shortcuts1");
    object->addProperty("shortcutConvention", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.convention;
    });
    object->addMethod("accelerators", [&fake] { return fake.accelerators(); });
    object->addMethod("addAccelerator", [&fake](const std::string& action, uint32_t keycode, uint32_t keysym,
                                                uint32_t modifiers) {
        fake.record("add " + action + " " + std::to_string(keycode) + " " + std::to_string(keysym) + " "
                    + std::to_string(modifiers));
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("removeAccelerator", [&fake](const std::string& action, uint32_t keysym, uint32_t modifiers) {
        fake.record("remove " + action + " " + std::to_string(keysym) + " " + std::to_string(modifiers));
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("replaceAccelerator", [&fake](const std::string& action, uint32_t oldKeysym,
                                                    uint32_t oldModifiers, uint32_t keycode, uint32_t keysym,
                                                    uint32_t modifiers) {
        fake.record("replace " + action + " " + std::to_string(oldKeysym) + " " + std::to_string(oldModifiers) + " "
                    + std::to_string(keycode) + " " + std::to_string(keysym) + " " + std::to_string(modifiers));
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("restoreDefault", [&fake](const std::string& action) {
        fake.record("restore " + action);
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("restoreAllDefaults", [&fake] {
        fake.record("restore-all");
        std::lock_guard guard(fake.mutex);
        return fake.nextResult;
    });
    object->addMethod("describeKey", [&fake](uint32_t keycode, uint32_t keysym) {
        fake.record("describe " + std::to_string(keycode) + " " + std::to_string(keysym));
        return std::string("{\"keysym\":113,\"keyName\":\"q\",\"keyText\":\"Q\"}");
    });
    object->addMethod("preventProcessingAccelerators",
                      [&fake](bool prevent) { fake.record("prevent " + std::to_string(prevent ? 1 : 0)); });
    object->addMethod("hasCustomAccelerators", [&fake] {
        std::lock_guard guard(fake.mutex);
        return fake.maximize != fake.maximizeDefaults;
    });
    object->addSignal("acceleratorsChanged", "");
    object->publish();
    fake.object = object.get();
    return object;
}

} // namespace settings::testing

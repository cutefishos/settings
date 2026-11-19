// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtest/gtest.h>

#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "base/dbus.h"
#include "ui/application.h"

namespace settings::testing {

using UpdateEntry = std::tuple<std::string, std::string, std::string>;

// cutefish-services' software update object, and PackageKit's name when
// the test wants it, on a connection of their own on the test's session bus.
class FakeService
{
public:
    FakeService()
        : m_bus(cutefish::base::dbus::Connection::open(cutefish::base::dbus::Connection::Bus::Session,
                                                       &cutefish::ui::Application::instance()->loop()))
    {
        m_object = std::make_unique<cutefish::base::dbus::Object>(
                m_bus.get(), "/org/cutefish/Services/SoftwareUpdate", "org.cutefish.Services.SoftwareUpdate");
        m_object->addProperty("state", [this] { return state; });
        m_object->addProperty("updates", [this] { return updates; });
        m_object->addProperty("phase", [this] { return phase; });
        m_object->addProperty("progress", [this] { return progress; });
        m_object->addProperty("canCancel", [this] { return canCancel; });
        m_object->addProperty("error", [this] { return error; });
        m_object->addMethod("check", [this] { calls.push_back("check"); });
        m_object->addMethod("update", [this] { calls.push_back("update"); });
        m_object->addMethod("cancel", [this] { calls.push_back("cancel"); });
        m_object->addMethod("restart", [this] { calls.push_back("restart"); });
        EXPECT_TRUE(m_object->publish());
        EXPECT_TRUE(m_bus->requestName("org.cutefish.Services"));
    }

    bool ownPackageKit() { return m_bus->requestName("org.freedesktop.PackageKit"); }

    void change()
    {
        m_object->emitPropertiesChanged({"state", "updates", "phase", "progress", "canCancel", "error"});
    }

    std::string state = "available";
    std::vector<UpdateEntry> updates = {
        {"openssl", "3.5.4-1~deb13u2", "Secure Sockets Layer toolkit"},
        {"tzdata", "2026f-0+deb13u1", "time zone and daylight-saving time data"},
    };
    std::string phase = "downloading";
    int32_t progress = -1;
    bool canCancel = false;
    std::string error;
    std::vector<std::string> calls;

private:
    std::unique_ptr<cutefish::base::dbus::Connection> m_bus;
    std::unique_ptr<cutefish::base::dbus::Object> m_object;
};

} // namespace settings::testing

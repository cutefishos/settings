// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <systemd/sd-bus.h>

#include <algorithm>
#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "base/dbus.h"

namespace settings::testing {

// net.reactivated.Fprint with one reader, as fprintd shows it.
struct FakeFprintd
{
    std::mutex mutex;
    std::vector<std::string> enrolled;
    // What EnrollStart answers instead of starting, when set.
    std::string enrollError;
    std::string claimError;
    int stages = 6;
    std::string scanType = "press";
    std::string enrolling;
    std::atomic<int> claims = 0;
    std::atomic<int> releases = 0;
    std::atomic<int> enrollStops = 0;
    cutefish::base::dbus::Object* device = nullptr;

    // On the service thread: what the driver reports for the finger being
    // recorded; a completed print is stored.
    void status(const std::string& result, bool done)
    {
        if (done && result == "enroll-completed") {
            std::lock_guard guard(mutex);
            enrolled.push_back(enrolling);
        }
        device->emitSignal("EnrollStatus", result, done);
    }

    std::vector<std::string> storedFingers()
    {
        std::lock_guard guard(mutex);
        return enrolled;
    }
};

namespace detail {

// The property names have hyphens, which the toolkit's exported properties
// (and sd-bus) refuse, so GetAll is answered by hand.
inline int fprintProperties(sd_bus_message* message, void* userdata, sd_bus_error*)
{
    if (!sd_bus_message_is_method_call(message, "org.freedesktop.DBus.Properties", "GetAll"))
        return 0;
    auto* fake = static_cast<FakeFprintd*>(userdata);
    sd_bus_message* reply = nullptr;
    sd_bus_message_new_method_return(message, &reply);
    sd_bus_message_open_container(reply, 'a', "{sv}");
    sd_bus_message_append(reply, "{sv}", "name", "s", "Test Sensor");
    sd_bus_message_append(reply, "{sv}", "scan-type", "s", fake->scanType.c_str());
    sd_bus_message_append(reply, "{sv}", "num-enroll-stages", "i", int32_t(fake->stages));
    sd_bus_message_close_container(reply);
    sd_bus_send(nullptr, reply, nullptr);
    sd_bus_message_unref(reply);
    return 1;
}

struct FprintObjects
{
    FprintObjects(cutefish::base::dbus::Connection& bus)
        : manager(&bus, "/net/reactivated/Fprint/Manager", "net.reactivated.Fprint.Manager")
        , device(&bus, "/net/reactivated/Fprint/Device/0", "net.reactivated.Fprint.Device")
    {
    }
    ~FprintObjects() { sd_bus_slot_unref(properties); }

    cutefish::base::dbus::Object manager;
    cutefish::base::dbus::Object device;
    sd_bus_slot* properties = nullptr;
};

} // namespace detail

inline std::shared_ptr<void> serveFprintd(cutefish::base::dbus::Connection& bus, FakeFprintd& fake)
{
    using namespace cutefish::base::dbus;
    auto objects = std::make_shared<detail::FprintObjects>(bus);
    objects->manager.addMethod("GetDefaultDevice", "", "o", [](MethodCall& call) {
        call.reply(ObjectPath{"/net/reactivated/Fprint/Device/0"});
    });
    Object& device = objects->device;
    device.addMethod("Claim", "s", "", [&fake](MethodCall& call) {
        if (!fake.claimError.empty()) {
            call.replyError(fake.claimError, "claim refused");
            return;
        }
        ++fake.claims;
        call.reply();
    });
    device.addMethod("Release", "", "", [&fake](MethodCall& call) {
        ++fake.releases;
        call.reply();
    });
    device.addMethod("ListEnrolledFingers", "s", "as", [&fake](MethodCall& call) {
        std::lock_guard guard(fake.mutex);
        if (fake.enrolled.empty()) {
            call.replyError("net.reactivated.Fprint.Error.NoEnrolledPrints", "No fingerprints enrolled");
            return;
        }
        call.reply(fake.enrolled);
    });
    device.addMethod("DeleteEnrolledFinger", "s", "", [&fake](MethodCall& call) {
        std::string finger;
        call.read(finger);
        std::lock_guard guard(fake.mutex);
        std::erase(fake.enrolled, finger);
        call.reply();
    });
    device.addMethod("EnrollStart", "s", "", [&fake](MethodCall& call) {
        if (!fake.enrollError.empty()) {
            call.replyError(fake.enrollError, "Not Authorized: net.reactivated.fprint.device.enroll");
            return;
        }
        std::lock_guard guard(fake.mutex);
        call.read(fake.enrolling);
        call.reply();
    });
    device.addMethod("EnrollStop", "", "", [&fake](MethodCall& call) {
        ++fake.enrollStops;
        call.reply();
    });
    device.addSignal("EnrollStatus", "sb");
    sd_bus_add_object(bus.bus(), &objects->properties, "/net/reactivated/Fprint/Device/0",
                      detail::fprintProperties, &fake);
    objects->manager.publish();
    device.publish();
    fake.device = &device;
    return objects;
}

} // namespace settings::testing

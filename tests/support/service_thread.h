// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "base/dbus.h"
#include "base/event_loop.h"
#include "base/timer.h"

namespace settings::testing {

// Runs the test thread's loop until the condition holds or the time is up,
// for the logic tests, which have no driver's.
inline bool waitFor(const std::function<bool()>& condition,
                    std::chrono::milliseconds timeout = std::chrono::milliseconds(3000))
{
    if (condition())
        return true;
    cutefish::base::EventLoop& loop = *cutefish::base::EventLoop::current();
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    cutefish::base::Timer timer(&loop);
    timer.onTimeout = [&] {
        if (condition() || std::chrono::steady_clock::now() >= deadline)
            loop.quit();
    };
    timer.start(std::chrono::milliseconds(5));
    loop.run();
    return condition();
}

// A D-Bus service on a thread of its own, with its own loop and connection,
// as the compositor serves it from another process. setup() exports the
// objects on the service thread and returns what keeps them alive.
class ServiceThread
{
public:
    using Setup = std::function<std::shared_ptr<void>(cutefish::base::dbus::Connection& bus)>;

    ServiceThread(std::string name, Setup setup)
        : m_name(std::move(name))
        , m_setup(std::move(setup))
    {
    }

    ~ServiceThread() { stop(); }

    // False when another connection owns the name: the code under test would
    // talk to that service instead.
    [[nodiscard]] bool start()
    {
        std::unique_lock lock(m_mutex);
        m_ready = false;
        m_owned = false;
        m_thread = std::thread([this] {
            cutefish::base::EventLoop loop;
            std::unique_ptr<cutefish::base::dbus::Connection> bus =
                    cutefish::base::dbus::Connection::open(cutefish::base::dbus::Connection::Bus::Session, &loop);
            std::shared_ptr<void> objects = m_setup(*bus);
            const bool owned = bus->requestName(m_name);
            {
                std::lock_guard guard(m_mutex);
                m_loop = &loop;
                m_owned = owned;
                m_ready = true;
            }
            m_readyChanged.notify_all();
            loop.run();
            objects.reset();
            bus.reset();
        });
        m_readyChanged.wait(lock, [this] { return m_ready; });
        return m_owned;
    }

    void stop()
    {
        if (!m_thread.joinable())
            return;
        post([] { cutefish::base::EventLoop::current()->quit(); });
        m_thread.join();
        std::lock_guard guard(m_mutex);
        m_loop = nullptr;
    }

    // Runs the task on the service thread.
    void post(std::function<void()> task)
    {
        std::lock_guard guard(m_mutex);
        if (m_loop)
            m_loop->post(std::move(task));
    }

private:
    std::string m_name;
    Setup m_setup;
    std::thread m_thread;
    std::mutex m_mutex;
    std::condition_variable m_readyChanged;
    cutefish::base::EventLoop* m_loop = nullptr;
    bool m_ready = false;
    bool m_owned = false;
};

// Waits for a stopped service's name to leave the bus.
inline bool waitForNameGone(const std::string& name)
{
    const cutefish::base::dbus::Proxy bus(cutefish::base::dbus::Connection::session(), "org.freedesktop.DBus",
                                          "/org/freedesktop/DBus", "org.freedesktop.DBus");
    return waitFor([&] { return !bus.call("NameHasOwner", name).value<bool>().value_or(false); });
}

// Whether a session bus is there for services to run on.
inline bool hasSessionBus()
{
    return cutefish::base::dbus::Connection::session() != nullptr;
}

} // namespace settings::testing

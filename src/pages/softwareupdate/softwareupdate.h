// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <string>
#include <vector>

#include "base/dbus.h"
#include "base/signal.h"

namespace settings {

// cutefish-services' org.cutefish.Services.SoftwareUpdate: the pending system
// updates, which the service checks and installs while the session runs.
// Read asynchronously and kept up to date, so the getters never block.
class SoftwareUpdate
{
public:
    enum class State {
        Unknown,
        Checking,
        UpToDate,
        Available,
        Updating,
        // Installed in this boot; a restart finishes them.
        RestartSuggested,
        Error,
    };

    enum class Phase {
        Downloading,
        Installing,
    };

    struct Update
    {
        std::string name;
        std::string version;
        std::string summary;

        bool operator==(const Update&) const = default;
    };

    SoftwareUpdate();

    State state() const { return m_state; }
    const std::vector<Update>& updates() const { return m_updates; }
    // While updating.
    Phase phase() const { return m_phase; }
    // While updating: from 0 to 100, -1 while unknown.
    int progress() const { return m_progress; }
    // While updating: whether cancel() can still stop it.
    bool canCancel() const { return m_canCancel; }
    // In the Error state: the package manager's text, which may be empty.
    const std::string& error() const { return m_error; }

    void check();
    void update();
    void cancel();
    void restart();

    // Where the page looks for PackageKit: the system bus, unless a test
    // put its own.
    static cutefish::base::dbus::Connection* packageKitBus();
    static void setPackageKitBus(cutefish::base::dbus::Connection* bus);

    cutefish::base::Signal<> changed;

private:
    void reload();
    void apply(const cutefish::base::dbus::VariantMap& properties);

    cutefish::base::dbus::Proxy m_service;
    cutefish::base::dbus::NameWatch m_watch;
    cutefish::base::dbus::Match m_changedMatch;
    cutefish::base::dbus::PendingCall m_reload;
    State m_state = State::Unknown;
    std::vector<Update> m_updates;
    Phase m_phase = Phase::Downloading;
    int m_progress = -1;
    bool m_canCancel = false;
    std::string m_error;
};

} // namespace settings

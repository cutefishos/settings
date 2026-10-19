// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "base/signal.h"
#include "cutefish/networkmanager/network_model.h"

namespace settings::network {

// The networks the pages list: NetworkManager's, or the sample networks of
// --mock-network, which have no NetworkManager connection behind them.
class NetworkList
{
public:
    NetworkList();
    ~NetworkList();

    bool isMock() const { return !m_model; }
    const std::vector<cutefish::networkmanager::NetworkItem>& items() const;
    // One type of row, sorted for a list.
    std::vector<cutefish::networkmanager::NetworkItem> rows(cutefish::networkmanager::ConnectionType type) const;
    // The row whose key, connection path or SSID is the value.
    std::optional<cutefish::networkmanager::NetworkItem> find(const std::string& connectionPath,
                                                              const std::string& ssid = {}) const;

    cutefish::base::Signal<> changed;

private:
    std::unique_ptr<cutefish::networkmanager::NetworkModel> m_model;
    std::vector<cutefish::networkmanager::NetworkItem> m_mock;
    cutefish::base::Subscription m_subscription;
};

// The sample networks: a connected and a saved Wi-Fi network, two others,
// and two wired connections.
std::vector<cutefish::networkmanager::NetworkItem> mockNetworks();

} // namespace settings::network

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <memory>
#include <vector>

#include "base/signal.h"
#include "cutefish/bluez/bluetooth_object.h"

namespace cutefish::ui {
class DialogButton;
class DialogWindow;
class TextField;
class Window;
}

namespace settings {

class InfoRow;
class ListRow;
class SwitchRow;

// An adapter's name, address, power, visibility and pairing.
class AdapterDetailsDialog
{
public:
    AdapterDetailsDialog(cutefish::ui::Window* parent, std::shared_ptr<cutefish::bluez::BluetoothObject> adapter);
    ~AdapterDetailsDialog();

    void show();

private:
    void update();
    void rename();

    std::shared_ptr<cutefish::bluez::BluetoothObject> m_adapter;
    std::unique_ptr<cutefish::ui::DialogWindow> m_dialog;
    cutefish::ui::TextField* m_name;
    InfoRow* m_address;
    SwitchRow* m_powered;
    SwitchRow* m_discoverable;
    SwitchRow* m_pairable;
    cutefish::base::Subscription m_changed;
};

// A device's type, model, version and address, its trust and block
// switches, and forgetting it.
class DeviceDetailsDialog
{
public:
    DeviceDetailsDialog(cutefish::ui::Window* parent, std::shared_ptr<cutefish::bluez::BluetoothObject> device);
    ~DeviceDetailsDialog();

    void show();

private:
    void update();

    std::shared_ptr<cutefish::bluez::BluetoothObject> m_device;
    std::unique_ptr<cutefish::ui::DialogWindow> m_dialog;
    InfoRow* m_type;
    InfoRow* m_model;
    InfoRow* m_version;
    InfoRow* m_address;
    SwitchRow* m_trusted;
    SwitchRow* m_blocked;
    cutefish::ui::DialogButton* m_forget;
    // The Device Information service shows up once the device's services resolve.
    bool m_servicesResolved = false;
    cutefish::base::Subscription m_changed;
};

} // namespace settings

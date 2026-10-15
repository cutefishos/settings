// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/pagelist.h"

#include <map>

#include "window/page.h"

using namespace cutefish::ui;

namespace settings {

static std::vector<PageInfo> makePages()
{
    std::vector<PageInfo> pages;
    const auto add = [&pages](PageInfo page) { pages.push_back(std::move(page)); };

    // Sidebar pages, in sidebar order. Ethernet and Bluetooth join it once
    // their hardware is found, after Wi-Fi.
    add({.id = "wlan", .name = "wlan", .title = "Wi-Fi", .icon = "network-wireless-connected-100",
         .iconColor = Color::rgb(0x0067FF), .category = "Network",
         .keywords = "wireless, WLAN, network, internet, password, DNS, IP address"});
    add({.id = "ethernet", .name = "ethernet", .title = "Ethernet", .icon = "network.svg",
         .iconColor = Color::rgb(0x0067FF), .category = "Network",
         .keywords = "wired, LAN, network, internet, cable, IP address", .initiallyAvailable = false});
    add({.id = "bluetooth", .name = "bluetooth", .title = "Bluetooth", .icon = "bluetooth.svg",
         .iconColor = Color::rgb(0x0067FF), .category = "Network",
         .keywords = "wireless, devices, pairing, headphones", .initiallyAvailable = false});
    add({.id = "network", .name = "network", .title = "Network", .icon = "globe.svg",
         .iconColor = Color::rgb(0x0067FF), .category = "Network",
         .keywords = "VPN, proxy, hotspot, tethering, internet"});
    add({.id = "appearance", .name = "appearance", .title = "Appearance", .icon = "appearance.svg",
         .iconColor = Color::rgb(0x03B4CB), .category = "Desktop",
         .keywords = "theme, dark mode, light mode, accent color, dim wallpaper"});
    add({.id = "background", .name = "background", .title = "Wallpaper", .icon = "wallpaper.svg",
         .iconColor = Color::rgb(0x34B4A7), .category = "Desktop",
         .keywords = "background, desktop picture, image"});
    add({.id = "dock", .name = "dock", .title = "Dock", .icon = "dock.svg", .iconColor = Color::rgb(0x8585FC),
         .category = "Desktop", .keywords = "taskbar, position, size, auto hide, glass effect"});
    add({.id = "notifications", .name = "notifications", .title = "Notifications", .icon = "notifications.svg",
         .iconColor = Color::rgb(0xF16884), .category = "Desktop", .keywords = "do not disturb, banners, alerts"});
    add({.id = "display", .name = "display", .title = "Display", .icon = "display.svg",
         .iconColor = Color::rgb(0x0087ED), .category = "Devices",
         .keywords = "monitor, screen, resolution, refresh rate, scaling, brightness, rotation"});
    add({.id = "sound", .name = "sound", .title = "Sound", .icon = "sound.svg", .iconColor = Color::rgb(0xF16884),
         .category = "Devices",
         .keywords = "audio, volume, speakers, headphones, microphone, input, output"});
    add({.id = "keyboard", .name = "keyboard", .title = "Keyboard", .icon = "keyboard.svg",
         .iconColor = Color::rgb(0x5E6AD2), .category = "Devices",
         .keywords = "input method, key repeat, keyboard style"});
    add({.id = "mouse", .name = "mouse", .title = "Mouse", .icon = "cursor.svg", .iconColor = Color::rgb(0x3385FF),
         .category = "Devices", .keywords = "pointer speed, acceleration, natural scrolling, left-handed"});
    add({.id = "touchpad", .name = "touchpad", .title = "Touchpad", .icon = "touchpad.svg",
         .iconColor = Color::rgb(0x999999), .category = "Devices",
         .keywords = "trackpad, tap to click, natural scrolling, secondary click, pointer speed"});
    add({.id = "touchscreen", .name = "touchscreen", .title = "Touchscreen", .icon = "touchscreen.svg",
         .iconColor = Color::rgb(0x999999), .category = "Devices", .keywords = "touch, touch screen, touch panel"});
    add({.id = "accounts", .name = "accounts", .title = "Users", .icon = "accounts.svg",
         .iconColor = Color::rgb(0xDA7C43), .category = "System",
         .keywords = "accounts, password, login, administrator, avatar"});
    add({.id = "language", .name = "language", .title = "Language", .icon = "language.svg",
         .iconColor = Color::rgb(0x20A7FF), .category = "System", .keywords = "region, locale, system language"});
    add({.id = "datetime", .name = "datetime", .title = "Date & Time", .icon = "datetime.svg",
         .iconColor = Color::rgb(0x418CFF), .category = "System",
         .keywords = "clock, time zone, 24-hour time, NTP, sync"});
    add({.id = "defaultapps", .name = "defaultapps", .title = "Default Applications", .icon = "defaultapps.svg",
         .iconColor = Color::rgb(0x418CFF), .category = "System", .keywords = "web browser, email client, terminal"});
    add({.id = "battery", .name = "battery", .title = "Battery", .icon = "battery.svg",
         .iconColor = Color::rgb(0x2EC347), .category = "System",
         .keywords = "charge, charging limit, battery health, capacity, history, percentage"});
    add({.id = "power", .name = "power", .title = "Power", .icon = "power.svg", .iconColor = Color::rgb(0xFE8433),
         .category = "System", .keywords = "energy, performance, power saving, sleep, turn off display"});
    add({.id = "about", .name = "about", .title = "About", .icon = "about.svg", .iconColor = Color::rgb(0x24A7FD),
         .category = "System", .keywords = "system, version, kernel, processor, memory, storage, hardware"});

    // Sub pages, opened from inside a sidebar page; search lists them after it.
    add({.id = "vpn", .name = "vpn", .title = "VPN", .icon = "network-vpn", .iconColor = Color::rgb(0x48484C),
         .keywords = "VPN, WireGuard, OpenVPN, tunnel, network", .parentId = "network"});
    add({.id = "proxy", .name = "proxy", .title = "Proxy", .icon = "proxy.svg", .iconColor = Color::rgb(0x0067FF),
         .keywords = "HTTP, HTTPS, SOCKS, FTP, PAC, network", .parentId = "network"});
    add({.id = "hotspot", .name = "hotspot", .title = "Hotspot", .icon = "network-wireless-hotspot",
         .iconColor = Color::rgb(0x2EBC6B), .keywords = "tethering, share internet, access point, Wi-Fi, wireless",
         .parentId = "network"});
    add({.id = "fonts", .title = "Fonts", .icon = "fonts.svg", .iconColor = Color::rgb(0xFE8433),
         .keywords = "general font, fixed font, font size, anti-aliasing, hinting", .parentId = "appearance"});
    add({.id = "icontheme", .title = "Icon Theme", .icon = "icontheme.svg", .iconColor = Color::rgb(0x2EC347),
         .keywords = "icons", .parentId = "appearance"});
    add({.id = "cursortheme", .title = "Cursor Style", .icon = "cursor.svg", .iconColor = Color::rgb(0x3385FF),
         .keywords = "cursor theme, cursor size, pointer", .parentId = "appearance"});
    add({.id = "keyboardlayout", .title = "Keyboard Layout", .icon = "keyboard.svg",
         .iconColor = Color::rgb(0x5E6AD2), .keywords = "input source", .parentId = "keyboard"});
    add({.id = "hotcorners", .name = "hotcorners", .title = "Hot Corners", .icon = "dock.svg",
         .iconColor = Color::rgb(0x8585FC), .keywords = "screen corners, show desktop, show all windows",
         .parentId = "dock"});
    add({.id = "shortcuts", .title = "Keyboard Shortcuts", .icon = "keyboard.svg", .iconColor = Color::rgb(0x5E6AD2),
         .keywords = "hotkeys, key bindings", .parentId = "keyboard"});
    return pages;
}

struct Registration
{
    PageFactory create;
    AvailabilityFactory watchAvailability;
};

// Filled before main() by the pages' static registrations.
static std::map<std::string, Registration>& registrations()
{
    static std::map<std::string, Registration> map;
    return map;
}

PageRegistration::PageRegistration(const char* id, PageFactory create, AvailabilityFactory watchAvailability)
{
    registrations()[id] = {std::move(create), std::move(watchAvailability)};
}

const std::vector<PageInfo>& pages()
{
    static const std::vector<PageInfo> list = [] {
        std::vector<PageInfo> pages = makePages();
        for (PageInfo& page : pages) {
            const auto it = registrations().find(page.id);
            if (it == registrations().end())
                continue;
            page.create = it->second.create;
            page.watchAvailability = it->second.watchAvailability;
        }
        return pages;
    }();
    return list;
}

const PageInfo* findPage(const std::string& id)
{
    for (const PageInfo& page : pages()) {
        if (page.id == id)
            return &page;
    }
    return nullptr;
}

} // namespace settings

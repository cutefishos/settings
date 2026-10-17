// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>

#include "ui/color_provider.h"

namespace settings {

// Settings' own colours, after the toolkit's; colors.cpp gives them for
// each theme. Retune the look there rather than at the use sites.
enum class SettingsColor : uint16_t {
    SidebarBackground = uint16_t(cutefish::ui::ColorId::ApplicationFirst),
    SidebarItemCurrent,

    RowHighlight,
    RowComboBoxHovered,
    RowComboBoxOpen,
    StandardButtonBackground,
    KeyCap,
    PreferredLanguageTag,
    WallpaperPlaceholder,

    TimeZoneMapPill,

    BatteryChipBackground,
    BatteryChipBorder,
    GraphGrid,
    GraphBaseline,
    GraphProbe,
};

void addSettingsColors(cutefish::ui::ColorMixer& mixer, const cutefish::ui::Theme& theme);

// Added before main by every program that names one of the colours.
[[maybe_unused]] inline const bool settingsColorsAdded =
    (cutefish::ui::ColorProvider::addMixer(addSettingsColors), true);

} // namespace settings

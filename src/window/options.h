// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

namespace settings {

// The command line's switches that pages read.
struct Options
{
    // Sample network data, for screenshots and tests.
    bool mockNetwork = false;
    // Sample battery data.
    bool mockBattery = false;
    // A second sample battery, with mockBattery.
    bool mockExtraBattery = false;
};

const Options& options();
void setOptions(const Options& options);

} // namespace settings

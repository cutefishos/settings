// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>

namespace settings {

// What the About page tells about the system.
namespace about {

// The os-release PRETTY_NAME.
std::string prettyProductName();
// As Qt names it: x86_64, arm64, i386.
std::string architecture();
std::string kernelVersion();
// "<model> x <count>" from /proc/cpuinfo.
std::string processor();
std::string memorySize();
// "<used> / <total>" of the root file system.
std::string internalStorage();

// With binary units, B through YB, whole numbers.
std::string formatByteSize(double size);

} // namespace about

} // namespace settings

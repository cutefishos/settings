// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pages/about/aboutinfo.h"

#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

namespace settings::about {

static std::string unquoted(std::string value)
{
    if (value.size() >= 2 && (value.front() == '"' || value.front() == '\'') && value.back() == value.front())
        value = value.substr(1, value.size() - 2);
    std::string result;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '\\' && i + 1 < value.size())
            ++i;
        result += value[i];
    }
    return result;
}

static std::string osRelease(const std::string& key)
{
    for (const char* path : {"/etc/os-release", "/usr/lib/os-release"}) {
        std::ifstream file(path);
        if (!file)
            continue;
        std::string line;
        while (std::getline(file, line)) {
            if (line.starts_with(key + "="))
                return unquoted(line.substr(key.size() + 1));
        }
        return {};
    }
    return {};
}

std::string prettyProductName()
{
    std::string name = osRelease("PRETTY_NAME");
    if (!name.empty())
        return name;
    utsname system;
    if (uname(&system) == 0)
        return std::string(system.sysname) + " " + system.release;
    return {};
}

static std::string firmwareString(const char* path)
{
    std::ifstream file(path);
    if (!file)
        return {};
    std::stringstream stream;
    stream << file.rdbuf();
    std::string value = stream.str();
    // Device tree strings end in a NUL byte, DMI ones in a newline.
    const size_t end = value.find_last_not_of(std::string(" \t\n\0", 4));
    value.erase(end == std::string::npos ? 0 : end + 1);
    const size_t begin = value.find_first_not_of(" \t");
    return begin == std::string::npos ? std::string() : value.substr(begin);
}

static std::string lowercase(std::string value)
{
    for (char& c : value)
        c = std::tolower(static_cast<unsigned char>(c));
    return value;
}

// Firmware vendors leave these in fields they never filled in.
static bool isPlaceholder(const std::string& value)
{
    static const char* const placeholders[] = {
        "",
        "to be filled by o.e.m.",
        "system product name",
        "system manufacturer",
        "default string",
        "not applicable",
        "not specified",
        "o.e.m.",
        "none",
    };
    const std::string lowered = lowercase(value);
    for (const char* placeholder : placeholders) {
        if (lowered == placeholder)
            return true;
    }
    return false;
}

std::string computerModel(const std::string& vendor, const std::string& family, const std::string& product,
                          const std::string& deviceTreeModel)
{
    std::string model = !isPlaceholder(family) ? family : !isPlaceholder(product) ? product : std::string();
    if (model.empty())
        return isPlaceholder(deviceTreeModel) ? std::string() : deviceTreeModel;
    if (isPlaceholder(vendor))
        return model;

    const std::string vendorWord = lowercase(vendor.substr(0, vendor.find(' ')));
    const std::string loweredModel = lowercase(model);
    if (loweredModel.starts_with(vendorWord)
        && (loweredModel.size() == vendorWord.size() || loweredModel[vendorWord.size()] == ' '))
        return model;
    return vendor + " " + model;
}

std::string computerModel()
{
    return computerModel(firmwareString("/sys/class/dmi/id/sys_vendor"),
                         firmwareString("/sys/class/dmi/id/product_family"),
                         firmwareString("/sys/class/dmi/id/product_name"),
                         firmwareString("/sys/firmware/devicetree/base/model"));
}

std::string architecture()
{
    utsname system;
    if (uname(&system) != 0)
        return {};
    const std::string machine = system.machine;
    if (machine == "aarch64")
        return "arm64";
    if (machine == "i486" || machine == "i586" || machine == "i686")
        return "i386";
    return machine;
}

std::string kernelVersion()
{
    utsname system;
    return uname(&system) == 0 ? system.release : std::string();
}

std::string processor()
{
    std::ifstream file("/proc/cpuinfo");
    if (!file)
        return {};
    std::string model;
    bool found = false;
    int count = 0;
    std::string line;
    while (std::getline(file, line)) {
        if (!found && line.starts_with("model name")) {
            const size_t colon = line.find(':');
            if (colon != std::string::npos) {
                // The space after the colon stays, as it always did.
                const size_t end = line.find(':', colon + 1);
                model = line.substr(colon + 1, end == std::string::npos ? std::string::npos : end - colon - 1);
            }
            found = true;
        } else if (line.starts_with("processor")) {
            ++count;
        }
    }
    if (!found)
        return "Unknown";
    if (count > 0)
        model += " x " + std::to_string(count);
    return model;
}

std::string formatByteSize(double size)
{
    static const char* const units[] = {"B", "KB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    int unit = 0;
    while (std::abs(size) >= 1024 && unit < 8) {
        size /= 1024;
        ++unit;
    }
    char number[64];
    snprintf(number, sizeof(number), "%.0f", size);
    return std::string(number) + " " + units[unit];
}

std::string memorySize()
{
    struct sysinfo info;
    if (sysinfo(&info) != 0)
        return {};
    return formatByteSize(double(info.totalram) * info.mem_unit);
}

std::string internalStorage()
{
    struct statvfs root;
    if (statvfs("/", &root) != 0)
        return formatByteSize(0) + " / " + formatByteSize(0);
    const double total = double(root.f_blocks) * root.f_frsize;
    const double available = double(root.f_bavail) * root.f_frsize;
    return formatByteSize(total - available) + " / " + formatByteSize(total);
}

} // namespace settings::about

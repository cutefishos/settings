// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/fingerprint/fingers.h"

#include <algorithm>

#include "base/i18n.h"

using namespace cutefish;
using cutefish::fprint::Finger;
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

static base::dbus::Connection* s_bus = nullptr;

base::dbus::Connection* fingerprintBus()
{
    return s_bus ? s_bus : base::dbus::Connection::system();
}

void setFingerprintBus(base::dbus::Connection* bus)
{
    s_bus = bus;
}

const std::vector<Finger>& fingerOrder()
{
    static const std::vector<Finger> order = {
        Finger::RightIndex, Finger::LeftIndex, Finger::RightMiddle, Finger::LeftMiddle, Finger::RightThumb,
        Finger::LeftThumb, Finger::RightRing, Finger::LeftRing, Finger::RightLittle, Finger::LeftLittle,
    };
    return order;
}

std::vector<Finger> inFingerOrder(const std::vector<Finger>& enrolled)
{
    std::vector<Finger> fingers;
    for (Finger finger : fingerOrder()) {
        if (std::find(enrolled.begin(), enrolled.end(), finger) != enrolled.end())
            fingers.push_back(finger);
    }
    return fingers;
}

std::optional<Finger> freeFinger(const std::vector<Finger>& enrolled)
{
    for (Finger finger : fingerOrder()) {
        if (std::find(enrolled.begin(), enrolled.end(), finger) == enrolled.end())
            return finger;
    }
    return std::nullopt;
}

std::string fingerprintName(int number)
{
    return format(tr("Fingerprint {number}"), {{"number", std::to_string(number)}});
}

EnrollmentStep enrollmentStep(int stagesPassed, int stages)
{
    if (stages < 4)
        return EnrollmentStep::Place;
    return stagesPassed >= stages * 2 / 3 ? EnrollmentStep::AdjustGrip : EnrollmentStep::Place;
}

} // namespace settings

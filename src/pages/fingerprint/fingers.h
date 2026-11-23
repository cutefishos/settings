// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "base/dbus.h"
#include "cutefish/fprint/fingerprint_finger.h"

namespace settings {

// How many fingerprints the page lets the user keep; readers store more, but
// a short list is one to look after.
inline constexpr size_t kMaxFingerprints = 5;

// The system bus fprintd is on; tests point it at their own.
cutefish::base::dbus::Connection* fingerprintBus();
void setFingerprintBus(cutefish::base::dbus::Connection* bus);

// The fingers in the order new fingerprints take them: the index fingers
// first, then the middle ones, the thumbs, and the rest.
const std::vector<cutefish::fprint::Finger>& fingerOrder();

// The stored fingers in that order, which is the order of the page's list.
std::vector<cutefish::fprint::Finger> inFingerOrder(const std::vector<cutefish::fprint::Finger>& enrolled);

// The finger a new fingerprint is stored under: the first of fingerOrder()
// that has none yet. The page numbers its fingerprints, so which finger it
// is does not matter to the user.
std::optional<cutefish::fprint::Finger> freeFinger(const std::vector<cutefish::fprint::Finger>& enrolled);

// "Fingerprint 1" for the first row of the list.
std::string fingerprintName(int number);

// What the person is asked to do while a print is recorded.
enum class EnrollmentStep {
    // Place the finger flat, again and again.
    Place,
    // The last third: the edges of the finger.
    AdjustGrip,
};

// A reader that wants few scans has no use for a second step.
EnrollmentStep enrollmentStep(int stagesPassed, int stages);

} // namespace settings

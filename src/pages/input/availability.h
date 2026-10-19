// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <memory>

#include "window/pagelist.h"

namespace settings {

// Answers once, on the loop's next turn, as the Qt version's sidebar
// checked a device once when it was built and never again.
std::unique_ptr<AvailabilityWatch> answerOnce(bool available, AvailabilityCallback callback);

} // namespace settings

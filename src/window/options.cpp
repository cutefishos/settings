// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/options.h"

namespace settings {

static Options s_options;

const Options& options()
{
    return s_options;
}

void setOptions(const Options& options)
{
    s_options = options;
}

} // namespace settings

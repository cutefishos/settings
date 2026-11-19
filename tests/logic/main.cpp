// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "base/event_loop.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    // For the models' D-Bus connections and timers, as an application's.
    cutefish::base::EventLoop loop;
    return RUN_ALL_TESTS();
}

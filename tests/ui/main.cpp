// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include "ui/application.h"
#include "widgets/picture.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    cutefish::ui::Application application(argc, argv, cutefish::ui::Application::Platform::Headless);
    settings::setDataDirectory(SETTINGS_SOURCE_DIR);
    return RUN_ALL_TESTS();
}

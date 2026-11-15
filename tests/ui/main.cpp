// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <cstdlib>
#include <string>

#include "ui/application.h"
#include "widgets/picture.h"
#include "window/settingswindow.h"

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    cutefish::ui::Application application(argc, argv, cutefish::ui::Application::Platform::Headless);
    settings::setDataDirectory(SETTINGS_SOURCE_DIR);
    // The user's saved sidebar width would move every page.
    char scratch[] = "/tmp/settings-ui-tests-XXXXXX";
    if (mkdtemp(scratch))
        settings::SettingsWindow::setConfigPath(std::string(scratch) + "/settings.conf");
    return RUN_ALL_TESTS();
}

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "application.h"
#include <QDebug>
#include <QIcon>

int main(int argc, char *argv[])
{
    Application app(argc, argv);

    app.setWindowIcon(QIcon::fromTheme("cutefish-settings"));

    return 0;
}

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "widgets/picture.h"

#include "ui/painter.h"
#include "widgets/card.h"

using namespace cutefish::ui;

namespace settings {

static std::string &dataDirectory()
{
    static std::string directory = SETTINGS_DATA_DIR;
    return directory;
}

std::string dataPath(const std::string& name)
{
    return dataDirectory() + "/" + name;
}

void setDataDirectory(std::string directory)
{
    dataDirectory() = std::move(directory);
}

Picture::Picture(std::string path)
    : m_path(std::move(path))
{
}

void Picture::setPath(std::string path)
{
    m_path = std::move(path);
    requestPaint();
}

void Picture::setTint(std::optional<Color> tint)
{
    m_tint = tint;
    requestPaint();
}

void Picture::setCornerRadius(double radius)
{
    m_radius = radius;
    requestPaint();
}

// The toolkit fits the picture centred with its shape kept, as Qt's SVG
// image reader does at another aspect.
void Picture::paint(Painter& painter)
{
    if (m_path.empty())
        return;
    const RectF bounds = localRect();
    painter.save();
    if (m_radius > 0)
        painter.clipRoundedRect(bounds, m_radius, themeCorners());
    painter.drawSvg(m_path, bounds, m_tint);
    painter.restore();
}

} // namespace settings

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <optional>
#include <string>

#include "ui/color_mixer.h"
#include "ui/view.h"

namespace settings {

// The path of one of settings' installed files, such as "images/logo.svg".
std::string dataPath(const std::string& name);
// Where dataPath() looks; tests point it at the source tree.
void setDataDirectory(std::string directory);

// One of settings' own pictures, drawn to fill the view, optionally tinted
// and clipped to rounded corners.
class Picture : public cutefish::ui::View
{
public:
    explicit Picture(std::string path = {});

    void setPath(std::string path);
    void setTint(std::optional<cutefish::ui::ColorRecipe> tint);
    void setCornerRadius(double radius);

protected:
    void paint(cutefish::ui::Painter& painter) override;

private:
    std::string m_path;
    std::optional<cutefish::ui::ColorRecipe> m_tint;
    double m_radius = 0;
};

} // namespace settings

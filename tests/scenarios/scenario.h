// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <gtest/gtest.h>

#include <chrono>
#include <memory>
#include <string>

#include "ui/keys.h"
#include "ui/test_driver.h"
#include "ui/window.h"
#include "window/settingswindow.h"

// The failure handler of every driver of the scenarios.
inline void scenarioFailure(const std::string& message)
{
    ADD_FAILURE() << message;
}

// A headless settings window at 150%, on a page opened by name, worked only
// through the driver.
class Scenario
{
public:
    explicit Scenario(const std::string& page = {}, const std::string& subPage = {})
        : m_window(std::make_unique<settings::SettingsWindow>())
    {
        m_window->window().setScale(1.5);
        m_window->window().resize({900, 820});
        if (!page.empty())
            m_window->switchToPage(page);
        if (!subPage.empty())
            m_window->pushPage(subPage);
        m_window->reveal();
        driver.layout();
    }

    settings::SettingsWindow& settingsWindow() { return *m_window; }

    // A driver of the window open over the settings window, such as a
    // dialog, which has the keyboard focus; null while none is open.
    std::unique_ptr<cutefish::ui::TestDriver> dialog()
    {
        cutefish::ui::Window* window = driver.dialog();
        if (!window) {
            ADD_FAILURE() << "no window open over the settings window";
            return nullptr;
        }
        auto dialog = std::make_unique<cutefish::ui::TestDriver>(*window, scenarioFailure);
        dialog->setKeyboardFocus(true);
        return dialog;
    }

private:
    std::unique_ptr<settings::SettingsWindow> m_window;

public:
    cutefish::ui::TestDriver driver{m_window->window(), scenarioFailure};
};

// A click into the field, then its text selected and typed over.
inline void replaceText(cutefish::ui::TestDriver& driver, const std::string& name, const std::string& text)
{
    driver.click(name);
    driver.typeKey(cutefish::ui::key::letter('a'), cutefish::ui::ControlModifier);
    driver.typeKey(cutefish::ui::key::BackSpace);
    driver.typeText(text);
}

// Turns the wheel over the view named `area` until the view named `name`
// shows whole inside it, as the user scrolls a list or a page to it.
inline bool scrollTo(cutefish::ui::TestDriver& driver, const std::string& area, const std::string& name)
{
    const auto inside = [&] {
        const cutefish::ui::RectF frame = driver.frame(area);
        const cutefish::ui::RectF item = driver.frame(name);
        return item.y >= frame.y && item.bottom() <= frame.bottom();
    };
    for (int turn = 0; turn < 40 && !inside(); ++turn) {
        const bool below = driver.frame(name).y > driver.frame(area).y;
        cutefish::ui::ScrollEvent wheel;
        wheel.deltaY = below ? 120 : -120;
        wheel.value120Y = below ? 120 : -120;
        driver.scroll(area, wheel);
        cutefish::ui::TestDriver::waitFor(inside, std::chrono::milliseconds(100));
    }
    if (!inside())
        ADD_FAILURE() << "\"" << name << "\" does not scroll into \"" << area << "\"";
    return inside();
}

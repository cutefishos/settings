// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <memory>
#include <string>

#include "widgets/listrow.h"

namespace cutefish::base {
class Timer;
}

namespace cutefish::ui {
class Slider;
}

namespace settings {

class SliderControls;

// A slider whose visible track, not its handle's travel, spans its place in a
// row, as on the Sound page: the track ends where switches and combo boxes
// do, and the handle overhangs those ends.
class RowSlider : public cutefish::ui::View
{
public:
    RowSlider();

    cutefish::ui::Slider* slider() const { return m_slider; }

    cutefish::ui::SizeF sizeHint() const override;

protected:
    void layout() override;

private:
    cutefish::ui::Slider* m_slider;
};

// A labelled slider row for a list card, with optional end icons, captions
// under the track's ends, or a level meter in place of the slider.
class SliderRow : public ListRow
{
public:
    explicit SliderRow(std::string text = {});
    ~SliderRow() override;

    void setRange(double from, double to);
    // Snaps to the step on release, as FishUI's slider does by default.
    void setStepSize(double step);
    // Ticks under the track, a taller slider, and snapping while dragged.
    void setStepped(bool stepped);
    // The value the setting has. A drag in progress, and the 300 ms after the
    // last move while the change goes out, keep the handle where the user put it.
    void setValue(double value);
    double value() const;
    void setSliderEnabled(bool enabled);

    // Captions flush with the two ends of the track, each under a tick.
    void setCaptions(std::string left, std::string right);
    // FishUI glyphs at the two ends, inside the same span as a bare slider.
    void setIcons(std::string left, std::string right);
    // 0 to 1 shows a level meter in place of the slider, on the same track;
    // negative shows the slider.
    void setMeterLevel(double level);

    cutefish::ui::Slider* slider() const;
    // The handle is held.
    bool isPressed() const;

    std::function<void(double value)> onMoved;
    std::function<void()> onPressedChanged;

protected:
    void layout() override;

private:
    SliderControls* m_controls;
    std::unique_ptr<cutefish::base::Timer> m_settle;
    bool m_settling = false;
    double m_value = 0;
};

} // namespace settings

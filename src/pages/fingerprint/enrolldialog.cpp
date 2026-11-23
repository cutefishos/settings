// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/fingerprint/enrolldialog.h"

#include <chrono>

#include "base/i18n.h"
#include "pages/fingerprint/fingerprintview.h"
#include "pages/fingerprint/fingers.h"
#include "ui/animation.h"
#include "ui/cell_layout.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "widgets/metrics.h"

using namespace cutefish::ui;
using cutefish::fprint::Enrollment;
using cutefish::fprint::Failure;
using cutefish::fprint::ScanType;
using cutefish::i18n::tr;

namespace settings {

static constexpr double kWidth = 360;
// The text under the print changes length; a fixed height keeps the dialog
// and its buttons from jumping.
// Keeps the lines of the words short enough to break evenly.
static constexpr double kTextInset = 20;
static constexpr double kHeight = 400;
// The title and two lines of guidance.
static constexpr double kWordsHeight = 76;
static constexpr double kButtonHeight = 32;

namespace {

struct Message
{
    std::string title;
    std::string detail;
    // Whether the detail is a correction of the last scan, in full contrast.
    bool urgent = false;
};

std::string hintText(Enrollment::Hint hint)
{
    switch (hint) {
    case Enrollment::Hint::RetryScan:
        return tr("The sensor could not read your finger. Try again.");
    case Enrollment::Hint::SwipeTooShort:
        return tr("The swipe was too short. Try again.");
    case Enrollment::Hint::TooFast:
        return tr("That was too fast. Try again, a little slower.");
    case Enrollment::Hint::NotCentered:
        return tr("Center your finger on the sensor and try again.");
    case Enrollment::Hint::RemoveAndRetry:
        return tr("Lift your finger, then place it on the sensor again.");
    case Enrollment::Hint::None:
        break;
    }
    return {};
}

Message failureMessage(Failure failure)
{
    switch (failure) {
    case Failure::InUse:
        return {tr("The sensor is busy"), tr("Another program is using the fingerprint sensor. Close it and try again.")};
    case Failure::Duplicate:
        return {tr("This fingerprint is already added"),
                tr("Try a different finger, or delete the fingerprint that is already stored.")};
    case Failure::Full:
        return {tr("The sensor is full"),
                tr("The fingerprint sensor has no room for another fingerprint. Delete one and try again.")};
    case Failure::Disconnected:
        return {tr("The sensor was disconnected"), tr("Connect the fingerprint sensor again and try once more.")};
    default:
        break;
    }
    return {tr("Couldn’t add the fingerprint"), tr("Something went wrong while reading your finger. Try again.")};
}

Message scanningMessage(const Enrollment& enrollment, ScanType scanType)
{
    if (const std::string hint = hintText(enrollment.hint()); !hint.empty())
        return {scanType == ScanType::Swipe ? tr("Swipe your finger across the sensor")
                                           : tr("Place your finger on the sensor"),
                hint, true};
    if (scanType == ScanType::Swipe) {
        return {tr("Swipe your finger across the sensor"), tr("Repeat until the fingerprint is complete.")};
    }
    if (enrollmentStep(enrollment.stagesPassed(), enrollment.stages()) == EnrollmentStep::AdjustGrip) {
        return {tr("Adjust your grip"),
                tr("Keep lifting and resting your finger, changing its position a little each time.")};
    }
    return {tr("Place your finger on the sensor"),
            tr("Lift and rest your finger repeatedly.")};
}

} // namespace

EnrollDialog::EnrollDialog(SettingsWindow& parent, cutefish::fprint::Reader& reader,
                           std::unique_ptr<Enrollment> enrollment)
    : FormDialog(parent, tr("Add Fingerprint"), kWidth)
    , m_reader(reader)
    , m_enrollment(std::move(enrollment))
{
    setFixedHeight(kHeight);
    ColumnLayout* column = content();

    // Every state has the same parts in the same places: the print and its
    // words in a block that always takes the room of two lines of guidance,
    // centred a little above the middle, and a row for the button that is
    // kept even while there is none, so a Try Again that turns up moves
    // nothing.
    column->addChild<View>()->setLayoutHints({.fillHeight = true, .stretch = 1});

    m_print = column->addChild<FingerprintView>();
    m_print->setName("enroll:print");
    m_print->setLayoutHints({.horizontalAlignment = Alignment::Center});

    ColumnLayout* words = column->addChild<ColumnLayout>();
    words->setSpacing(metrics::s);
    words->setLayoutHints({.fillWidth = true, .minimumHeight = kWordsHeight, .margins = {0, metrics::s, 0, 0}});

    m_title = words->addChild<Label>();
    m_title->setName("enroll:title");
    m_title->setFontSize(metrics::points(12));
    m_title->setWeight(600);
    m_title->setWrap(true);
    m_title->setTextAlignment(Label::TextAlignment::Center);
    m_title->setLayoutHints({.fillWidth = true, .margins = {kTextInset, 0, kTextInset, 0}});

    m_detail = words->addChild<Label>();
    m_detail->setName("enroll:detail");
    m_detail->setWrap(true);
    m_detail->setTextAlignment(Label::TextAlignment::Center);
    m_detail->setLayoutHints({.fillWidth = true, .margins = {kTextInset, 0, kTextInset, 0}});
    // Holds the words at the top of their block, whatever their lines.
    words->addChild<View>()->setLayoutHints({.fillHeight = true});

    column->addChild<View>()->setLayoutHints({.fillHeight = true, .stretch = 1.3});

    m_buttons = addButtonRow();
    m_buttons->setLayoutHints({.fillWidth = true, .minimumHeight = kButtonHeight});
    m_action = m_buttons->addChild<DialogButton>(tr("Done"), true);
    m_action->setLayoutHints({.fillWidth = true});
    m_action->setName("enroll:action");
    m_action->onClicked = [this] {
        if (m_enrollment->state() == Enrollment::State::Completed)
            close();
        else
            tryAgain();
    };

    canClose = [this] {
        if (m_enrollment)
            m_enrollment->cancel();
        return true;
    };
    watch();
    update();
}

EnrollDialog::~EnrollDialog() = default;

void EnrollDialog::watch()
{
    m_changed = m_enrollment->changed.connect([this] { update(); });
}

void EnrollDialog::tryAgain()
{
    const auto finger = freeFinger(m_reader.enrolled());
    std::unique_ptr<Enrollment> enrollment = finger ? m_reader.enroll(*finger) : nullptr;
    if (!enrollment) {
        close();
        return;
    }
    m_changed.reset();
    m_enrollment = std::move(enrollment);
    watch();
    update();
}

// The button fades in where its room was kept.
void EnrollDialog::showAction(bool show)
{
    if (show == m_action->isVisible())
        return;
    m_action->setVisible(show);
    if (!show)
        return;
    m_action->setOpacity(0);
    if (!m_fade)
        m_fade = std::make_unique<Animation>(window());
    m_fade->setDuration(std::chrono::milliseconds(220));
    m_fade->setEasing(Animation::Easing::OutCubic);
    m_fade->onUpdate = [this](double value) { m_action->setOpacity(value); };
    m_fade->start();
}

void EnrollDialog::update()
{
    if (!isOpen())
        return;
    const Enrollment& enrollment = *m_enrollment;
    const ScanType scanType = m_reader.scanType();
    Message message;
    bool finished = false;
    bool retryable = false;
    switch (enrollment.state()) {
    case Enrollment::State::Starting:
    case Enrollment::State::Scanning:
        message = scanningMessage(enrollment, scanType);
        break;
    case Enrollment::State::Completed:
        message = {tr("Fingerprint added"), tr("Your fingerprint is ready to use.")};
        finished = true;
        break;
    case Enrollment::State::Failed:
        // Dismissing the password prompt is an answer, not a problem.
        if (enrollment.failure() == Failure::NotAuthorized) {
            close();
            return;
        }
        message = failureMessage(enrollment.failure());
        finished = true;
        retryable = enrollment.failure() != Failure::Full && enrollment.failure() != Failure::Disconnected;
        break;
    case Enrollment::State::Cancelled:
        return;
    }

    const bool completed = enrollment.state() == Enrollment::State::Completed;
    const bool failed = finished && !completed;
    m_print->setComplete(completed);
    m_print->setProgress(enrollment.stages() > 0 ? double(enrollment.stagesPassed()) / enrollment.stages() : 0);
    m_title->setText(message.title);
    m_detail->setText(message.detail);
    m_detail->setColor(message.urgent ? ColorRecipe(ColorId::Text) : ColorRecipe(ColorId::DisabledText));

    const bool action = completed || (failed && retryable);
    m_action->setText(completed ? tr("Done") : tr("Try Again"));
    showAction(action);
}

} // namespace settings

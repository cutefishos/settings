// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <chrono>
#include <memory>
#include <vector>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/signal.h"
#include "base/timer.h"
#include "cutefish/fprint/fingerprint_reader.h"
#include "pages/fingerprint/enrolldialog.h"
#include "pages/fingerprint/fingers.h"
#include "ui/button.h"
#include "ui/cell_layout.h"
#include "ui/dialog_window.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowicon.h"
#include "widgets/sections.h"
#include "widgets/standardbutton.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::fprint::Enrollment;
using cutefish::fprint::Failure;
using cutefish::fprint::Finger;
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

namespace {

// A stored fingerprint: its number and a Delete button.
class FingerprintRow : public ListRow
{
public:
    FingerprintRow(const std::string& name, std::function<void()> remove)
        : ListRow(name)
    {
        setInteractive(false);
        setLeading<RowIcon>("fingerprint.svg", accent::red);
        m_delete = addContent<Button>(tr("Delete"));
        m_delete->onClicked = std::move(remove);
    }

    Button* deleteButton() const { return m_delete; }

private:
    Button* m_delete;
};

class FingerprintPage : public Page
{
public:
    explicit FingerprintPage(SettingsWindow& window)
        : Page(window, tr("Fingerprint"))
        , m_reader(fingerprintBus())
    {
        m_empty = content()->addChild<Card>();
        m_empty->setName("fingerprint:empty");
        m_empty->setLayoutHints({.fillWidth = true});
        m_emptyText = m_empty->addChild<Label>(tr("No fingerprints added"));
        m_emptyText->setTextAlignment(Label::TextAlignment::Center);
        m_emptyText->setLayoutHints({.fillWidth = true});

        m_list = content()->addChild<Card>(Card::Style::List);
        m_list->setName("fingerprint:list");
        m_list->setLayoutHints({.fillWidth = true});

        m_add = content()->addChild<StandardButton>(tr("Add Fingerprint…"));
        m_add->setName("fingerprint:add");
        m_add->onClicked = [this] { startEnrollment(); };

        m_message = content()->addChild<Footnote>();
        m_message->setName("fingerprint:message");
        m_message->setColor(accent::red);

        m_enrolled = m_reader.enrolledChanged.connect([this] { update(); });
        m_ready = m_reader.readyChanged.connect([this] { update(); });
        update();
    }

protected:
    void layout() override
    {
        m_emptyText->setColor(Theme::current().disabledText);
        Page::layout();
    }

private:
    // Lets what is handling an event finish before it goes.
    template<typename T>
    static void dispose(std::unique_ptr<T>& object)
    {
        std::shared_ptr<T> closing(object.release());
        base::EventLoop::current()->post([closing] {});
    }

    bool isBusy() const { return m_pending || m_enroll || m_confirm || m_removing; }

    void update()
    {
        const bool listed = m_reader.isListed();
        const std::string& error = m_reader.listError();
        const std::vector<Finger> fingers = inFingerOrder(m_reader.enrolled());
        const bool usable = listed && error.empty();

        m_empty->setVisible(usable && fingers.empty());
        m_list->setVisible(usable && !fingers.empty());
        m_add->setVisible(usable && fingers.size() < kMaxFingerprints);
        m_add->setEnabled(!isBusy());
        const std::string message = listed && !error.empty()
                ? std::string(tr("The fingerprints cannot be read. Manage them from your own desktop session."))
                : m_problem;
        m_message->setText(message);
        m_message->setVisible(!message.empty());

        // The rows go only once the list changed, after a delete returned.
        for (FingerprintRow* row : m_rows)
            m_list->takeChild(row);
        m_rows.clear();
        for (size_t i = 0; i < fingers.size(); ++i) {
            const Finger finger = fingers[i];
            const int number = int(i) + 1;
            FingerprintRow* row = m_list->addChild<FingerprintRow>(
                    fingerprintName(number), [this, finger, number] { confirmDelete(finger, number); });
            row->setName("fingerprint:row:" + std::to_string(number));
            row->deleteButton()->setName("fingerprint:delete:" + std::to_string(number));
            row->deleteButton()->setEnabled(!isBusy());
            m_rows.push_back(row);
        }
    }

    void startEnrollment()
    {
        const std::vector<Finger>& enrolled = m_reader.enrolled();
        const auto finger = freeFinger(enrolled);
        if (isBusy() || !finger || enrolled.size() >= kMaxFingerprints)
            return;
        m_problem.clear();
        m_pending = m_reader.enroll(*finger);
        if (!m_pending)
            return;
        m_pendingChanged = m_pending->changed.connect([this] { pendingChanged(); });
        update();
    }

    // The password prompt is over, one way or another.
    void pendingChanged()
    {
        if (m_pending->state() == Enrollment::State::Starting)
            return;
        m_pendingChanged.reset();
        const bool refused = m_pending->state() == Enrollment::State::Failed
                && m_pending->failure() == Failure::NotAuthorized;
        if (refused) {
            dispose(m_pending);
            update();
            return;
        }
        m_enroll = std::make_unique<EnrollDialog>(settingsWindow(), m_reader, std::move(m_pending));
        m_enroll->onClosed = [this] {
            dispose(m_enroll);
            update();
        };
        m_enroll->show();
        update();
    }

    void confirmDelete(Finger finger, int number)
    {
        if (isBusy())
            return;
        m_confirm = std::make_unique<ConfirmDialog>(
                &settingsWindow().window(), tr("Delete this fingerprint"),
                format(tr("Delete “{name}”? You will no longer be able to use it to authenticate."),
                       {{"name", fingerprintName(number)}}));
        m_confirm->setCancelText(tr("Cancel"));
        m_confirm->setConfirmText(tr("Delete"));
        m_confirm->setDestructive(true);
        m_confirm->onAccepted = [this, finger] {
            dispose(m_confirm);
            remove(finger);
        };
        m_confirm->onRejected = [this] {
            dispose(m_confirm);
            update();
        };
        m_confirm->show();
        update();
    }

    void remove(Finger finger)
    {
        m_removing = true;
        m_problem.clear();
        update();
        m_reader.remove(finger, [this](Failure failure, const std::string&) {
            m_removing = false;
            if (failure == Failure::InUse)
                m_problem = tr("The sensor is being used by another program. Try again in a moment.");
            else if (failure != Failure::None && failure != Failure::NotAuthorized)
                m_problem = tr("The fingerprint could not be deleted.");
            update();
        });
    }

    fprint::Reader m_reader;
    base::Subscription m_enrolled;
    base::Subscription m_ready;
    base::Subscription m_pendingChanged;
    Card* m_empty;
    Label* m_emptyText;
    Card* m_list;
    StandardButton* m_add;
    Footnote* m_message;
    std::vector<FingerprintRow*> m_rows;
    std::string m_problem;
    std::unique_ptr<Enrollment> m_pending;
    std::unique_ptr<EnrollDialog> m_enroll;
    std::unique_ptr<ConfirmDialog> m_confirm;
    bool m_removing = false;
};

// The page is there while fprintd has a reader; until it has answered, there
// is none either way.
class FingerprintWatch : public AvailabilityWatch
{
public:
    explicit FingerprintWatch(AvailabilityCallback callback)
        : m_callback(std::move(callback))
        , m_reader(fingerprintBus())
    {
        if (m_reader.isReady()) {
            answerLater();
            return;
        }
        m_ready = m_reader.readyChanged.connect([this] { m_callback(m_reader.isAvailable()); });
    }

private:
    // Not from inside the watch's creation, which the window is still busy with.
    void answerLater()
    {
        m_answer.setSingleShot(true);
        m_answer.onTimeout = [this] { m_callback(m_reader.isAvailable()); };
        m_answer.start(std::chrono::milliseconds(0));
    }

    AvailabilityCallback m_callback;
    fprint::Reader m_reader;
    base::Subscription m_ready;
    base::Timer m_answer;
};

} // namespace

static const PageRegistration registration(
        "fingerprint",
        [](SettingsWindow& window) -> std::unique_ptr<Page> { return std::make_unique<FingerprintPage>(window); },
        [](SettingsWindow&, AvailabilityCallback callback) -> std::unique_ptr<AvailabilityWatch> {
            return std::make_unique<FingerprintWatch>(std::move(callback));
        });

} // namespace settings

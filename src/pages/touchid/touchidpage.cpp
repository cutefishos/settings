// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>
#include <list>
#include <memory>
#include <pwd.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "base/dbus.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "base/settings.h"
#include "ui/dialog_window.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/progress_bar.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/formdialog.h"
#include "widgets/iconbutton.h"
#include "widgets/listrow.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;
using cutefish::base::dbus::ObjectPath;
using cutefish::i18n::tr;
using cutefish::i18n::trNoop;

namespace settings {

namespace {

// Drawn tinted with the theme's text colour like the toolkit's own glyphs, so
// they read in both themes; a white SVG asset would vanish on a light card.
const bool touchIdGlyphs = [] {
    Icon::registerGlyph(
            "touchid-pencil",
            R"(<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"><g class="ColorScheme-Text" fill="#ffffff"><path d="M11 1.9a1.4 1.4 0 0 1 2 0l1.1 1.1a1.4 1.4 0 0 1 0 2l-.9.9-3.1-3.1.9-.9ZM9.4 3.5l3.1 3.1L6 13.1l-3.6.9a.5.5 0 0 1-.6-.6l.9-3.6 6.7-6.3Z"/></g></svg>)");
    Icon::registerGlyph(
            "touchid-trash",
            R"(<svg width="16" height="16" viewBox="0 0 16 16" xmlns="http://www.w3.org/2000/svg"><g class="ColorScheme-Text" fill="#ffffff"><rect x="2.5" y="3.9" width="11" height="1.4" rx="0.7"/><path d="M6.3 2.4h3.4v1.3H6.3z"/><path fill-rule="evenodd" d="M4 6.1h8l-.62 7.2a1 1 0 0 1-1 .9H5.62a1 1 0 0 1-1-.9L4 6.1Zm2.5 1.6v4.7h1V7.7h-1Zm2 0v4.7h1V7.7h-1Z"/></g></svg>)");
    return true;
}();

} // namespace

namespace {

constexpr const char* kService = "net.reactivated.Fprint";
constexpr const char* kManagerPath = "/net/reactivated/Fprint/Manager";
constexpr const char* kManagerInterface = "net.reactivated.Fprint.Manager";
constexpr const char* kDeviceInterface = "net.reactivated.Fprint.Device";
constexpr double kDialogWidth = 400;
constexpr double kFieldWidth = 200;
// Most readers ask for five to ten touches; the bar reaches nine tenths on the
// fifth and the last one completes it.
constexpr double kStagesShown = 5;

// The order macOS enrols fingers in, so the first Add takes the most useful.
constexpr const char* kFingerOrder[] = {
        "right-index-finger",  "right-thumb",        "left-thumb",        "left-index-finger",
        "right-middle-finger", "left-middle-finger", "right-ring-finger", "left-ring-finger",
        "right-little-finger", "left-little-finger",
};

struct FingerName
{
    const char* id;
    const char* label;
};

constexpr FingerName kFingerNames[] = {
        {"right-index-finger", trNoop("Right index finger")},
        {"left-index-finger", trNoop("Left index finger")},
        {"right-thumb", trNoop("Right thumb")},
        {"left-thumb", trNoop("Left thumb")},
        {"right-middle-finger", trNoop("Right middle finger")},
        {"left-middle-finger", trNoop("Left middle finger")},
        {"right-ring-finger", trNoop("Right ring finger")},
        {"left-ring-finger", trNoop("Left ring finger")},
        {"right-little-finger", trNoop("Right little finger")},
        {"left-little-finger", trNoop("Left little finger")},
};

std::string currentUserName()
{
    const passwd* user = getpwuid(getuid());
    return user && user->pw_name ? user->pw_name : std::string();
}

std::string standardLabel(const std::string& finger)
{
    for (const FingerName& name : kFingerNames) {
        if (finger == name.id)
            return tr(name.label);
    }
    return finger;
}

bool isEnrolled(const std::vector<std::string>& fingers, const std::string& finger)
{
    return std::find(fingers.begin(), fingers.end(), finger) != fingers.end();
}

// fprintd reports one line per touch while a finger is being enrolled.
std::string enrollMessage(const std::string& result)
{
    if (result == "enroll-stage-passed")
        return tr("Lift the finger and place it again");
    if (result == "enroll-retry-scan" || result == "enroll-remove-and-retry")
        return tr("That did not read. Lift the finger and try again.");
    if (result == "enroll-swipe-too-short")
        return tr("Hold the finger a little longer.");
    if (result == "enroll-finger-not-centered")
        return tr("Place the finger in the middle of the reader.");
    if (result == "enroll-data-full")
        return tr("The reader has no room for another fingerprint.");
    if (result == "enroll-already-enrolled")
        return tr("This finger is already enrolled.");
    return tr("The fingerprint could not be read.");
}

// The names a user gave their fingers; fprintd keeps the hardware name, so a
// rename is kept here, beside the rest of Settings' own preferences.
class FingerLabels
{
public:
    FingerLabels()
        : m_settings(cutefish::base::paths::cutefishConfig("touchid"))
    {
    }

    std::string label(const std::string& finger) const
    {
        const std::string custom = m_settings.stringValue("Labels/" + finger);
        return custom.empty() ? standardLabel(finger) : custom;
    }

    void setLabel(const std::string& finger, const std::string& label)
    {
        if (label.empty() || label == standardLabel(finger))
            m_settings.remove("Labels/" + finger);
        else
            m_settings.setValue("Labels/" + finger, label);
        m_settings.sync();
    }

private:
    cutefish::base::Settings m_settings;
};

// Watches for a reader, so the page only joins the sidebar on hardware that has
// one, as macOS only shows Touch ID on Macs with a reader.
class TouchIdWatch final : public AvailabilityWatch
{
public:
    TouchIdWatch(AvailabilityCallback callback)
        : m_manager(cutefish::base::dbus::Connection::system(), kService, kManagerPath, kManagerInterface)
        , m_callback(std::move(callback))
    {
        if (!m_manager.isValid()) {
            m_callback(false);
            return;
        }
        m_call = m_manager.callAsync("GetDevices", [this](const cutefish::base::dbus::Reply& reply) {
            const auto devices = reply.value<std::vector<ObjectPath>>();
            m_callback(devices && !devices->empty());
        });
    }

private:
    cutefish::base::dbus::Proxy m_manager;
    // Dropping a pending call drops the call.
    cutefish::base::dbus::PendingCall m_call;
    AvailabilityCallback m_callback;
};

// Settings → Touch ID, modelled on the pane of the same name in macOS: the
// reader's fingerprints, one Add row under them, and nothing else. It is not
// part of Users: the account page is about who the account is, this is about
// what the hardware is allowed to do.
class TouchIdPage final : public Page
{
public:
    explicit TouchIdPage(SettingsWindow& window)
        : Page(window, tr("Touch ID"))
        , m_window(window)
    {
        Section* section = content()->addChild<Section>(tr("Fingerprints"));
        m_list = section->addChild<Card>(Card::Style::List);
        m_status = content()->addChild<Footnote>();
        m_status->setVisible(false);
        m_note = content()->addChild<Footnote>(
                tr("A fingerprint unlocks this computer and answers administrator prompts that go through the "
                   "system's authentication stack. The print stays in the reader."));

        m_manager = cutefish::base::dbus::Proxy(
                cutefish::base::dbus::Connection::system(), kService, kManagerPath, kManagerInterface);
        if (!m_manager.isValid())
            return;
        m_manager.setInteractiveAuthorizationAllowed(true);
        keep(m_manager.callAsync("GetDevices", [this](const cutefish::base::dbus::Reply& reply) {
            const auto devices = reply.value<std::vector<ObjectPath>>();
            if (!devices || devices->empty())
                return;
            m_device = cutefish::base::dbus::Proxy(cutefish::base::dbus::Connection::system(), kService,
                                                   devices->front().path, kDeviceInterface);
            m_device.setInteractiveAuthorizationAllowed(true);
            m_enrollStatus = m_device.onSignal(
                    "EnrollStatus", [this](const cutefish::base::dbus::Message& message) {
                        std::string result;
                        bool done = false;
                        if (!message.read(result, done))
                            return;
                        onEnrollStatus(result, done);
                    });
            refresh();
        }));
        rebuild();
    }

private:
    void refresh()
    {
        if (!m_device.isValid())
            return;
        const std::string user = currentUserName();
        keep(m_device.callAsync(
                "ListEnrolledFingers",
                [this](const cutefish::base::dbus::Reply& reply) {
                    const auto fingers = reply.value<std::vector<std::string>>();
                    m_fingers = fingers ? *fingers : std::vector<std::string> {};
                    rebuild();
                },
                user));
    }

    void rebuild()
    {
        for (ListRow* row : m_rows)
            m_list->takeChild(row);
        m_rows.clear();

        for (const std::string& finger : m_fingers) {
            const std::string name = finger;
            ListRow* row = m_list->addChild<ListRow>(m_labels.label(name));
            row->setContentSpacing(8);

            IconButton* rename = row->addContent<IconButton>("touchid-pencil", 18);
            rename->onClicked = [this, name] { openRename(name); };
            IconButton* remove = row->addContent<IconButton>("touchid-trash", 18);
            remove->onClicked = [this, name] { removeFinger(name); };
            m_rows.push_back(row);
        }

        m_add = m_list->addChild<ListRow>(tr("Add Fingerprint…"));
        m_add->setSeparatorVisible(!m_fingers.empty());
        m_add->onClicked = [this] { openEnroll(); };
    }

    void removeFinger(const std::string& finger)
    {
        if (!m_device.isValid())
            return;
        keep(m_device.callAsync(
                "DeleteEnrolledFinger",
                [this, finger](const cutefish::base::dbus::Reply& reply) {
                    if (reply.isError()) {
                        setStatus(reply.error().message, true);
                        return;
                    }
                    m_labels.setLabel(finger, {});
                    setStatus({}, false);
                    refresh();
                },
                currentUserName(), finger));
    }

    // The same ten names the Add sheet offers: a print is one of the reader's
    // slots, so renaming picks a slot rather than typing a label.
    void openRename(const std::string& finger)
    {
        m_renameDialog = std::make_unique<FormDialog>(m_window, tr("Rename Fingerprint"), kDialogWidth);
        Card* form = m_renameDialog->content()->addChild<Card>(Card::Style::List);
        ListRow* row = form->addChild<ListRow>(tr("Finger"));
        row->setContentSpacing(12);
        RowComboBox* choice = row->addContent<RowComboBox>();

        std::vector<std::string> labels;
        const std::string shown = m_labels.label(finger);
        // A name saved before the list existed falls back to the slot the
        // reader enrolled, rather than the first item.
        int current = 0;
        for (int i = 0; i < int(std::size(kFingerNames)); ++i) {
            if (finger == kFingerNames[i].id)
                current = i;
        }
        for (int i = 0; i < int(std::size(kFingerNames)); ++i) {
            const std::string label = standardLabel(kFingerNames[i].id);
            labels.push_back(label);
            if (label == shown)
                current = i;
        }
        choice->setItems(labels);
        choice->setCurrentIndex(current);

        RowLayout* buttons = m_renameDialog->addButtonRow();
        buttons->addChild<DialogButton>(tr("Cancel"))->onClicked = [this] { m_renameDialog->close(); };
        DialogButton* save = buttons->addChild<DialogButton>(tr("Rename"), true);
        save->onClicked = [this, finger, choice] {
            const int index = choice->currentIndex();
            if (index >= 0 && index < int(std::size(kFingerNames)))
                m_labels.setLabel(finger, standardLabel(kFingerNames[index].id));
            m_renameDialog->close();
            rebuild();
        };
        m_renameDialog->show();
        choice->setFocus();
    }

    // Asks which finger, then shows the reader filling one touch at a time, as
    // macOS's Touch ID sheet does.
    void openEnroll()
    {
        std::string first;
        for (const FingerName& name : kFingerNames) {
            if (!isEnrolled(m_fingers, name.id)) {
                first = name.id;
                break;
            }
        }
        if (first.empty() || !m_device.isValid())
            return;

        m_enrolling = false;
        m_stage = 0;
        m_enrollDialog = std::make_unique<FormDialog>(m_window, tr("Add Fingerprint"), kDialogWidth);

        Card* form = m_enrollDialog->content()->addChild<Card>(Card::Style::List);
        ListRow* row = form->addChild<ListRow>(tr("Finger"));
        row->setContentSpacing(12);
        m_fingerChoice = row->addContent<RowComboBox>();
        std::vector<std::string> labels;
        for (const FingerName& name : kFingerNames)
            labels.push_back(m_labels.label(name.id));
        m_fingerChoice->setItems(labels);
        for (int i = 0; i < int(std::size(kFingerNames)); ++i) {
            if (first == kFingerNames[i].id)
                m_fingerChoice->setCurrentIndex(i);
        }

        m_progressCard = m_enrollDialog->content()->addChild<Card>();
        m_progressCard->setVisible(false);
        m_progressLabel = m_progressCard->addChild<Label>(tr("Touch the reader"));
        m_progressLabel->setWrap(true);
        m_progressLabel->setLayoutHints({.fillWidth = true});
        m_progress = m_progressCard->addChild<ProgressBar>();
        m_progress->setValue(0);

        RowLayout* buttons = m_enrollDialog->addButtonRow();
        DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
        cancel->onClicked = [this] { closeEnroll(true); };
        m_enrollButton = buttons->addChild<DialogButton>(tr("Add"), true);
        m_enrollButton->onClicked = [this] { startEnroll(); };
        // Closing the sheet with its control or Escape must not leave the
        // reader claimed and waiting for a finger.
        m_enrollDialog->onClosed = [this] {
            if (m_enrolling && m_device.isValid()) {
                m_device.send("EnrollStop");
                m_device.send("Release");
            }
            m_enrolling = false;
        };
        m_enrollDialog->show();
    }

    std::string selectedFinger() const
    {
        const int index = m_fingerChoice ? m_fingerChoice->currentIndex() : -1;
        if (index < 0 || index >= int(std::size(kFingerNames)))
            return {};
        return kFingerNames[index].id;
    }

    void startEnroll()
    {
        const std::string finger = selectedFinger();
        if (finger.empty())
            return;
        m_enrolling = true;
        m_stage = 0;
        m_fingerChoice->setEnabled(false);
        m_enrollButton->setEnabled(false);
        m_progressCard->setVisible(true);
        m_progress->setValue(0);
        m_progressLabel->setText(tr("Touch the reader"));
        keep(m_device.callAsync(
                "Claim",
                [this, finger](const cutefish::base::dbus::Reply& claim) {
                    if (claim.isError()) {
                        showEnrollError(claim.error().message);
                        return;
                    }
                    keep(m_device.callAsync(
                            "EnrollStart",
                            [this](const cutefish::base::dbus::Reply& start) {
                                if (start.isError())
                                    showEnrollError(start.error().message);
                            },
                            finger));
                },
                currentUserName()));
    }

    void onEnrollStatus(const std::string& result, bool done)
    {
        if (!m_enrolling)
            return;
        if (result == "enroll-completed" || (done && result == "enroll-completed")) {
            m_enrolling = false;
            m_progress->setValue(1);
            m_progressLabel->setText(tr("Fingerprint added."));
            m_device.send("Release");
            refresh();
            closeEnroll(false);
            return;
        }
        if (result == "enroll-stage-passed") {
            ++m_stage;
            m_progress->setValue(std::min(0.9, m_stage / kStagesShown));
        }
        if (done) {
            m_enrolling = false;
            m_device.send("Release");
            showEnrollError(enrollMessage(result));
            return;
        }
        if (result == "enroll-data-full") {
            m_enrolling = false;
            m_device.send("EnrollStop");
            m_device.send("Release");
            showEnrollError(enrollMessage(result));
            return;
        }
        m_progressLabel->setText(enrollMessage(result));
    }

    void showEnrollError(const std::string& message)
    {
        m_progressLabel->setText(message.empty() ? tr("The fingerprint was not added.") : message);
        m_progressLabel->setColor(accent::red);
        m_progress->setValue(0);
        m_fingerChoice->setEnabled(true);
        m_enrollButton->setEnabled(true);
        m_enrolling = false;
    }

    void closeEnroll(bool stop)
    {
        if (stop && m_enrolling && m_device.isValid()) {
            m_device.send("EnrollStop");
            m_device.send("Release");
        }
        m_enrolling = false;
        if (m_enrollDialog)
            m_enrollDialog->close();
        m_enrollDialog.reset();
        m_fingerChoice = nullptr;
        m_progress = nullptr;
        m_progressCard = nullptr;
        m_progressLabel = nullptr;
        m_enrollButton = nullptr;
    }

    void setStatus(const std::string& message, bool error)
    {
        m_status->setText(message);
        if (error)
            m_status->setColor(accent::red);
        m_status->setVisible(!message.empty());
    }

    // The bus unregisters a pending call whose slot is dropped, so every call
    // of this page keeps its slot for as long as the page lives. A list, so a
    // slot appended from a reply never moves one that is running.
    void keep(cutefish::base::dbus::PendingCall call) { m_calls.push_back(std::move(call)); }

    SettingsWindow& m_window;
    cutefish::base::dbus::Proxy m_manager;
    cutefish::base::dbus::Proxy m_device;
    std::list<cutefish::base::dbus::PendingCall> m_calls;
    cutefish::base::dbus::Match m_enrollStatus;
    FingerLabels m_labels;
    Card* m_list;
    Footnote* m_status;
    Footnote* m_note;
    ListRow* m_add = nullptr;
    std::vector<ListRow*> m_rows;
    std::vector<std::string> m_fingers;
    std::unique_ptr<FormDialog> m_renameDialog;
    std::unique_ptr<FormDialog> m_enrollDialog;
    RowComboBox* m_fingerChoice = nullptr;
    Card* m_progressCard = nullptr;
    Label* m_progressLabel = nullptr;
    ProgressBar* m_progress = nullptr;
    DialogButton* m_enrollButton = nullptr;
    bool m_enrolling = false;
    int m_stage = 0;
};

const PageRegistration registration(
        "touchid",
        [](SettingsWindow& window) { return std::make_unique<TouchIdPage>(window); },
        [](SettingsWindow&, AvailabilityCallback callback) {
            return std::make_unique<TouchIdWatch>(std::move(callback));
        });

} // namespace

} // namespace settings

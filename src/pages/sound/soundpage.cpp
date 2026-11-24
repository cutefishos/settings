// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "base/i18n.h"
#include "cutefish/audio/audio_model.h"
#include "cutefish/audio/backend.h"
#include "cutefish/audio/volume_monitor.h"
#include "cutefish/bluez/manager.h"
#include "base/signal.h"
#include "ui/button.h"
#include "ui/icon.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/segmented_control.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowcombobox.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish;
using namespace cutefish::audio;
using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;
using cutefish::i18n::trc;

namespace settings {

// What kind of device a row is, as people tell them apart.
static std::string deviceType(const AudioObject& device)
{
    if (device.isVirtualDevice())
        return tr("Virtual");
    const std::string bus = device.bus();
    if (bus == "bluetooth")
        return tr("Bluetooth");
    if (bus == "usb")
        return "USB";
    const std::vector<AudioObject::Choice> ports = device.ports();
    const int active = device.activePortIndex();
    if (active >= 0 && active < int(ports.size()) && ports[active].name.starts_with("hdmi"))
        return "HDMI";
    return tr("Built-in");
}

// BlueZ's Audio Sink profile: the headset as the A2DP sink of this computer.
static constexpr const char* kA2dpSinkUuid = "0000110b-0000-1000-8000-00805f9b34fb";

static bool isUsableProfile(const AudioObject::Choice& profile)
{
    return profile.name != "off" && profile.availability != AudioObject::Unavailable;
}

static bool isA2dpProfile(const AudioObject::Choice& profile)
{
    return profile.name.starts_with("a2dp-");
}

static bool isHeadsetProfile(const AudioObject::Choice& profile)
{
    return profile.name.starts_with("headset-");
}

static bool hasFrontPair(const AudioObject& device)
{
    const std::vector<std::string> channels = device.channels();
    return std::find(channels.begin(), channels.end(), "FL") != channels.end()
           && std::find(channels.begin(), channels.end(), "FR") != channels.end();
}

// -1 is all left, 1 all right. Only a device with both front channels has one.
static double balance(const AudioObject& device)
{
    const std::vector<std::string> channels = device.channels();
    const auto left = std::find(channels.begin(), channels.end(), "FL");
    const auto right = std::find(channels.begin(), channels.end(), "FR");
    const std::vector<double> volumes = device.channelVolumes();
    if (left == channels.end() || right == channels.end() || volumes.size() != channels.size())
        return 0;
    const double leftVolume = volumes[left - channels.begin()];
    const double rightVolume = volumes[right - channels.begin()];
    const double high = std::max(leftVolume, rightVolume);
    if (high <= 0)
        return 0;
    return leftVolume >= rightVolume ? rightVolume / high - 1 : 1 - leftVolume / high;
}

static void setBalance(AudioObject& device, double value)
{
    const std::vector<std::string> channels = device.channels();
    std::vector<double> volumes = device.channelVolumes();
    const double volume = device.volume();
    for (size_t i = 0; i < channels.size() && i < volumes.size(); ++i) {
        if (channels[i] == "FL")
            volumes[i] = volume * std::min(1.0, 1 - value);
        else if (channels[i] == "FR")
            volumes[i] = volume * std::min(1.0, 1 + value);
    }
    device.setChannelVolumes(volumes);
}

namespace {

// A label in the theme's secondary text colour.
class SecondaryLabel : public Label
{
public:
    using Label::Label;

protected:
    void layout() override
    {
        setColor(Theme::current().disabledText);
        Label::layout();
    }
};

// A sink or source: its name, its type and a check mark on the default one.
class DeviceRow : public ListRow
{
public:
    explicit DeviceRow(std::shared_ptr<AudioObject> device)
        : m_device(std::move(device))
    {
        setName("sound:device:" + std::to_string(m_device->index()));
        m_type = addContent<SecondaryLabel>();
        m_check = addContent<TrailingIcon>("check", 18);
        update();
    }

    const std::shared_ptr<AudioObject>& device() const { return m_device; }

    void update()
    {
        setText(m_device->displayName());
        m_type->setText(deviceType(*m_device));
        m_check->icon()->setVisible(m_device->isDefault());
    }

protected:
    void layout() override
    {
        m_check->icon()->setColor(Theme::current().highlight);
        ListRow::layout();
    }

private:
    std::shared_ptr<AudioObject> m_device;
    Label* m_type;
    TrailingIcon* m_check;
};

class SoundPage : public Page
{
public:
    explicit SoundPage(SettingsWindow& window)
        : Page(window, tr("Sound"))
        , m_backend(Backend::shared())
        , m_outputs(AudioObject::Sink, m_backend)
        , m_inputs(AudioObject::Source, m_backend)
        , m_monitor(m_backend)
    {
        m_connecting = content()->addChild<Card>();
        Label* connecting = m_connecting->addChild<Label>(tr("Connecting to the audio service…"));
        connecting->setWrap(true);
        connecting->setLayoutHints({.fillWidth = true});

        m_errorCard = content()->addChild<Card>();
        m_error = m_errorCard->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});
        Button* dismiss = m_errorCard->addChild<Button>(tr("Dismiss"));
        dismiss->onClicked = [this] { setOperationError({}); };

        ColumnLayout* devices = content()->addChild<ColumnLayout>();
        devices->setSpacing(metrics::l);
        devices->setLayoutHints({.fillWidth = true});
        m_tabs = devices->addChild<SegmentedControl>(std::vector<std::string>{tr("Output"), tr("Input")});
        m_tabs->setName("sound:tabs");
        m_tabs->setLayoutHints({.fillWidth = true});
        m_tabs->onCurrentChanged = [this](int) {
            rebuildDevices();
            update();
        };

        m_deviceCard = devices->addChild<Card>(Card::Style::List);
        m_noDevices = m_deviceCard->addChild<ListRow>();
        m_noDevices->setInteractive(false);
        m_noDevices->setEnabled(false);

        // A headset's card picks between A2DP and the hands-free profile; the
        // row only shows while there is a choice to make.
        m_modeCard = devices->addChild<Card>(Card::Style::List);
        m_modeCard->setName("sound:mode:card");
        m_modeCard->setVisible(false);
        ListRow* modeRow = m_modeCard->addChild<ListRow>(tr("Audio mode"));
        modeRow->setInteractive(false);
        m_mode = modeRow->addContent<RowComboBox>();
        m_mode->setName("sound:mode");
        m_mode->onActivated = [this](int index) { activateMode(index); };

        m_sliders = devices->addChild<Card>(Card::Style::List);
        m_sliders->setName("sound:sliders");
        m_outputVolume = m_sliders->addChild<SliderRow>(tr("Output volume"));
        m_outputVolume->setIcons("audio-volume-zero", "audio-volume-high");
        m_outputVolume->setRange(AudioObject::MinimalVolume, AudioObject::NormalVolume);
        m_outputVolume->setStepSize(0.01);
        m_outputVolume->onMoved = [this](double value) {
            if (const auto device = m_outputs.defaultNode()) {
                device->setVolume(value);
                // With no mute switch, muting shows as zero here.
                device->setMuted(value == 0);
            }
        };
        m_balance = m_sliders->addChild<SliderRow>(tr("Balance"));
        m_balance->setName("sound:balance");
        m_balance->setCaptions(trc("balance", "Left"), trc("balance", "Right"));
        m_balance->setRange(-1, 1);
        m_balance->setStepSize(0.02);
        m_balance->onMoved = [this](double value) {
            if (const auto device = m_outputs.defaultNode())
                setBalance(*device, value);
        };
        m_inputVolume = m_sliders->addChild<SliderRow>(tr("Input volume"));
        m_inputVolume->setRange(AudioObject::MinimalVolume, AudioObject::NormalVolume);
        m_inputVolume->setStepSize(0.01);
        m_inputVolume->onMoved = [this](double value) {
            if (const auto device = m_inputs.defaultNode()) {
                device->setVolume(value);
                device->setMuted(false);
            }
        };
        m_inputLevel = m_sliders->addChild<SliderRow>(tr("Input level"));
        m_inputLevel->setMeterLevel(0);

        m_subscriptions.push_back(m_backend->connectedChanged.connect([this] {
            setOperationError({});
            update();
        }));
        m_subscriptions.push_back(
            m_backend->operationFailed.connect([this](std::string message) { setOperationError(message); }));
        for (AudioModel* model : {&m_outputs, &m_inputs}) {
            m_subscriptions.push_back(model->rowsChanged.connect([this] {
                rebuildDevices();
                update();
            }));
            m_subscriptions.push_back(model->dataChanged.connect([this] { update(); }));
            m_subscriptions.push_back(model->defaultNodeChanged.connect([this] { update(); }));
        }
        m_subscriptions.push_back(m_monitor.volumeChanged.connect([this] { updateMeter(); }));
        // Cards are not in the sink and source models; their profiles can
        // change without a node coming or going.
        m_subscriptions.push_back(m_backend->objectAdded.connect(
                [this](const std::shared_ptr<AudioObject>&) { update(); }));
        m_subscriptions.push_back(m_backend->objectRemoved.connect(
                [this](const std::shared_ptr<AudioObject>&) { update(); }));
        rebuildDevices();
        setOperationError({});
        update();
    }

private:
    bool showInputs() const { return m_tabs->currentIndex() == 1; }
    AudioModel& model() { return showInputs() ? m_inputs : m_outputs; }

    void setOperationError(const std::string& message)
    {
        m_operationError = message;
        m_errorCard->setVisible(!message.empty());
        m_error->setText(format(tr("Could not apply the audio change: {error}"), {{"error", message}}));
    }

    void rebuildDevices()
    {
        for (DeviceRow* row : m_rows)
            m_deviceCard->takeChild(row);
        m_rows.clear();
        // The rows go before the placeholder, which stays last.
        std::unique_ptr<View> placeholder = m_deviceCard->takeChild(m_noDevices);
        for (const auto& device : model().objects()) {
            DeviceRow* row = m_deviceCard->addChild<DeviceRow>(device);
            const std::weak_ptr<AudioObject> target = device;
            row->onClicked = [this, target] {
                const auto object = target.lock();
                if (!object || object->isDefault())
                    return;
                setOperationError({});
                object->setDefault(true);
            };
            m_rows.push_back(row);
        }
        m_deviceCard->addChild(std::move(placeholder));
    }

    void update()
    {
        for (DeviceRow* row : m_rows)
            row->update();
        const bool inputs = showInputs();
        const bool connected = m_backend->isConnected();
        m_connecting->setVisible(!connected);
        m_noDevices->setText(inputs ? tr("No input devices") : tr("No output devices"));
        m_noDevices->setVisible(connected && model().count() == 0);

        const std::shared_ptr<AudioObject> output = m_outputs.defaultNode();
        const std::shared_ptr<AudioObject> input = m_inputs.defaultNode();
        m_sliders->setVisible(inputs ? input != nullptr : output && output->hasVolume());

        m_outputVolume->setVisible(!inputs && output && output->hasVolume());
        m_outputVolume->setName(output ? "sound:volume:" + std::to_string(output->index()) : std::string());
        m_outputVolume->setValue(output && !output->muted() ? output->volume() : 0);
        m_outputVolume->setSliderEnabled(output && output->volumeWritable());

        m_balance->setVisible(!inputs && output && hasFrontPair(*output));
        m_balance->setValue(output ? balance(*output) : 0);
        m_balance->setSliderEnabled(output && output->volumeWritable());

        m_inputVolume->setVisible(inputs && input && input->hasVolume());
        m_inputVolume->setName(input ? "sound:volume:" + std::to_string(input->index()) : std::string());
        m_inputVolume->setValue(input ? input->volume() : 0);
        m_inputVolume->setSliderEnabled(input && input->volumeWritable());

        m_inputLevel->setVisible(inputs && input);
        m_monitor.setTarget(inputs ? input : nullptr);
        updateMeter();
        updateMode();
    }

    // The Bluetooth card of the default sink or source. A headset is one card
    // with an A2DP and a hands-free profile, and only one of them can be active.
    void updateMode()
    {
        const std::shared_ptr<AudioObject> device =
                showInputs() ? m_inputs.defaultNode() : m_outputs.defaultNode();
        std::shared_ptr<AudioObject> card;
        if (device && device->bus() == "bluetooth" && device->cardIndex() != AudioObject::InvalidId)
            card = m_backend->object(device->cardIndex());

        if (card != m_card.lock()) {
            m_cardChanged.reset();
            m_card = card;
            if (card)
                m_cardChanged = card->changed.connect([this] { updateMode(); });
        }
        m_modes.clear();
        if (!card) {
            m_pendingA2dp = false;
            m_modeCard->setVisible(false);
            return;
        }

        // activeProfileIndex() and setActiveProfileIndex() count positions in
        // profiles(); the profiles' own index fields are PipeWire's ids.
        int bestA2dp = -1;
        int bestHeadset = -1;
        for (size_t i = 0; i < card->profiles().size(); ++i) {
            const AudioObject::Choice& profile = card->profiles()[i];
            if (!isUsableProfile(profile))
                continue;
            if (isA2dpProfile(profile)) {
                if (bestA2dp < 0 || profile.priority > card->profiles()[bestA2dp].priority)
                    bestA2dp = int(i);
            } else if (isHeadsetProfile(profile)) {
                if (bestHeadset < 0 || profile.priority > card->profiles()[bestHeadset].priority)
                    bestHeadset = int(i);
            }
        }

        std::vector<std::string> items;
        if (bestA2dp >= 0) {
            m_modes.push_back({bestA2dp, Mode::A2dp});
        } else {
            // A headset whose A2DP link is not up yet only lists its
            // hands-free profile. Asking BlueZ for the Audio Sink connects
            // the high-quality link, and PipeWire adds its profile.
            m_modes.push_back({-1, Mode::A2dp});
        }
        items.push_back(tr("High fidelity (A2DP)"));
        if (bestHeadset >= 0) {
            m_modes.push_back({bestHeadset, Mode::Headset});
            items.push_back(tr("Headset (HFP)"));
        }

        m_modeCard->setVisible(items.size() > 1);
        if (items.size() > 1) {
            // The row names the kind of link, not the codec profile behind
            // it, so any active A2DP or HFP profile checks the same item.
            const int active = card->activeProfileIndex();
            Mode::Kind activeKind = Mode::Other;
            if (active >= 0 && active < int(card->profiles().size())) {
                if (isA2dpProfile(card->profiles()[active]))
                    activeKind = Mode::A2dp;
                else if (isHeadsetProfile(card->profiles()[active]))
                    activeKind = Mode::Headset;
            }
            int current = -1;
            for (size_t i = 0; i < m_modes.size(); ++i) {
                if (m_modes[i].kind == activeKind)
                    current = int(i);
            }
            // Only when they change, so an open menu stays as it is.
            const bool changed = m_modeItems != items;
            if (changed) {
                m_modeItems = items;
                m_mode->setItems(std::move(items));
            }
            m_mode->setCurrentIndex(current);
            // Rebuilding the items does not relabel a row whose index stays put.
            if (changed)
                m_mode->setDisplayText(std::nullopt);
        }

        if (m_pendingA2dp && bestA2dp >= 0) {
            m_pendingA2dp = false;
            if (card->activeProfileIndex() != bestA2dp)
                card->setActiveProfileIndex(bestA2dp);
        }
    }

    void activateMode(int index)
    {
        if (index < 0 || index >= int(m_modes.size()))
            return;
        const std::shared_ptr<AudioObject> card = m_card.lock();
        if (!card)
            return;
        setOperationError({});
        const Mode& mode = m_modes[index];
        if (mode.profile >= 0) {
            if (mode.profile != card->activeProfileIndex())
                card->setActiveProfileIndex(mode.profile);
            return;
        }

        // Connect the A2DP link; the card's profiles then include it, and
        // updateMode() selects it.
        const std::string path = card->property("api.bluez5.path");
        if (path.empty())
            return;
        if (!m_bluez)
            m_bluez = bluez::Manager::shared();
        const std::shared_ptr<bluez::BluetoothObject> device = m_bluez ? m_bluez->object(path) : nullptr;
        if (!device) {
            setOperationError(tr("the Bluetooth device is not available"));
            return;
        }
        m_pendingA2dp = true;
        device->connectProfile(kA2dpSinkUuid);
    }

    void updateMeter()
    {
        // Peaks in dB over the bottom 50 dB, so speech fills about half.
        const double volume = m_monitor.volume();
        m_inputLevel->setMeterLevel(volume > 0 ? std::max(0.0, 1 + std::log10(volume) * 20 / 50) : 0);
    }

    struct Mode
    {
        enum Kind {
            Other,
            A2dp,
            Headset,
        };

        // The card's profile, or -1 when BlueZ must connect A2DP first.
        int profile = -1;
        Kind kind = Other;
    };

    std::shared_ptr<Backend> m_backend;
    AudioModel m_outputs;
    AudioModel m_inputs;
    VolumeMonitor m_monitor;
    std::string m_operationError;
    Card* m_connecting;
    Card* m_errorCard;
    Label* m_error;
    SegmentedControl* m_tabs;
    Card* m_deviceCard;
    ListRow* m_noDevices;
    std::vector<DeviceRow*> m_rows;
    Card* m_modeCard;
    RowComboBox* m_mode;
    std::vector<std::string> m_modeItems;
    std::vector<Mode> m_modes;
    std::weak_ptr<AudioObject> m_card;
    base::Subscription m_cardChanged;
    bool m_pendingA2dp = false;
    std::shared_ptr<bluez::Manager> m_bluez;
    Card* m_sliders;
    SliderRow* m_outputVolume;
    SliderRow* m_balance;
    SliderRow* m_inputVolume;
    SliderRow* m_inputLevel;
    std::vector<base::Subscription> m_subscriptions;
};

} // namespace

static const PageRegistration registration("sound", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<SoundPage>(window);
});

} // namespace settings

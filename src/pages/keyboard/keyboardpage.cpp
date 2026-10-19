// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <cmath>
#include <cstdlib>
#include <vector>

#include "base/i18n.h"
#include "pages/input/checkrow.h"
#include "pages/keyboard/inputmethod.h"
#include "pages/keyboard/keyboard.h"
#include "ui/label.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/sections.h"
#include "widgets/sliderrow.h"
#include "window/page.h"
#include "window/pagelist.h"
#include "window/settingswindow.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::tr;
using cutefish::i18n::trc;

namespace settings {

// Slider stops, slowest first, in milliseconds.
static const std::vector<int> kRepeatDelays{1500, 1000, 750, 500, 400, 300, 250, 200, 150};
static const std::vector<int> kRepeatIntervals{500, 250, 100, 67, 50, 40, 33, 25, 20};

static int nearestStop(const std::vector<int>& stops, int value)
{
    size_t best = 0;
    for (size_t i = 1; i < stops.size(); ++i) {
        if (std::abs(stops[i] - value) < std::abs(stops[best] - value))
            best = i;
    }
    return int(best);
}

namespace {

// A wrapped secondary text in a padded card.
class NoteCard : public Card
{
public:
    explicit NoteCard(std::string text)
    {
        m_label = addChild<Label>(std::move(text));
        m_label->setWrap(true);
        m_label->setLayoutHints({.fillWidth = true});
    }

protected:
    void layout() override
    {
        m_label->setColor(Theme::current().disabledText);
        Card::layout();
    }

private:
    Label* m_label;
};

class KeyboardPage : public Page
{
public:
    explicit KeyboardPage(SettingsWindow& window)
        : Page(window, tr("Keyboard"))
    {
        Section* inputMethods = content()->addChild<Section>(tr("Input method"));
        m_noInputMethods = inputMethods->addChild<NoteCard>(
                tr("Input methods can only be selected in a Cutefish Wayland session."));
        m_inputMethodCard = inputMethods->addChild<Card>(Card::Style::List);
        m_inputMethodCard->setName("keyboard:inputMethods");
        m_notRunning = inputMethods->addChild<Footnote>(tr("The selected input method is not running."));
        m_restartNote = inputMethods->addChild<Footnote>(
                tr("Applications started before the input method may need to be restarted."));
        m_restartNote->setOpacity(0.8);

        m_shortcutKey = content()->addChild<Section>(tr("Shortcut key"));
        Card* conventions = m_shortcutKey->addChild<Card>(Card::Style::List);
        m_pc = conventions->addChild<CheckRow>(trc("key", "Ctrl"));
        m_pc->onClicked = [this] { m_keyboard.setShortcutConvention("pc"); };
        m_command = conventions->addChild<CheckRow>(trc("key", "Command"));
        m_command->onClicked = [this] { m_keyboard.setShortcutConvention("command"); };
        m_conventionNote = m_shortcutKey->addChild<Footnote>();

        Card* navigation = content()->addChild<Card>(Card::Style::List);
        m_layout = navigation->addChild<NavigationItem>(tr("Layout"));
        m_layout->onClicked = [&window] { window.pushPage("keyboardlayout"); };
        NavigationItem* shortcuts = navigation->addChild<NavigationItem>(tr("Keyboard Shortcuts"));
        shortcuts->onClicked = [&window] { window.pushPage("shortcuts"); };

        m_repeatCard = content()->addChild<Card>(Card::Style::List);
        m_repeatCard->setName("keyboard:repeat");
        m_repeat = m_repeatCard->addChild<SwitchRow>(tr("Key repeat"));
        m_repeat->onToggled = [this](bool checked) { m_keyboard.setRepeatEnabled(checked); };
        m_delay = m_repeatCard->addChild<SliderRow>(tr("Delay until repeat"));
        m_delay->setRange(0, double(kRepeatDelays.size() - 1));
        m_delay->setStepSize(1);
        m_delay->onMoved = [this](double value) { m_keyboard.setRepeatDelay(kRepeatDelays[size_t(std::round(value))]); };
        m_rate = m_repeatCard->addChild<SliderRow>(tr("Repeat rate"));
        m_rate->setRange(0, double(kRepeatIntervals.size() - 1));
        m_rate->setStepSize(1);
        m_rate->onMoved = [this](double value) {
            m_keyboard.setRepeatInterval(kRepeatIntervals[size_t(std::round(value))]);
        };

        m_inputMethodChanged = m_inputMethod.changed().connect([this] { updateInputMethods(); });
        m_keyboardChanged = m_keyboard.changed().connect([this] { updateKeyboard(); });
        updateInputMethods();
        updateKeyboard();
    }

private:
    void updateInputMethods()
    {
        const bool available = m_inputMethod.isAvailable();
        const std::string current = m_inputMethod.current();
        m_noInputMethods->setVisible(!available);
        m_inputMethodCard->setVisible(available);
        m_notRunning->setVisible(available && !current.empty() && !m_inputMethod.isRunning());
        m_restartNote->setVisible(available);

        std::vector<InputMethod::Method> methods{{"", tr("None"), ""}};
        methods.insert(methods.end(), m_inputMethod.methods().begin(), m_inputMethod.methods().end());
        std::vector<std::string> ids;
        for (const InputMethod::Method& method : methods)
            ids.push_back(method.id);
        if (ids != m_methodIds) {
            while (!m_inputMethodCard->children().empty())
                m_inputMethodCard->takeChild(m_inputMethodCard->children().back().get());
            m_methodRows.clear();
            for (const InputMethod::Method& method : methods) {
                CheckRow* row = m_inputMethodCard->addChild<CheckRow>(method.name);
                row->setIcon(method.icon.empty() ? "input-keyboard" : method.icon);
                row->onClicked = [this, id = method.id] { m_inputMethod.select(id); };
                m_methodRows.push_back(row);
            }
            m_methodIds = ids;
        }
        for (size_t i = 0; i < m_methodRows.size(); ++i)
            m_methodRows[i]->setChecked(m_methodIds[i] == current);
    }

    void updateKeyboard()
    {
        const bool available = m_keyboard.isAvailable();
        const std::string convention = m_keyboard.shortcutConvention();
        m_shortcutKey->setVisible(available);
        m_pc->setChecked(convention == "pc");
        m_command->setChecked(convention == "command");
        m_conventionNote->setText(
                convention == "command"
                        ? tr("The key next to the space bar works as Command and the one beside it as Option, on "
                             "any keyboard: Command+C copies, Command+Tab switches windows, Command+Q quits. The "
                             "shortcuts can be changed in Keyboard Shortcuts.")
                        : tr("Ctrl is the shortcut key and Alt switches windows. The shortcuts can be changed in "
                             "Keyboard Shortcuts."));
        m_layout->setVisible(available);
        m_layout->setValue(m_keyboard.layoutName());
        m_repeatCard->setVisible(available);
        const bool repeat = m_keyboard.repeatEnabled();
        m_repeat->setChecked(repeat);
        m_delay->setEnabled(repeat);
        m_delay->setValue(nearestStop(kRepeatDelays, m_keyboard.repeatDelay()));
        m_rate->setEnabled(repeat);
        m_rate->setValue(nearestStop(kRepeatIntervals, m_keyboard.repeatInterval()));
    }

    InputMethod m_inputMethod;
    Keyboard m_keyboard;
    base::Subscription m_inputMethodChanged;
    base::Subscription m_keyboardChanged;
    NoteCard* m_noInputMethods;
    Card* m_inputMethodCard;
    Footnote* m_notRunning;
    Footnote* m_restartNote;
    std::vector<std::string> m_methodIds;
    std::vector<CheckRow*> m_methodRows;
    Section* m_shortcutKey;
    CheckRow* m_pc;
    CheckRow* m_command;
    Footnote* m_conventionNote;
    NavigationItem* m_layout;
    Card* m_repeatCard;
    SwitchRow* m_repeat;
    SliderRow* m_delay;
    SliderRow* m_rate;
};

} // namespace

static const PageRegistration registration("keyboard", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<KeyboardPage>(window);
});

} // namespace settings

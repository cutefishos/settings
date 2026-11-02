// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <chrono>
#include <memory>

#include <unicode/unistr.h>
#include <unicode/unum.h>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "base/locale.h"
#include "pages/language/addlanguagedialog.h"
#include "pages/language/chooseregiondialog.h"
#include "pages/language/language.h"
#include "pages/language/preferredlanguagelist.h"
#include "pages/language/region.h"
#include "ui/label.h"
#include "ui/layout.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"
#include "window/page.h"
#include "window/pagelist.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;
using cutefish::i18n::trc;

namespace settings {

static std::string automatic(const std::string& value)
{
    return format(tr("Automatic ({value})"), {{"value", value}});
}

static std::string measurementName(base::MeasurementSystem system)
{
    switch (system) {
    case base::MeasurementSystem::US:
        return trc("measurement system", "US");
    case base::MeasurementSystem::UK:
        return trc("measurement system", "UK");
    case base::MeasurementSystem::Metric:
        break;
    }
    return trc("measurement system", "Metric");
}

static std::string temperatureName(base::TemperatureUnit unit)
{
    return unit == base::TemperatureUnit::Fahrenheit ? tr("Fahrenheit (°F)") : tr("Celsius (°C)");
}

// An amount of money in the region's currency, as the formats write it.
static std::string currencyText(const base::Locale& locale, double amount)
{
    UErrorCode status = U_ZERO_ERROR;
    UNumberFormat* number = unum_open(UNUM_CURRENCY, nullptr, 0, locale.formatsId().c_str(), nullptr, &status);
    if (U_FAILURE(status))
        return locale.toString(amount, 2);
    UChar buffer[128];
    const int32_t length = unum_formatDouble(number, amount, buffer, 128, nullptr, &status);
    unum_close(number);
    if (U_FAILURE(status))
        return locale.toString(amount, 2);
    std::string text;
    icu::UnicodeString(buffer, length).toUTF8String(text);
    return text;
}

namespace {

class LanguagePage : public Page
{
public:
    explicit LanguagePage(SettingsWindow& window)
        : Page(window, tr("Language & Region"))
    {
        Section* section = content()->addChild<Section>(tr("Preferred languages"));
        Card* card = section->addChild<Card>(Card::Style::List);
        PreferredLanguageList* list = card->addChild<PreferredLanguageList>(m_language);
        list->setLayoutHints(
                {.fillWidth = true, .margins = {metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0}});

        m_add = card->addChild<ListRow>(tr("Add language…"));
        m_add->setName("language:add");
        m_add->setContentAbove(true);
        m_add->onClicked = [this] {
            m_dialog = std::make_unique<AddLanguageDialog>(settingsWindow(), m_language);
            m_dialog->onClosed = [this] {
                std::shared_ptr<AddLanguageDialog> closing(m_dialog.release());
                base::EventLoop::current()->post([closing] {});
            };
            m_dialog->show();
        };
        section->addChild<Footnote>(
                tr("Drag a language to the top to make it the system language. Right-click a language to remove "
                   "it."));

        m_error = content()->addChild<Label>();
        m_error->setWrap(true);
        m_error->setLayoutHints({.fillWidth = true});

        addRegionSection();
        addPreviewSection();

        m_subscriptions.push_back(m_language.savingChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_language.errorMessageChanged.connect([this] { update(); }));
        m_subscriptions.push_back(m_language.preferredLanguagesChanged.connect([this] { updateRegion(); }));
        m_subscriptions.push_back(m_region.changed.connect([this] { updateRegion(); }));
        update();
        updateRegion();
    }

private:
    void addRegionSection()
    {
        Section* section = content()->addChild<Section>(tr("Region"));
        Card* card = section->addChild<Card>(Card::Style::List);

        m_regionRow = card->addChild<NavigationItem>(tr("Region"));
        m_regionRow->setName("region:choose");
        m_regionRow->onClicked = [this] {
            if (m_regionDialog)
                return;
            m_regionDialog = std::make_unique<ChooseRegionDialog>(settingsWindow(), m_region);
            m_regionDialog->onClosed = [this] {
                std::shared_ptr<ChooseRegionDialog> closing(m_regionDialog.release());
                base::EventLoop::current()->post([closing] {});
            };
            m_regionDialog->show();
        };

        ListRow* weekRow = card->addChild<ListRow>(tr("First day of week"));
        weekRow->setInteractive(false);
        m_firstDay = weekRow->addContent<RowComboBox>();
        m_firstDay->setName("region:first-day");
        m_firstDay->onActivated = [this](int index) { m_region.setFirstDayOfWeek(index); };

        ListRow* measurementRow = card->addChild<ListRow>(tr("Measurement system"));
        measurementRow->setInteractive(false);
        m_measurement = measurementRow->addContent<RowComboBox>();
        m_measurement->setName("region:measurement");
        m_measurement->onActivated = [this](int index) {
            m_region.setMeasurementSystem(index == 0 ? std::nullopt
                                                     : std::optional(base::MeasurementSystem(index - 1)));
        };

        ListRow* temperatureRow = card->addChild<ListRow>(tr("Temperature"));
        temperatureRow->setInteractive(false);
        m_temperature = temperatureRow->addContent<RowComboBox>();
        m_temperature->setName("region:temperature");
        m_temperature->onActivated = [this](int index) {
            m_region.setTemperatureUnit(index == 0 ? std::nullopt : std::optional(base::TemperatureUnit(index - 1)));
        };

        section->addChild<Footnote>(tr("Applications that are already open may use the new region after they "
                                       "restart."));
    }

    void addPreviewSection()
    {
        Section* section = content()->addChild<Section>(tr("Format preview"));
        Card* card = section->addChild<Card>(Card::Style::List);
        m_previewDate = card->addChild<InfoRow>(tr("Date"));
        m_previewShort = card->addChild<InfoRow>(tr("Date and time"));
        m_previewNumber = card->addChild<InfoRow>(tr("Number"));
        m_previewCurrency = card->addChild<InfoRow>(tr("Currency"));
    }

    void update()
    {
        m_add->setEnabled(!m_language.isSaving());
        m_error->setText(m_language.errorMessage());
        m_error->setVisible(!m_language.errorMessage().empty());
    }

    void updateRegion()
    {
        const std::string region = m_region.region();
        m_regionRow->setValue(Region::displayName(region));

        const std::string language = m_language.preferredLanguages().empty()
                ? Language::systemLanguage()
                : m_language.preferredLanguages().front().code;
        const base::Locale defaults = base::Locale::regional(language, {.region = region});
        const base::Locale& display = base::Locale::system();
        const base::RegionalPreferences& preferences = m_region.preferences();

        std::vector<std::string> days {automatic(display.standaloneDayName(defaults.firstDayOfWeek()))};
        for (int day = 1; day <= 7; ++day)
            days.push_back(display.standaloneDayName(day));
        m_firstDay->setItems(days);
        m_firstDay->setCurrentIndex(preferences.firstDayOfWeek);

        std::vector<std::string> systems {automatic(measurementName(defaults.measurementSystem()))};
        for (base::MeasurementSystem system :
             {base::MeasurementSystem::Metric, base::MeasurementSystem::US, base::MeasurementSystem::UK})
            systems.push_back(measurementName(system));
        m_measurement->setItems(systems);
        m_measurement->setCurrentIndex(preferences.measurementSystem ? int(*preferences.measurementSystem) + 1 : 0);

        const bool fahrenheit = defaults.temperatureUnit() == base::TemperatureUnit::Fahrenheit;
        m_temperature->setItems(std::vector<std::string> {automatic(fahrenheit ? "°F" : "°C"),
                                                          temperatureName(base::TemperatureUnit::Celsius),
                                                          temperatureName(base::TemperatureUnit::Fahrenheit)});
        m_temperature->setCurrentIndex(preferences.temperatureUnit ? int(*preferences.temperatureUnit) + 1 : 0);

        const base::Locale formats = m_region.locale();
        const auto now = std::chrono::system_clock::now();
        const base::DateTime dateTime = base::DateTime::fromSecondsSinceEpoch(
                std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
        m_previewDate->setValue(formats.toString(dateTime.date, base::Locale::FormatType::Long));
        m_previewShort->setValue(formats.toString(dateTime.date, base::Locale::FormatType::Short) + ' '
                                 + formats.toString(dateTime.time, base::Locale::FormatType::Short));
        m_previewNumber->setValue(formats.toString(1234567.89, 2));
        m_previewCurrency->setValue(currencyText(formats, 1234.56));
    }

    Language m_language;
    Region m_region;
    std::vector<base::Subscription> m_subscriptions;
    ListRow* m_add;
    Label* m_error;
    NavigationItem* m_regionRow;
    RowComboBox* m_firstDay;
    RowComboBox* m_measurement;
    RowComboBox* m_temperature;
    InfoRow* m_previewDate;
    InfoRow* m_previewShort;
    InfoRow* m_previewNumber;
    InfoRow* m_previewCurrency;
    std::unique_ptr<AddLanguageDialog> m_dialog;
    std::unique_ptr<ChooseRegionDialog> m_regionDialog;
};

} // namespace

static const PageRegistration registration("language", [](SettingsWindow& window) -> std::unique_ptr<Page> {
    return std::make_unique<LanguagePage>(window);
});

} // namespace settings

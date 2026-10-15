// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "window/page.h"

#include <algorithm>

#include "ui/painter.h"
#include "ui/quick_layout.h"
#include "ui/theme.h"
#include "widgets/metrics.h"
#include "widgets/sections.h"
#include "window/settingswindow.h"

using namespace cutefish::ui;

namespace settings {

// The Flickable's content: the page's column inside its margins, as tall as
// the column's implicit height and the margins.
class PageContent : public View
{
public:
    PageContent()
    {
        column = addChild<ColumnLayout>();
        column->setSpacing(metrics::sectionSpacing);
    }

    SizeF sizeHint() const override
    {
        const SizeF size = column->sizeHint();
        return {size.width + margins.horizontal(), size.height + margins.vertical()};
    }

    double heightForWidth(double width) const override
    {
        return column->heightForWidth(std::max(0.0, width - margins.horizontal())) + margins.vertical();
    }

    ColumnLayout* column;
    const Margins margins{metrics::pageMargin, SettingsWindow::headerHeight + metrics::pageTopMargin,
                          metrics::pageMargin, metrics::pageMargin};

protected:
    // The column fills the page less the margins, as when its ColumnLayout
    // is anchored to a page that does not scroll; a scrolling page is exactly
    // as tall as the column wants.
    void layout() override
    {
        const double width = std::max(0.0, geometry().width - margins.horizontal());
        const double height = std::max(column->heightForWidth(width), geometry().height - margins.vertical());
        column->setGeometry({margins.left, margins.top, width, height});
    }
};

Page::Page(SettingsWindow& window, std::string title)
    : m_settingsWindow(window)
    , m_title(std::move(title))
{
    m_content = setContent<PageContent>()->column;
    // The bar runs from under the floating header.
    setVerticalBarPadding(SettingsWindow::headerHeight, Theme::current().smallRadius);
}

Page::~Page() = default;

// Opaque: only the sidebar shows the window's frosted backdrop.
void Page::paint(Painter& painter)
{
    painter.fillRect(localRect(), Theme::current().background);
}

PendingPage::PendingPage(SettingsWindow& window, std::string title)
    : Page(window, std::move(title))
{
    content()->addChild<Footnote>("This page is not ported yet.");
}

} // namespace settings

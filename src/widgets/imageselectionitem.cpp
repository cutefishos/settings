// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "widgets/imageselectionitem.h"

#include "ui/label.h"
#include "ui/painter.h"
#include "ui/cell_layout.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/metrics.h"
#include "widgets/picture.h"

using namespace cutefish::ui;

namespace settings {

// The preview: a picture inside the ring's inset or a view of its own size
// centred, and the ring while checked. The ring sits the inset outside the
// picture; its radius grows by as much, so its corners stay concentric with
// the picture's.
class SelectionPreview : public View
{
public:
    SelectionPreview(SizeF size, double inset)
        : m_size(size)
        , m_inset(inset)
    {
        setLayoutHints({.preferredWidth = size.width, .preferredHeight = size.height});
    }

    void setPicture(std::string path)
    {
        m_picture = addChild<Picture>(std::move(path));
    }

    void setIcon(std::unique_ptr<View> icon)
    {
        m_icon = icon.get();
        addChild(std::move(icon));
    }

    void setChecked(bool checked)
    {
        m_checked = checked;
        requestPaint();
    }

    SizeF sizeHint() const override { return m_size; }

protected:
    void layout() override
    {
        const RectF bounds = localRect();
        if (m_picture) {
            m_picture->setCornerRadius(Theme::current().bigRadius);
            m_picture->setGeometry(bounds.shrunk(Margins::all(m_inset)));
        }
        if (m_icon) {
            const SizeF icon = m_icon->sizeHint();
            m_icon->setGeometry({rounding::centeredStart(bounds.width, icon.width),
                                 rounding::centeredStart(bounds.height, icon.height), icon.width, icon.height});
        }
    }

    void paint(Painter& painter) override
    {
        if (!m_checked)
            return;
        const Theme& theme = Theme::current();
        painter.drawRoundedRect(localRect(), {theme.bigRadius, -m_inset, themeCorners()}, Color{0, 0, 0, 0}, 2,
                                theme.highlight);
    }

private:
    SizeF m_size;
    double m_inset;
    Picture* m_picture = nullptr;
    View* m_icon = nullptr;
    bool m_checked = false;
};

ImageSelectionItem::ImageSelectionItem(std::string path, std::string text, SizeF previewSize)
{
    m_layout = addChild<ColumnLayout>();
    m_preview = m_layout->addChild<SelectionPreview>(previewSize, 4);
    m_preview->setPicture(std::move(path));
    build(std::move(text));
}

ImageSelectionItem::ImageSelectionItem(std::unique_ptr<View> icon, std::string text, SizeF previewSize,
                                       double previewInset)
{
    m_layout = addChild<ColumnLayout>();
    m_preview = m_layout->addChild<SelectionPreview>(previewSize, previewInset);
    m_preview->setIcon(std::move(icon));
    build(std::move(text));
}

void ImageSelectionItem::build(std::string text)
{
    m_layout->setSpacing(metrics::s);
    m_label = m_layout->addChild<Label>(std::move(text));
    m_label->setVisible(!m_label->text().empty());
    m_label->setLayoutHints({.horizontalAlignment = Alignment::Center});
}

void ImageSelectionItem::setChecked(bool checked)
{
    if (checked == m_checked)
        return;
    m_checked = checked;
    m_preview->setChecked(checked);
    requestLayout();
}

// The item is as large as its layout, as the Qt version's was.
SizeF ImageSelectionItem::sizeHint() const
{
    return m_layout->sizeHint();
}

double ImageSelectionItem::heightForWidth(double width) const
{
    return m_layout->heightForWidth(width);
}

void ImageSelectionItem::layout()
{
    const Theme& theme = Theme::current();
    m_label->setColor(m_checked ? theme.highlight : theme.text);
    m_layout->setGeometry(localRect());
}

void ImageSelectionItem::pointerPress(const PointerEvent& event)
{
    m_pressed = event.button == MouseButton::Left;
}

void ImageSelectionItem::pointerRelease(const PointerEvent& event)
{
    if (!m_pressed)
        return;
    m_pressed = false;
    if (localRect().contains(event.position) && onClicked)
        onClicked();
}

} // namespace settings

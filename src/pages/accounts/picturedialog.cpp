// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/accounts/picturedialog.h"

#include <optional>

#include "base/file.h"
#include "base/i18n.h"
#include "base/paths.h"
#include "cutefish/accounts/account_picture.h"
#include "ui/cell_layout.h"
#include "ui/dialog_window.h"
#include "ui/file_chooser.h"
#include "ui/image_view.h"
#include "ui/painter.h"
#include "ui/theme.h"
#include "ui/window.h"
#include "widgets/metrics.h"

using namespace cutefish;
using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;

namespace settings {

static constexpr double kWidth = 400;
static constexpr int kColumns = 5;

// A preset picture in a circle, with a ring around it while it is selected,
// fainter while hovered.
class PictureTile : public View
{
public:
    static constexpr double kPictureSize = 56;
    static constexpr double kRingWidth = 2;
    static constexpr double kInset = 4;
    static constexpr double kSize = kPictureSize + 2 * kInset;

    explicit PictureTile(std::string path)
        : m_path(std::move(path))
    {
        m_picture = addChild<ImageView>();
        m_picture->setFillMode(ImageView::FillMode::PreserveAspectCrop);
        m_picture->setSourceSize({kPictureSize, kPictureSize});
        m_picture->setMipmap(true);
        m_picture->setCornerRadius(kPictureSize / 2);
        m_picture->setSource(m_path);
    }

    const std::string& path() const { return m_path; }

    void setSelected(bool selected)
    {
        if (selected == m_selected)
            return;
        m_selected = selected;
        requestPaint();
    }

    SizeF sizeHint() const override { return {kSize, kSize}; }
    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::function<void()> onClicked;

protected:
    void layout() override { m_picture->setGeometry(localRect().adjusted(kInset, kInset, -kInset, -kInset)); }

    void paint(Painter& painter) override
    {
        const RectF bounds = localRect();
        const double ring = m_selected ? 1 : m_hovered ? 0.4 : 0;
        if (ring > 0) {
            painter.drawRoundedRect(bounds, {bounds.width / 2, 0, Corners::Circular}, Color::rgb(0).withAlpha(0),
                                    kRingWidth, Theme::current().highlight.withAlpha(ring));
        }
    }

    void pointerEnter() override
    {
        m_hovered = true;
        requestPaint();
    }

    void pointerLeave() override
    {
        m_hovered = false;
        requestPaint();
    }

    void pointerPress(const PointerEvent& event) override { m_pressed = event.button == MouseButton::Left; }

    void pointerRelease(const PointerEvent& event) override
    {
        if (!m_pressed)
            return;
        m_pressed = false;
        if (localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    std::string m_path;
    ImageView* m_picture;
    bool m_selected = false;
    bool m_hovered = false;
    bool m_pressed = false;
};

// The picture the account will have, large, on the account's colour until it
// is decoded or where there is none.
class PicturePreview : public View
{
public:
    static constexpr double kSize = 96;

    explicit PicturePreview(const std::string& userName)
        : m_color(Color::rgb(accounts::pictureColor(userName)))
    {
        m_picture = addChild<ImageView>();
        m_picture->setFillMode(ImageView::FillMode::PreserveAspectCrop);
        m_picture->setSourceSize({kSize, kSize});
        m_picture->setMipmap(true);
        m_picture->setCornerRadius(kSize / 2);
        m_picture->onStatusChanged = [this](ImageView::Status) { requestPaint(); };
    }

    void setPicture(const std::string& path) { m_picture->setSource(path); }

    SizeF sizeHint() const override { return {kSize, kSize}; }

protected:
    void layout() override { m_picture->setGeometry(localRect()); }

    void paint(Painter& painter) override
    {
        if (m_picture->status() != ImageView::Status::Ready)
            painter.fillEllipse(localRect(), m_color);
    }

private:
    Color m_color;
    ImageView* m_picture;
};

PictureDialog::PictureDialog(SettingsWindow& parent, std::shared_ptr<accounts::UserAccount> account)
    : FormDialog(parent, tr("Choose a picture"), kWidth)
    , m_account(std::move(account))
{
    setCloseVisible(false);

    m_preview = content()->addChild<PicturePreview>(m_account->userName());
    m_preview->setLayoutHints({.horizontalAlignment = Alignment::Center});

    // The buttons keep more room above them than the dialog's spacing gives.
    GridLayout* grid = content()->addChild<GridLayout>(kColumns);
    grid->setColumnSpacing(metrics::m);
    grid->setRowSpacing(metrics::m);
    grid->setLayoutHints({.horizontalAlignment = Alignment::Center, .margins = {0, 0, 0, metrics::s}});

    // The service may keep a copy of a preset instead of its path, so a preset
    // is found by its contents too.
    m_initial = m_account->iconFileName();
    const std::optional<std::string> contents = base::file::read(m_initial);
    std::string preset;
    for (const std::string& path : accounts::presetPictures()) {
        PictureTile* tile = grid->addChild<PictureTile>(path);
        tile->onClicked = [this, path] { select(path); };
        m_tiles.push_back(tile);
        if (preset.empty() && (path == m_initial || (contents && !contents->empty() && base::file::read(path) == contents)))
            preset = path;
    }
    if (!preset.empty())
        m_initial = preset;

    RowLayout* buttons = addButtonRow();
    buttons->setLayoutHints({.fillWidth = true});
    DialogButton* choose = buttons->addChild<DialogButton>(tr("Choose File…"));
    choose->setName("picture-choose-file");
    choose->onClicked = [this] { chooseFile(); };
    buttons->addChild<View>()->setLayoutHints({.fillWidth = true});
    DialogButton* cancel = buttons->addChild<DialogButton>(tr("Cancel"));
    cancel->onClicked = [this] { requestClose(); };
    m_save = buttons->addChild<DialogButton>(tr("Save"), true);
    m_save->onClicked = [this] { save(); };

    select(m_initial);
}

PictureDialog::~PictureDialog() = default;

void PictureDialog::select(const std::string& path)
{
    m_selected = path;
    m_preview->setPicture(path);
    for (PictureTile* tile : m_tiles)
        tile->setSelected(tile->path() == path);
    m_save->setEnabled(!m_selected.empty() && m_selected != m_initial);
}

void PictureDialog::chooseFile()
{
    m_chooser = std::make_unique<FileChooser>(window());
    m_chooser->setMode(FileChooser::Mode::OpenFile);
    m_chooser->setModal(true);
    m_chooser->setFilters(
            {FileChooser::Filter::fromNameFilter(
                     format(tr("Image files ({patterns})"),
                            {{"patterns", "*.jpg *.jpeg *.png *.webp *.bmp *.tif *.tiff *.avif"}})),
             FileChooser::Filter::fromNameFilter(tr("All files (*)"))});
    m_chooser->setCurrentFolder(base::paths::userDirectory(base::paths::UserDirectory::Pictures));
    m_chooser->onAccepted = [this](const std::vector<std::string>& paths, const std::string&) {
        if (!paths.empty())
            select(paths.front());
    };
    m_chooser->open();
}

void PictureDialog::save()
{
    if (m_selected.empty())
        return;
    m_account->setIconFileName(m_selected);
    if (onPicked)
        onPicked(m_selected);
    close();
}

} // namespace settings

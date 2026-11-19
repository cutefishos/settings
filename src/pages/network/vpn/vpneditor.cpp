// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/network/vpn/vpneditor.h"

#include <algorithm>
#include <climits>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "pages/network/common/textchecks.h"
#include "ui/button.h"
#include "ui/file_chooser.h"
#include "ui/label.h"
#include "ui/cell_layout.h"
#include "ui/text_field.h"
#include "ui/theme.h"
#include "widgets/card.h"
#include "widgets/listrow.h"
#include "widgets/metrics.h"
#include "widgets/rowcombobox.h"
#include "widgets/sections.h"

using namespace cutefish::ui;
using cutefish::i18n::format;
using cutefish::i18n::tr;
namespace nm = cutefish::networkmanager;
namespace schema = cutefish::networkmanager::vpn::schema;

namespace settings::network {

static constexpr double kFieldWidth = 240;
// A field sharing its width with another control keeps this much.
static constexpr double kMinimumFieldWidth = 120;

namespace {

// Text in the accent colour that acts when clicked.
class LinkLabel : public View
{
public:
    explicit LinkLabel(std::string text)
    {
        m_label = addChild<Label>(std::move(text));
    }

    void setText(std::string text) { m_label->setText(std::move(text)); }

    SizeF sizeHint() const override { return m_label->sizeHint(); }
    bool acceptsPointer() const override { return true; }
    CursorShape cursorShape() const override { return CursorShape::Pointer; }

    std::function<void()> onClicked;

protected:
    void layout() override
    {
        m_label->setColor(Theme::current().highlight);
        m_label->setGeometry(localRect());
    }

    void pointerRelease(const PointerEvent& event) override
    {
        if (event.button == MouseButton::Left && localRect().contains(event.position) && onClicked)
            onClicked();
    }

private:
    Label* m_label;
};

} // namespace

// Below a card, lined up with the card's text.
static LinkLabel* addLink(ColumnLayout* parent, const std::string& text)
{
    LinkLabel* link = parent->addChild<LinkLabel>(text);
    link->setLayoutHints({.margins = {metrics::cardHorizontalPadding, 0, metrics::cardHorizontalPadding, 0}});
    return link;
}

// The fields of a plugin VPN: the basic ones as rows of the editor's first
// card, then the advanced ones, a card per section, behind a toggle.
class PluginForm : public ColumnLayout
{
public:
    std::function<void()> onAccepted;
    std::function<void()> onChanged;

    PluginForm(Card* basic, VpnFormModel& model)
        : m_model(model)
        , m_basic(basic)
    {
        setSpacing(metrics::dialogSpacing);
        setLayoutHints({.fillWidth = true});
    }

    // The rows of the model's fields, after it was loaded.
    void load()
    {
        m_advancedShown = false;
        build();
    }

    void sharedChanged()
    {
        for (FieldRow& row : m_rows) {
            if (row.storage)
                loadStorage(row);
        }
        update();
    }

    void secretsChanged()
    {
        reload();
    }

private:
    struct FieldRow
    {
        const schema::FormField* field = nullptr;
        ListRow* row = nullptr;
        TextFieldRow* fieldRow = nullptr;
        TextField* text = nullptr;
        RowComboBox* box = nullptr;
        RowComboBox* storage = nullptr;
        SwitchRow* toggle = nullptr;
    };

    // An advanced section and its rows in m_rows, hidden while none of them shows.
    struct Group
    {
        Section* section = nullptr;
        size_t first = 0;
        size_t last = 0;
    };

    void build()
    {
        while (!children().empty())
            takeChild(children().front().get());
        m_rows.clear();
        m_groups.clear();
        m_rows.reserve(m_model.fields().size());

        bool hasAdvanced = false;
        for (const schema::FormField& field : m_model.fields()) {
            if (!field.advanced)
                addRow(m_basic, field);
            hasAdvanced |= field.advanced;
        }

        m_link = addLink(this, tr("Show advanced options"));
        m_link->setVisible(hasAdvanced);
        m_link->onClicked = [this] {
            m_advancedShown = !m_advancedShown;
            update();
        };

        m_advanced = addChild<ColumnLayout>();
        m_advanced->setSpacing(metrics::dialogSpacing);
        m_advanced->setLayoutHints({.fillWidth = true});
        std::string section;
        Card* card = nullptr;
        for (const schema::FormField& field : m_model.fields()) {
            if (!field.advanced)
                continue;
            if (!card || field.section != section) {
                section = field.section;
                Section* group = m_advanced->addChild<Section>(section);
                group->title()->setVisible(!section.empty());
                card = group->addChild<Card>(Card::Style::List);
                m_groups.push_back({group, m_rows.size(), m_rows.size()});
            }
            addRow(card, field);
            m_groups.back().last = m_rows.size();
        }
        reload();
    }

    TextFieldRow* addFieldRow(Card* card, FieldRow& row)
    {
        TextFieldRow* fieldRow = card->addChild<TextFieldRow>(row.field->label, kFieldWidth);
        row.row = fieldRow;
        row.fieldRow = fieldRow;
        row.text = fieldRow->field();
        row.text->setName("vpn:field:" + row.field->key);
        row.text->onAccepted = [this] { accepted(); };
        return fieldRow;
    }

    void addRow(Card* card, const schema::FormField& field)
    {
        FieldRow row;
        row.field = &field;
        const std::string key = field.key;
        switch (field.type) {
        case schema::Type::Password: {
            TextFieldRow* fieldRow = addFieldRow(card, row);
            row.text->setPlaceholder(tr("Asked on connect"));
            row.text->setEchoMode(TextField::EchoMode::Password);
            row.text->onTextEdited = [this, key, text = row.text] {
                m_model.setSecret(key, text->text());
                changed();
            };
            row.storage = fieldRow->addContent<RowComboBox>();
            row.storage->onActivated = [this, index = m_rows.size()](int choice) {
                m_model.setStorageIndex(*m_rows[index].field, choice);
                update();
            };
            break;
        }
        case schema::Type::File: {
            TextFieldRow* fieldRow = addFieldRow(card, row);
            row.text->setPlaceholder(field.required ? tr("Required") : tr("Optional"));
            row.text->onTextEdited = [this, key, text = row.text] { setValue(key, text->text()); };
            Button* choose = fieldRow->addContent<Button>(tr("Choose…"));
            // The field and its button share the other fields' width.
            fieldRow->setFieldWidth(
                    std::max(kMinimumFieldWidth, kFieldWidth - metrics::rowSpacing - choose->sizeHint().width));
            const std::string title = field.label;
            choose->onClicked = [this, key, title] {
                m_fileChooser = std::make_unique<FileChooser>(window());
                m_fileChooser->setTitle(title);
                m_fileChooser->onAccepted = [this, key](const std::vector<std::string>& paths, const std::string&) {
                    if (paths.empty())
                        return;
                    setValue(key, paths.front());
                    reload();
                };
                m_fileChooser->open();
            };
            break;
        }
        case schema::Type::Choice: {
            std::vector<std::string> labels;
            for (const schema::Choice& choice : field.choices)
                labels.push_back(choice.label);
            row.row = card->addChild<ListRow>(field.label);
            row.row->setInteractive(false);
            row.box = row.row->addContent<RowComboBox>(labels);
            row.box->onActivated = [this, key, choices = field.choices](int index) {
                if (index >= 0 && size_t(index) < choices.size())
                    setValue(key, choices[size_t(index)].value);
            };
            break;
        }
        case schema::Type::Bool: {
            row.toggle = card->addChild<SwitchRow>(field.label);
            row.row = row.toggle;
            row.toggle->onToggled = [this, key](bool checked) { setValue(key, checked ? "true" : "false"); };
            break;
        }
        default: {
            addFieldRow(card, row);
            row.text->setPlaceholder(!field.placeholder.empty() ? field.placeholder
                                     : field.required           ? tr("Required")
                                                                : tr("Optional"));
            if (field.type == schema::Type::Number) {
                const long long maximum = field.maximum > 0 ? field.maximum : INT_MAX;
                row.text->setValidator([maximum](std::string_view text) { return isNumberText(text, maximum); });
            }
            row.text->onTextEdited = [this, key, text = row.text] { setValue(key, text->text()); };
            break;
        }
        }
        m_rows.push_back(std::move(row));
    }

    void loadStorage(FieldRow& row)
    {
        std::vector<std::string> labels;
        for (const VpnFormModel::StorageChoice& choice : m_model.storageChoices(*row.field))
            labels.push_back(choice.label);
        row.storage->setItems(labels);
        row.storage->setCurrentIndex(m_model.storageIndex(*row.field));
    }

    // The fields read their values again.
    void reload()
    {
        for (FieldRow& row : m_rows) {
            const std::string& key = row.field->key;
            if (row.field->type == schema::Type::Password) {
                row.text->setText(m_model.secret(key));
                loadStorage(row);
            } else if (row.text) {
                row.text->setText(m_model.value(key));
            } else if (row.box) {
                row.box->setCurrentIndex(m_model.choiceIndex(*row.field));
            } else if (row.toggle) {
                row.toggle->setChecked(m_model.value(key) == "true");
            }
        }
        update();
    }

    void setValue(const std::string& key, const std::string& value)
    {
        m_model.setValue(key, value);
        update();
    }

    void accepted()
    {
        if (onAccepted)
            onAccepted();
    }

    void changed()
    {
        if (onChanged)
            onChanged();
    }

    void update()
    {
        for (FieldRow& row : m_rows) {
            row.row->setVisible(m_model.shown(*row.field));
            if (row.storage) {
                row.text->setVisible(m_model.saved(row.field->key));
                // The field and its storage choice share the other fields' width.
                const double choice = row.storage->sizeHint().width - row.storage->trailingOverhang();
                row.fieldRow->setFieldWidth(std::max(kMinimumFieldWidth, kFieldWidth - metrics::rowSpacing - choice));
            }
        }
        for (const Group& group : m_groups) {
            bool any = false;
            for (size_t i = group.first; i < group.last; ++i)
                any |= m_rows[i].row->isVisible();
            group.section->setVisible(any);
        }
        m_link->setText(m_advancedShown ? tr("Hide advanced options") : tr("Show advanced options"));
        m_advanced->setVisible(m_advancedShown);
        requestLayout();
        changed();
    }

    VpnFormModel& m_model;
    bool m_advancedShown = false;
    Card* m_basic;
    std::vector<FieldRow> m_rows;
    std::vector<Group> m_groups;
    LinkLabel* m_link = nullptr;
    ColumnLayout* m_advanced = nullptr;
    std::unique_ptr<FileChooser> m_fileChooser;
};

// A WireGuard tunnel as wg-quick lays it out: the interface, its first rows
// in the editor's first card, then a card per peer.
class WireGuardEditor : public ColumnLayout
{
public:
    std::function<void()> onAccepted;
    std::function<void()> onChanged;

    WireGuardEditor(Card* basic, VpnFormModel& model)
        : m_model(model)
    {
        setSpacing(metrics::dialogSpacing);
        setLayoutHints({.fillWidth = true});

        m_privateKey = addField(basic, tr("Private key"), tr("Required"),
                                [this](const std::string& text) { m_model.wireGuard().privateKey = text; });
        m_privateKey->setName("vpn:privateKey");
        m_privateKey->setEchoMode(TextField::EchoMode::Password);
        m_address = addField(basic, tr("Address"), "10.0.0.2/24",
                             [this](const std::string& text) { m_model.wireGuard().addresses = text; });
        m_address->setName("vpn:address");
        m_dns = addField(basic, tr("DNS"), tr("Optional"),
                         [this](const std::string& text) { m_model.wireGuard().dns = text; });

        m_link = addLink(this, tr("Show advanced options"));
        m_link->onClicked = [this] {
            m_advancedShown = !m_advancedShown;
            update();
        };

        m_advanced = addChild<Card>(Card::Style::List);
        m_interface = addField(m_advanced, tr("Interface name"), tr("From the name"),
                               [this](const std::string& text) { m_model.wireGuard().interfaceName = text; });
        m_interface->setMaximumLength(15);
        m_listenPort = addField(m_advanced, tr("Listen port"), tr("Automatic"),
                                [this](const std::string& text) { m_model.wireGuard().listenPort = text; });
        m_listenPort->setValidator([](std::string_view text) { return isNumberText(text, 65535); });
        m_fwmark = addField(m_advanced, tr("Firewall mark"), tr("Optional"),
                            [this](const std::string& text) { m_model.wireGuard().fwmark = text; });
        m_mtu = addField(m_advanced, tr("MTU"), tr("Automatic"),
                         [this](const std::string& text) { m_model.wireGuard().mtu = text; });
        m_mtu->setValidator([](std::string_view text) { return isNumberText(text, INT_MAX); });
        m_peerRoutes = m_advanced->addChild<SwitchRow>(tr("Add routes for the peers' allowed IPs"));
        m_peerRoutes->setChecked(true);
        m_peerRoutes->onToggled = [this](bool checked) { m_model.wireGuard().peerRoutes = checked; };

        m_peers = addChild<ColumnLayout>();
        m_peers->setSpacing(metrics::dialogSpacing);
        m_peers->setLayoutHints({.fillWidth = true});
        m_addPeer = addLink(this, tr("Add peer"));
        m_addPeer->onClicked = [this] {
            m_model.addPeer();
            addPeer(m_model.wireGuard().peers.back());
            update();
        };
    }

    // The fields of the model's interface and peers, after it was loaded.
    void load()
    {
        const nm::vpn::WireGuardForm& form = m_model.wireGuard();
        m_interface->setText(form.interfaceName);
        m_privateKey->setText(form.privateKey);
        m_address->setText(form.addresses);
        m_dns->setText(form.dns);
        m_listenPort->setText(form.listenPort);
        m_fwmark->setText(form.fwmark);
        m_mtu->setText(form.mtu);
        m_peerRoutes->setChecked(form.peerRoutes);
        m_advancedShown = false;
        while (!m_peers->children().empty())
            m_peers->takeChild(m_peers->children().front().get());
        m_peerCards.clear();
        for (const nm::vpn::WireGuardPeer& peer : form.peers)
            addPeer(peer);
        update();
    }

    void secretsChanged()
    {
        const nm::vpn::WireGuardForm& form = m_model.wireGuard();
        m_privateKey->setText(form.privateKey);
        for (size_t i = 0; i < m_peerCards.size(); ++i)
            m_peerCards[i].presharedKey->setText(form.peers[i].presharedKey);
        update();
    }

private:
    struct PeerCard
    {
        Card* card = nullptr;
        ListRow* header = nullptr;
        Button* remove = nullptr;
        TextField* publicKey = nullptr;
        TextField* presharedKey = nullptr;
        TextField* endpoint = nullptr;
        TextField* allowedIps = nullptr;
        TextField* keepalive = nullptr;
    };

    TextField* addField(Card* card, const char* label, const char* placeholder,
                        std::function<void(const std::string& text)> edited)
    {
        TextField* field = card->addChild<TextFieldRow>(label, kFieldWidth)->field();
        field->setPlaceholder(placeholder);
        field->onTextEdited = [this, field, edited = std::move(edited)] {
            edited(field->text());
            changed();
        };
        field->onAccepted = [this] {
            if (onAccepted)
                onAccepted();
        };
        return field;
    }

    // The peer's place in the model, as its card's among the cards.
    size_t peerIndex(const Card* owner) const
    {
        const auto found = std::find_if(m_peerCards.begin(), m_peerCards.end(),
                                        [owner](const PeerCard& card) { return card.card == owner; });
        return size_t(found - m_peerCards.begin());
    }

    nm::vpn::WireGuardPeer& peerOf(const Card* owner)
    {
        return m_model.wireGuard().peers[peerIndex(owner)];
    }

    void addPeer(const nm::vpn::WireGuardPeer& peer)
    {
        PeerCard card;
        card.card = m_peers->addChild<Card>(Card::Style::List);
        card.header = card.card->addChild<ListRow>(tr("Peer"));
        card.header->setInteractive(false);
        card.remove = card.header->addContent<Button>(tr("Remove"));
        card.remove->setName("vpn:removePeer:" + std::to_string(m_peerCards.size()));
        Card* owner = card.card;
        card.remove->onClicked = [this, owner] {
            const size_t index = peerIndex(owner);
            if (index == m_peerCards.size())
                return;
            m_model.removePeer(index);
            m_peerCards.erase(m_peerCards.begin() + ptrdiff_t(index));
            // The card holds the button being clicked.
            std::shared_ptr<View> gone = m_peers->takeChild(owner);
            update();
            cutefish::base::EventLoop::current()->post([gone] {});
        };
        card.publicKey = addField(card.card, tr("Public key"), tr("Required"),
                                  [this, owner](const std::string& text) { peerOf(owner).publicKey = text; });
        card.publicKey->setName("vpn:publicKey:" + std::to_string(m_peerCards.size()));
        card.publicKey->setText(peer.publicKey);
        card.presharedKey = addField(card.card, tr("Preshared key"),
                                     // TRANSLATORS: a placeholder: the stored key stays unless a new one is typed.
                                     peer.hasPresharedKey ? tr("Kept") : tr("Optional"),
                                     [this, owner](const std::string& text) {
                                         m_model.setPresharedKey(peerIndex(owner), text);
                                     });
        card.presharedKey->setEchoMode(TextField::EchoMode::Password);
        card.presharedKey->setText(peer.presharedKey);
        card.endpoint = addField(card.card, tr("Endpoint"), "vpn.example.com:51820",
                                 [this, owner](const std::string& text) { peerOf(owner).endpoint = text; });
        card.endpoint->setText(peer.endpoint);
        card.allowedIps = addField(card.card, tr("Allowed IPs"), tr("All traffic"),
                                   [this, owner](const std::string& text) { peerOf(owner).allowedIps = text; });
        card.allowedIps->setText(peer.allowedIps);
        card.keepalive = addField(card.card, tr("Keepalive"), tr("Off"),
                                  [this, owner](const std::string& text) { peerOf(owner).keepalive = text; });
        card.keepalive->setValidator([](std::string_view text) { return isNumberText(text, 65535); });
        card.keepalive->setText(peer.keepalive);
        m_peerCards.push_back(card);
    }

    void changed()
    {
        if (onChanged)
            onChanged();
    }

    void update()
    {
        m_link->setText(m_advancedShown ? tr("Hide advanced options") : tr("Show advanced options"));
        m_advanced->setVisible(m_advancedShown);
        const bool several = m_peerCards.size() > 1;
        for (size_t i = 0; i < m_peerCards.size(); ++i) {
            m_peerCards[i].header->setText(several ? format(tr("Peer {number}"), {{"number", std::to_string(i + 1)}})
                                                   : std::string(tr("Peer")));
            m_peerCards[i].remove->setVisible(several);
        }
        // Without peers the column would still take a spacing.
        m_peers->setVisible(!m_peerCards.empty());
        requestLayout();
        changed();
    }

    VpnFormModel& m_model;
    TextField* m_privateKey;
    TextField* m_address;
    TextField* m_dns;
    LinkLabel* m_link;
    Card* m_advanced;
    TextField* m_interface;
    TextField* m_listenPort;
    TextField* m_fwmark;
    TextField* m_mtu;
    SwitchRow* m_peerRoutes;
    ColumnLayout* m_peers;
    LinkLabel* m_addPeer;
    std::vector<PeerCard> m_peerCards;
    bool m_advancedShown = false;
};

VpnEditor::VpnEditor()
{
    setSpacing(metrics::dialogSpacing);
    setLayoutHints({.fillWidth = true});

    m_notInstalled = addChild<Footnote>();

    m_basic = addChild<Card>(Card::Style::List);
    m_name = m_basic->addChild<TextFieldRow>(tr("Name"), kFieldWidth)->field();
    m_name->setName("vpn:name");
    m_name->setPlaceholder(tr("Optional"));
    m_name->onTextEdited = [this] { m_model.setName(m_name->text()); };
    m_name->onAccepted = [this] {
        if (onAccepted)
            onAccepted();
    };

    m_switches = addChild<Card>(Card::Style::List);
    m_shared = m_switches->addChild<SwitchRow>(tr("Available to all users"));
    m_shared->onToggled = [this](bool checked) {
        if (checked == m_model.shared())
            return;
        m_model.setShared(checked);
        if (m_plugin)
            m_plugin->sharedChanged();
    };
    m_ownNetworkOnly = m_switches->addChild<SwitchRow>(tr("Use only for resources on its network"));
    m_ownNetworkOnly->onToggled = [this](bool checked) { m_model.setOwnNetworkOnly(checked); };
}

VpnEditor::~VpnEditor() = default;

void VpnEditor::load(const nm::VpnForm& form)
{
    m_model.load(form);
    m_name->setText(m_model.name());
    m_shared->setChecked(m_model.shared());
    m_ownNetworkOnly->setChecked(m_model.ownNetworkOnly());

    if (m_plugin)
        takeChild(m_plugin);
    if (m_wireGuard)
        takeChild(m_wireGuard);
    m_plugin = nullptr;
    m_wireGuard = nullptr;
    // The kind's own rows follow the name's.
    while (m_basic->children().size() > 1)
        m_basic->takeChild(m_basic->children().back().get());
    if (form.installed) {
        // Between the first card and the switches.
        std::unique_ptr<View> switches = takeChild(m_switches);
        if (m_model.isWireGuard()) {
            m_wireGuard = addChild<WireGuardEditor>(m_basic, m_model);
            m_wireGuard->onAccepted = [this] {
                if (onAccepted)
                    onAccepted();
            };
            m_wireGuard->onChanged = [this] { changed(); };
            m_wireGuard->load();
        } else {
            m_plugin = addChild<PluginForm>(m_basic, m_model);
            m_plugin->onAccepted = [this] {
                if (onAccepted)
                    onAccepted();
            };
            m_plugin->onChanged = [this] { changed(); };
            m_plugin->load();
        }
        addChild(std::move(switches));
    }

    m_notInstalled->setText(format(tr("Install {package} to set up this kind of VPN."), {{"package", form.package}}));
    m_notInstalled->setVisible(!form.installed && !form.kind.empty());
    m_basic->setVisible(form.installed);
    m_switches->setVisible(form.installed);
    changed();
}

void VpnEditor::setSecrets(const nm::VpnSecrets& secrets)
{
    m_model.setSecrets(secrets);
    if (m_plugin)
        m_plugin->secretsChanged();
    if (m_wireGuard)
        m_wireGuard->secretsChanged();
}

nm::VpnValues VpnEditor::collect() const
{
    return m_model.collect();
}

bool VpnEditor::complete() const
{
    return m_model.complete();
}

void VpnEditor::focusFirst()
{
    m_name->setFocus();
}

void VpnEditor::changed()
{
    requestLayout();
    if (onChanged)
        onChanged();
}

} // namespace settings::network

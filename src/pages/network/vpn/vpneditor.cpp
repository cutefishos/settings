// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include "pages/network/vpn/vpneditor.h"

#include <algorithm>
#include <climits>

#include "base/event_loop.h"
#include "base/i18n.h"
#include "cutefish/networkmanager/types.h"
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

    explicit PluginForm(Card* basic)
        : m_basic(basic)
    {
        setSpacing(metrics::dialogSpacing);
        setLayoutHints({.fillWidth = true});
    }

    void load(const std::vector<schema::FormField>& fields, bool shared)
    {
        m_fields = fields;
        m_shared = shared;
        m_values.clear();
        m_storage.clear();
        m_secrets.clear();
        for (const schema::FormField& field : fields) {
            m_values[field.key] = field.type == schema::Type::Bool ? (field.checked ? "true" : "false") : field.value;
            if (field.type == schema::Type::Password)
                m_storage[field.key] = field.storage;
        }
        m_advancedShown = false;
        build();
    }

    void setShared(bool shared)
    {
        if (shared == m_shared)
            return;
        m_shared = shared;
        for (FieldRow& row : m_rows) {
            if (row.storage)
                loadStorage(row);
        }
        update();
    }

    void setSecrets(const std::map<std::string, std::string>& secrets)
    {
        m_secrets.clear();
        for (const schema::FormField& field : m_fields) {
            const auto found = secrets.find(field.key);
            if (field.type == schema::Type::Password && found != secrets.end())
                m_secrets[field.key] = found->second;
        }
        reload();
    }

    const schema::Values& values() const { return m_values; }
    const std::map<std::string, uint32_t>& storage() const { return m_storage; }
    const std::map<std::string, std::string>& secrets() const { return m_secrets; }

    bool complete() const
    {
        for (const schema::FormField& field : m_fields) {
            const auto value = m_values.find(field.key);
            const std::string text = value == m_values.end() ? std::string() : value->second;
            if (field.required && shown(field) && text.find_first_not_of(" \t\n") == std::string::npos)
                return false;
        }
        return true;
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
        std::vector<uint32_t> storageValues;
    };

    // An advanced section and its rows in m_rows, hidden while none of them shows.
    struct Group
    {
        Section* section = nullptr;
        size_t first = 0;
        size_t last = 0;
    };

    bool shown(const schema::Field& field) const
    {
        std::vector<schema::Field> plain(m_fields.begin(), m_fields.end());
        return schema::shown(field, plain, m_values);
    }

    void build()
    {
        while (!children().empty())
            takeChild(children().front().get());
        m_rows.clear();
        m_groups.clear();
        m_rows.reserve(m_fields.size());

        bool hasAdvanced = false;
        for (const schema::FormField& field : m_fields) {
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
        for (const schema::FormField& field : m_fields) {
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
                m_secrets[key] = text->text();
                changed();
            };
            row.storage = fieldRow->addContent<RowComboBox>();
            row.storage->onActivated = [this, index = m_rows.size()](int choice) {
                FieldRow& current = m_rows[index];
                if (choice >= 0 && size_t(choice) < current.storageValues.size())
                    m_storage[current.field->key] = current.storageValues[size_t(choice)];
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
        const uint32_t saveFlags = m_shared ? nm::SecretSystemOwned : nm::SecretAgentOwned;
        std::vector<std::string> labels{m_shared ? tr("Saved for all") : tr("Saved"), tr("Ask each time")};
        row.storageValues = {saveFlags, nm::SecretNotSaved};
        if (row.field->allowNotRequired) {
            labels.push_back(tr("Not needed"));
            row.storageValues.push_back(nm::SecretNotRequired);
        }
        row.storage->setItems(labels);
        const auto found = m_storage.find(row.field->key);
        const uint32_t stored = found == m_storage.end() ? nm::SecretAgentOwned : found->second;
        const uint32_t wanted = stored == 0 || stored == 1 ? saveFlags : stored;
        const auto index = std::find(row.storageValues.begin(), row.storageValues.end(), wanted);
        row.storage->setCurrentIndex(index == row.storageValues.end() ? 0 : int(index - row.storageValues.begin()));
    }

    // The fields read their values again.
    void reload()
    {
        for (FieldRow& row : m_rows) {
            const std::string& key = row.field->key;
            const auto value = m_values.find(key);
            const std::string text = value == m_values.end() ? std::string() : value->second;
            if (row.field->type == schema::Type::Password) {
                const auto secret = m_secrets.find(key);
                row.text->setText(secret == m_secrets.end() ? std::string() : secret->second);
                loadStorage(row);
            } else if (row.text) {
                row.text->setText(text);
            } else if (row.box) {
                int index = 0;
                for (size_t i = 0; i < row.field->choices.size(); ++i) {
                    if (row.field->choices[i].value == text)
                        index = int(i);
                }
                row.box->setCurrentIndex(index);
            } else if (row.toggle) {
                row.toggle->setChecked(text == "true");
            }
        }
        update();
    }

    void setValue(const std::string& key, const std::string& value)
    {
        m_values[key] = value;
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
            row.row->setVisible(shown(*row.field));
            if (row.storage) {
                const auto found = m_storage.find(row.field->key);
                const uint32_t storage = found == m_storage.end() ? nm::SecretAgentOwned : found->second;
                const bool saving = storage != nm::SecretNotSaved && storage != nm::SecretNotRequired;
                row.text->setVisible(saving);
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

    std::vector<schema::FormField> m_fields;
    schema::Values m_values;
    std::map<std::string, uint32_t> m_storage;
    std::map<std::string, std::string> m_secrets;
    bool m_shared = false;
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

    explicit WireGuardEditor(Card* basic)
    {
        setSpacing(metrics::dialogSpacing);
        setLayoutHints({.fillWidth = true});

        m_privateKey = addField(basic, tr("Private key"), tr("Required"));
        m_privateKey->setEchoMode(TextField::EchoMode::Password);
        m_address = addField(basic, tr("Address"), "10.0.0.2/24");
        m_dns = addField(basic, tr("DNS"), tr("Optional"));

        m_link = addLink(this, tr("Show advanced options"));
        m_link->onClicked = [this] {
            m_advancedShown = !m_advancedShown;
            update();
        };

        m_advanced = addChild<Card>(Card::Style::List);
        m_interface = addField(m_advanced, tr("Interface name"), tr("From the name"));
        m_interface->setMaximumLength(15);
        m_listenPort = addField(m_advanced, tr("Listen port"), tr("Automatic"));
        m_listenPort->setValidator([](std::string_view text) { return isNumberText(text, 65535); });
        m_fwmark = addField(m_advanced, tr("Firewall mark"), tr("Optional"));
        m_mtu = addField(m_advanced, tr("MTU"), tr("Automatic"));
        m_mtu->setValidator([](std::string_view text) { return isNumberText(text, INT_MAX); });
        m_peerRoutes = m_advanced->addChild<SwitchRow>(tr("Add routes for the peers' allowed IPs"));
        m_peerRoutes->setChecked(true);

        m_peers = addChild<ColumnLayout>();
        m_peers->setSpacing(metrics::dialogSpacing);
        m_peers->setLayoutHints({.fillWidth = true});
        m_addPeer = addLink(this, tr("Add peer"));
        m_addPeer->onClicked = [this] {
            addPeer({});
            update();
        };
    }

    void load(const nm::vpn::WireGuardForm& form)
    {
        m_interface->setText(form.interfaceName);
        m_privateKey->setText({});
        m_hasPrivateKey = false;
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
        if (m_peerCards.empty())
            addPeer({});
        update();
    }

    void setSecrets(const nm::VpnSecrets& secrets)
    {
        if (!secrets.privateKey.empty()) {
            m_privateKey->setText(secrets.privateKey);
            m_hasPrivateKey = true;
        }
        for (PeerCard& card : m_peerCards) {
            const auto key = secrets.presharedKeys.find(card.publicKey->text());
            if (key != secrets.presharedKeys.end())
                card.presharedKey->setText(key->second);
        }
        update();
    }

    nm::vpn::WireGuardForm collect() const
    {
        nm::vpn::WireGuardForm form;
        form.interfaceName = m_interface->text();
        form.privateKey = m_privateKey->text();
        form.addresses = m_address->text();
        form.dns = m_dns->text();
        form.listenPort = m_listenPort->text();
        form.fwmark = m_fwmark->text();
        form.mtu = m_mtu->text();
        form.peerRoutes = m_peerRoutes->isChecked();
        for (const PeerCard& card : m_peerCards) {
            form.peers.push_back({card.publicKey->text(), card.presharedKey->text(), card.hasPresharedKey,
                                  card.endpoint->text(), card.allowedIps->text(), card.keepalive->text()});
        }
        return form;
    }

    bool complete() const
    {
        const auto blank = [](const std::string& text) { return text.find_first_not_of(" \t\n") == std::string::npos; };
        if (blank(m_address->text()))
            return false;
        if (!m_hasPrivateKey && blank(m_privateKey->text()))
            return false;
        if (m_peerCards.empty())
            return false;
        return std::none_of(m_peerCards.begin(), m_peerCards.end(),
                            [&blank](const PeerCard& card) { return blank(card.publicKey->text()); });
    }

private:
    struct PeerCard
    {
        Card* card = nullptr;
        ListRow* header = nullptr;
        Button* remove = nullptr;
        TextField* publicKey = nullptr;
        TextField* presharedKey = nullptr;
        bool hasPresharedKey = false;
        TextField* endpoint = nullptr;
        TextField* allowedIps = nullptr;
        TextField* keepalive = nullptr;
    };

    TextField* addField(Card* card, const char* label, const char* placeholder)
    {
        TextField* field = card->addChild<TextFieldRow>(label, kFieldWidth)->field();
        field->setPlaceholder(placeholder);
        field->onTextEdited = [this] { changed(); };
        field->onAccepted = [this] {
            if (onAccepted)
                onAccepted();
        };
        return field;
    }

    void addPeer(const nm::vpn::WireGuardPeer& peer)
    {
        PeerCard card;
        card.card = m_peers->addChild<Card>(Card::Style::List);
        card.header = card.card->addChild<ListRow>(tr("Peer"));
        card.header->setInteractive(false);
        card.remove = card.header->addContent<Button>(tr("Remove"));
        Card* owner = card.card;
        card.remove->onClicked = [this, owner] {
            const auto found = std::find_if(m_peerCards.begin(), m_peerCards.end(),
                                            [owner](const PeerCard& card) { return card.card == owner; });
            if (found == m_peerCards.end())
                return;
            m_peerCards.erase(found);
            // The card holds the button being clicked.
            std::shared_ptr<View> gone = m_peers->takeChild(owner);
            update();
            cutefish::base::EventLoop::current()->post([gone] {});
        };
        card.publicKey = addField(card.card, tr("Public key"), tr("Required"));
        card.publicKey->setText(peer.publicKey);
        card.presharedKey = addField(card.card, tr("Preshared key"),
                                     // TRANSLATORS: a placeholder: the stored key stays unless a new one is typed.
                                     peer.hasPresharedKey ? tr("Kept") : tr("Optional"));
        card.presharedKey->setEchoMode(TextField::EchoMode::Password);
        card.presharedKey->setText(peer.presharedKey);
        card.hasPresharedKey = peer.hasPresharedKey;
        card.endpoint = addField(card.card, tr("Endpoint"), "vpn.example.com:51820");
        card.endpoint->setText(peer.endpoint);
        card.allowedIps = addField(card.card, tr("Allowed IPs"), tr("All traffic"));
        card.allowedIps->setText(peer.allowedIps);
        card.keepalive = addField(card.card, tr("Keepalive"), tr("Off"));
        card.keepalive->setValidator([](std::string_view text) { return isNumberText(text, 65535); });
        card.keepalive->setText(peer.keepalive);
        TextField* preshared = card.presharedKey;
        card.presharedKey->onTextEdited = [this, owner, preshared] {
            // Clearing the field drops the stored key.
            for (PeerCard& card : m_peerCards) {
                if (card.card == owner)
                    card.hasPresharedKey = !preshared->text().empty();
            }
            changed();
        };
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
    bool m_hasPrivateKey = false;
    bool m_advancedShown = false;
};

VpnEditor::VpnEditor()
{
    setSpacing(metrics::dialogSpacing);
    setLayoutHints({.fillWidth = true});

    m_notInstalled = addChild<Footnote>();

    m_basic = addChild<Card>(Card::Style::List);
    m_name = m_basic->addChild<TextFieldRow>(tr("Name"), kFieldWidth)->field();
    m_name->setPlaceholder(tr("Optional"));
    m_name->onAccepted = [this] {
        if (onAccepted)
            onAccepted();
    };

    m_switches = addChild<Card>(Card::Style::List);
    m_shared = m_switches->addChild<SwitchRow>(tr("Available to all users"));
    m_shared->onToggled = [this](bool checked) {
        if (m_plugin)
            m_plugin->setShared(checked);
    };
    m_ownNetworkOnly = m_switches->addChild<SwitchRow>(tr("Use only for resources on its network"));
}

VpnEditor::~VpnEditor() = default;

void VpnEditor::load(const nm::VpnForm& form)
{
    m_form = form;
    m_name->setText(form.name);
    m_shared->setChecked(form.shared);
    m_ownNetworkOnly->setChecked(form.ownNetworkOnly);

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
        if (form.kind == "wireguard") {
            m_wireGuard = addChild<WireGuardEditor>(m_basic);
            m_wireGuard->onAccepted = [this] {
                if (onAccepted)
                    onAccepted();
            };
            m_wireGuard->onChanged = [this] { changed(); };
            m_wireGuard->load(form.wireguard);
        } else {
            m_plugin = addChild<PluginForm>(m_basic);
            m_plugin->onAccepted = [this] {
                if (onAccepted)
                    onAccepted();
            };
            m_plugin->onChanged = [this] { changed(); };
            m_plugin->load(form.fields, form.shared);
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
    if (m_plugin)
        m_plugin->setSecrets(secrets.values);
    if (m_wireGuard)
        m_wireGuard->setSecrets(secrets);
}

nm::VpnValues VpnEditor::collect() const
{
    nm::VpnValues values;
    values.kind = m_form.kind;
    values.name = m_name->text();
    values.shared = m_shared->isChecked();
    values.ownNetworkOnly = m_ownNetworkOnly->isChecked();
    if (m_plugin) {
        values.values = m_plugin->values();
        values.storage = m_plugin->storage();
        values.secrets = m_plugin->secrets();
    }
    if (m_wireGuard)
        values.wireguard = m_wireGuard->collect();
    return values;
}

bool VpnEditor::complete() const
{
    if (!m_form.installed)
        return false;
    if (m_plugin)
        return m_plugin->complete();
    if (m_wireGuard)
        return m_wireGuard->complete();
    return false;
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

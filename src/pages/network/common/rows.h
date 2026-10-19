// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#pragma once

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cutefish/networkmanager/network_model.h"
#include "widgets/listrow.h"

namespace cutefish::ui {
class BusyIndicator;
class Icon;
}

namespace settings {
class IconButton;
}

namespace settings::network {

// Puts the views in this order at the end of their parent's children.
void arrange(cutefish::ui::View* parent, const std::vector<cutefish::ui::View*>& order);

// The rows a card shows for a list of networks, one per network key, kept
// across changes so a row's state and animation carry on.
template<typename RowType>
class KeyedRows
{
public:
    explicit KeyedRows(std::function<RowType*(cutefish::ui::View* parent)> make)
        : m_make(std::move(make))
    {
    }

    // The rows for the keys in this order; rows of other keys go.
    std::vector<RowType*> update(cutefish::ui::View* parent, const std::vector<std::string>& keys)
    {
        std::map<std::string, RowType*> kept;
        std::vector<RowType*> rows;
        for (const std::string& key : keys) {
            auto found = m_rows.find(key);
            RowType* row = found != m_rows.end() ? found->second : m_make(parent);
            if (found != m_rows.end())
                m_rows.erase(found);
            kept[key] = row;
            rows.push_back(row);
        }
        for (auto& [key, row] : m_rows)
            parent->takeChild(row);
        m_rows = std::move(kept);
        return rows;
    }

private:
    std::function<RowType*(cutefish::ui::View* parent)> m_make;
    std::map<std::string, RowType*> m_rows;
};

// One wireless network. Clicking connects; the current connection is managed
// from the info dialog.
class WifiRow : public ListRow
{
public:
    WifiRow();

    // `displayed` is the state the row shows, which may lag behind NetworkManager's.
    void setItem(const cutefish::networkmanager::NetworkItem& item,
                 cutefish::networkmanager::ConnectionStatus displayed);
    const cutefish::networkmanager::NetworkItem& item() const { return m_item; }
    cutefish::networkmanager::ConnectionStatus displayedState() const { return m_displayed; }

    std::function<void(WifiRow* row)> onInfo;

protected:
    void layout() override;

private:
    cutefish::networkmanager::NetworkItem m_item;
    cutefish::networkmanager::ConnectionStatus m_displayed = cutefish::networkmanager::ConnectionStatus::Deactivated;
    cutefish::ui::Icon* m_icon;
    cutefish::ui::BusyIndicator* m_busy;
    cutefish::ui::Icon* m_check;
    cutefish::ui::Icon* m_lock;
    IconButton* m_info;
};

// Joined with a user name, which the join window asks for.
bool isEnterprise(cutefish::networkmanager::SecurityType type);

} // namespace settings::network

// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <functional>

#include "pagetest.h"
#include "pages/about/aboutinfo.h"
#include "pages/about/devicename.h"
#include "ui/keys.h"
#include "ui/test_input.h"
#include "ui/text_field.h"
#include "window/page.h"

TEST(AboutPage, ShowsTheSystemRows)
{
    PageFixture page("about");
    EXPECT_NE(page.dump().find("\"page:about\""), std::string::npos);
    // Version, type, kernel, processor, memory and storage all have values
    // here; the model row depends on the machine's firmware.
    const int modelRows = settings::about::computerModel().empty() ? 0 : 1;
    EXPECT_EQ(page.count("settings::InfoRow"), 6 + modelRows);
}

TEST(AboutPage, EditsTheDeviceName)
{
    PageFixture page("about");
    EXPECT_NE(page.dump().find("\"about:name\""), std::string::npos);
    EXPECT_EQ(page.count("TextField"), 1);
}

TEST(AboutPage, ReturnSelectsTheDeviceName)
{
    using namespace cutefish::ui;
    PageFixture page("about");
    std::function<View*(View*)> find = [&](View* view) -> View* {
        if (view->name() == "about:name")
            return view;
        for (const auto& child : view->children()) {
            if (View* found = find(child.get()))
                return found;
        }
        return nullptr;
    };
    auto* field = dynamic_cast<TextField*>(find(page.window->currentPage()));
    ASSERT_NE(field, nullptr);
    ASSERT_FALSE(field->text().empty());
    TestInput::setKeyboardFocus(page.window->window(), true);
    field->setFocus();
    // The name as it is, so nothing is sent to hostnamed.
    TestInput::typeKey(page.window->window(), key::Return);
    EXPECT_TRUE(field->hasFocus());
    EXPECT_EQ(field->selectedText(), field->text());
}

TEST(AboutPage, NamesTheDeviceAsHostnamedDoes)
{
    using settings::DeviceName;
    EXPECT_EQ(DeviceName::chooseName("Kai’s Laptop", "reion-pc", "localhost"), "Kai’s Laptop");
    EXPECT_EQ(DeviceName::chooseName("", "reion-pc", "localhost"), "reion-pc");
    EXPECT_EQ(DeviceName::chooseName("", "", "localhost"), "localhost");
}

TEST(AboutPage, LinksToTheLicenses)
{
    PageFixture page("about");
    EXPECT_EQ(page.count("settings::NavigationItem"), 1);
}

TEST(LicensesPage, ListsTheCompositorNotices)
{
    PageFixture page("about", "licenses");
    EXPECT_NE(page.dump().find("\"page:licenses\""), std::string::npos);
    EXPECT_EQ(page.count("settings::InfoRow"), 4);
    EXPECT_EQ(page.count("settings::Footnote"), 2);
}

TEST(AboutPage, FormatsSizesAsTheQtVersionDid)
{
    using settings::about::formatByteSize;
    EXPECT_EQ(formatByteSize(512), "512 B");
    EXPECT_EQ(formatByteSize(1536), "2 KB");
    EXPECT_EQ(formatByteSize(16.0 * 1024 * 1024 * 1024), "16 GB");
}

TEST(AboutPage, NamesTheComputerModel)
{
    using settings::about::computerModel;
    EXPECT_EQ(computerModel("VENDOR", "Book 13", "AB-1234", ""), "VENDOR Book 13");
    EXPECT_EQ(computerModel("Vendor Inc.", "Vendor Book", "", ""), "Vendor Book");
    EXPECT_EQ(computerModel("Vendor", "To be filled by O.E.M.", "Book 14", ""), "Vendor Book 14");
    EXPECT_EQ(computerModel("System manufacturer", "", "System Product Name", ""), "");
    EXPECT_EQ(computerModel("", "", "", "Board Model B"), "Board Model B");
    EXPECT_EQ(computerModel("Default string", "Default string", "Default string", ""), "");
}

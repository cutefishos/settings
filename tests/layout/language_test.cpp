// SPDX-FileCopyrightText: 2026 Reion Wong <reionwong@gmail.com>
// SPDX-License-Identifier: BSD-3-Clause

#include <gtest/gtest.h>

#include <chrono>
#include <string>
#include <utility>
#include <vector>

#include "language_config.h"
#include "pages/language/language.h"
#include "pages/language/preferredlanguagelist.h"
#include "service_thread.h"
#include "widgets/metrics.h"
#include "ui/keys.h"
#include "ui/layout.h"
#include "ui/test_input.h"
#include "ui/window.h"

using namespace cutefish;
using namespace cutefish::ui;
using namespace settings;
using namespace std::chrono_literals;
using settings::testing::LanguageConfig;
using settings::testing::waitFor;

namespace {

// The preferred language list in a window at 150%, its moves recorded.
struct DragFixture : ::testing::Test
{
    explicit DragFixture(std::vector<std::string> languages = {"en_US", "ja_JP", "de_DE"})
        : config(languages)
        , language(config.path)
    {
        window.setScale(1.5);
        auto root = std::make_unique<Column>();
        root->setMargins({20, 20, 20, 20});
        list = root->addChild<PreferredLanguageList>(language);
        list->onMoveRequested = [this](int from, int to) { moves.emplace_back(from, to); };
        window.setContentView(std::move(root));
        window.resize({520, 440});
        window.grab();
    }

    // The middle of a row's drag handle, in window coordinates: the rows keep
    // the card's padding inside the list.
    PointF handle(int index) const
    {
        const RectF rect = list->geometry();
        return {rect.right() - metrics::cardHorizontalPadding - 16 + 9,
                rect.y + PreferredLanguageList::kRowHeight * (index + 0.5)};
    }
    double rowY(double rows) const { return list->geometry().y + PreferredLanguageList::kRowHeight * rows; }

    PointF startDrag(int index)
    {
        const PointF point = handle(index);
        TestInput::movePointer(window, point);
        TestInput::press(window, point);
        TestInput::movePointer(window, {point.x, point.y + 16});
        EXPECT_TRUE(list->isDragging());
        return point;
    }

    LanguageConfig config;
    Language language;
    Window window;
    PreferredLanguageList* list;
    std::vector<std::pair<int, int>> moves;
};

struct LongDragFixture : DragFixture
{
    LongDragFixture()
        : DragFixture({"en_US", "en_GB", "ja_JP", "ko_KR", "fr_FR", "de_DE", "es_ES", "pt_BR", "ru_RU", "it_IT",
                       "id_ID", "vi_VN"})
    {
    }
};

} // namespace

TEST_F(DragFixture, MovesDown)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, {point.x, rowY(2.5)});
    EXPECT_EQ(list->targetIndex(), 2);
    EXPECT_TRUE(moves.empty());
    TestInput::release(window, {point.x, rowY(2.5)});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{0, 2}}));
    EXPECT_FALSE(list->isDragging());
}

TEST_F(DragFixture, MovesUp)
{
    const PointF point = startDrag(2);
    TestInput::movePointer(window, {point.x, rowY(0.5)});
    TestInput::release(window, {point.x, rowY(0.5)});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{2, 0}}));
}

TEST_F(DragFixture, EscapeCancels)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, {point.x, rowY(2.5)});
    TestInput::typeKey(window, key::Escape);
    EXPECT_FALSE(list->isDragging());
    TestInput::release(window, {point.x, rowY(2.5)});
    EXPECT_TRUE(moves.empty());
}

TEST_F(DragFixture, ReleasingOutsideCancels)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, {list->geometry().x - 10, rowY(2.5)});
    TestInput::release(window, {list->geometry().x - 10, rowY(2.5)});
    EXPECT_TRUE(moves.empty());
}

TEST_F(DragFixture, DroppingInPlaceSavesNothing)
{
    const PointF point = startDrag(0);
    TestInput::movePointer(window, point);
    TestInput::release(window, point);
    EXPECT_TRUE(moves.empty());
}

TEST_F(DragFixture, KeysMoveTheFocusedRow)
{
    TestInput::click(window, handle(1));
    TestInput::typeKey(window, key::Up);
    TestInput::typeKey(window, key::Down);
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{1, 0}, {1, 2}}));
}

TEST_F(LongDragFixture, ScrollsAtTheEdge)
{
    // Seven rows show; the rest scroll.
    EXPECT_EQ(list->geometry().height, PreferredLanguageList::kRowHeight * 7);
    const PointF point = startDrag(0);
    const PointF bottom{point.x, list->geometry().bottom() - 1};
    TestInput::movePointer(window, bottom);
    ASSERT_TRUE(waitFor([&] { return list->scrollY() > PreferredLanguageList::kRowHeight; }, 5s));
    TestInput::release(window, bottom);
    ASSERT_EQ(moves.size(), 1u);
    EXPECT_EQ(moves.front().first, 0);
    EXPECT_GT(moves.front().second, 0);
}

TEST_F(DragFixture, ReleasingAboveMovesToTheTop)
{
    const PointF point = startDrag(2);
    TestInput::movePointer(window, {point.x, rowY(0.5)});
    TestInput::movePointer(window, {point.x, list->geometry().y - 20});
    EXPECT_EQ(list->targetIndex(), 0);
    TestInput::release(window, {point.x, list->geometry().y - 20});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{2, 0}}));
}

TEST_F(DragFixture, ReleasingBelowMovesToTheBottom)
{
    const PointF point = startDrag(0);
    const double below = list->geometry().bottom() + 20;
    TestInput::movePointer(window, {point.x, below});
    EXPECT_EQ(list->targetIndex(), list->count() - 1);
    TestInput::release(window, {point.x, below});
    EXPECT_EQ(moves, (std::vector<std::pair<int, int>>{{0, list->count() - 1}}));
}

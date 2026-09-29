pragma Singleton

import QtQuick 2.12

// Spacing tokens for every page, on a 4 px grid. Pages use the semantic
// names; retune the look here rather than at the use sites.
QtObject {
    // Scale.
    readonly property int xxs: 2
    readonly property int xs: 4
    readonly property int s: 8
    readonly property int m: 12
    readonly property int l: 16
    readonly property int xl: 24
    readonly property int xxl: 32

    readonly property int headerTitlePointSize: 11

    // Page: the scroll area's margins and the rhythm between its sections.
    readonly property int pageMargin: xl
    // Below the floating window header, which already has its own air.
    readonly property int pageTopMargin: l
    readonly property int sectionSpacing: xl
    // Between a section's card and its label or footnote: tight, so the text
    // reads as belonging to that card and not to the neighbouring section.
    readonly property int sectionLabelSpacing: s

    // Card. Section labels and row separators share its horizontal padding
    // so every text edge on a page lines up.
    readonly property int cardHorizontalPadding: l
    readonly property int cardVerticalPadding: m
    readonly property int cardSpacing: m

    // Row inside a card.
    readonly property int rowHeight: 44
    // For a row holding a slider, a combo box or another control taller than a
    // switch: the extra height keeps the control from crowding the row's edges.
    readonly property int controlRowHeight: 52
    readonly property int rowSpacing: m

    // A full-width button standing alone between cards.
    readonly property int buttonHeight: 36

    // Content: controls placed side by side, and text stacked tightly.
    readonly property int controlSpacing: s
    readonly property int textSpacing: xxs

    // Selection tiles and grids of choices inside a card.
    readonly property int tileSpacing: xl

    // Two-column label/control forms.
    readonly property int formColumnSpacing: xl
    readonly property int formRowSpacing: l

    // How far each trailing glyph's drawing stops short of its box's end, as a share of the box.
    readonly property var glyphEndInsets: ({ "check": 1 / 6, "info": 1 / 8, "chevron-right": 5 / 16,
                                             "chevron-down": 3 / 16, "chevron-up": 3 / 16 })
    //! A row's last glyph takes this margin so its drawing, not its box, ends where a switch does.
    function trailingGlyphMargin(name, boxWidth, padding) {
        return -Math.round((glyphEndInsets[name] || 0) * boxWidth + (padding || 0))
    }

    // Sidebar.
    readonly property int sidebarMargin: m
    readonly property int sidebarItemHeight: 36
    // The space above and below the 24 px icon tile, so the tile sits in an even inset.
    readonly property int sidebarItemPadding: (sidebarItemHeight - 24) / 2
    readonly property int sidebarItemSpacing: xxs
    // Above a category label; below it stays tight so it groups with its items.
    readonly property int sidebarCategorySpacing: l

    // Dialogs.
    readonly property int dialogPadding: l
    readonly property int dialogSpacing: l
    readonly property int dialogButtonSpacing: s
    readonly property int dialogWindowButtonsMargin: s
}

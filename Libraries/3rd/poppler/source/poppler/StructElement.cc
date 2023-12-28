//========================================================================
//
// StructElement.cc
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2013, 2014 Igalia S.L.
// Copyright 2014 Luigi Scarso <luigi.scarso@gmail.com>
// Copyright 2014, 2017-2019, 2021, 2023 Albert Astals Cid <aacid@kde.org>
// Copyright 2015 Dmytro Morgun <lztoad@gmail.com>
// Copyright 2018, 2021, 2023 Adrian Johnson <ajohnson@redneon.com>
// Copyright 2018 Klarälvdalens Datakonsult AB, a KDAB Group company, <info@kdab.com>. Work sponsored by the LiMux project of the city of Munich
// Copyright 2018 Adam Reichold <adam.reichold@t-online.de>
//
//========================================================================

#include "StructElement.h"
#include "StructTreeRoot.h"
#include "GlobalParams.h"
#include "UnicodeMap.h"
#include "PDFDoc.h"
#include "Dict.h"

#include <cassert>

class GfxState;

static bool isPlacementName(Object *value)
{
    return value->isName("Block") || value->isName("Inline") || value->isName("Before") || value->isName("Start") || value->isName("End");
}

static bool isWritingModeName(Object *value)
{
    return value->isName("LrTb") || value->isName("RlTb") || value->isName("TbRl");
}

static bool isBorderStyleName(Object *value)
{
    return value->isName("None") || value->isName("Hidden") || value->isName("Dotted") || value->isName("Dashed") || value->isName("Solid") || value->isName("Double") || value->isName("Groove") || value->isName("Ridge")
            || value->isName("Inset") || value->isName("Outset");
}

static bool isTextAlignName(Object *value)
{
    return value->isName("Start") || value->isName("End") || value->isName("Center") || value->isName("Justify");
}

static bool isBlockAlignName(Object *value)
{
    return value->isName("Before") || value->isName("Middle") || value->isName("After") || value->isName("Justify");
}

static bool isInlineAlignName(Object *value)
{
    return value->isName("Start") || value->isName("End") || value->isName("Center");
}

static bool isNumber(Object *value)
{
    return value->isNum();
}

static bool isLineHeight(Object *value)
{
    return value->isName("Normal") || value->isName("Auto") || isNumber(value);
}

static bool isTextDecorationName(Object *value)
{
    return value->isName("None") || value->isName("Underline") || value->isName("Overline") || value->isName("LineThrough");
}

static bool isRubyAlignName(Object *value)
{
    return value->isName("Start") || value->isName("End") || value->isName("Center") || value->isName("Justify") || value->isName("Distribute");
}

static bool isRubyPositionName(Object *value)
{
    return value->isName("Before") || value->isName("After") || value->isName("Warichu") || value->isName("Inline");
}

static bool isGlyphOrientationName(Object *value)
{
    return value->isName("Auto") || value->isName("90") || value->isName("180") || value->isName("270") || value->isName("360") || value->isName("-90") || value->isName("-180");
}

static bool isListNumberingName(Object *value)
{
    return value->isName("None") || value->isName("Disc") || value->isName("Circle") || value->isName("Square") || value->isName("Decimal") || value->isName("UpperRoman") || value->isName("LowerRoman") || value->isName("UpperAlpha")
            || value->isName("LowerAlpha");
}

static bool isFieldRoleName(Object *value)
{
    return value->isName("rb") || value->isName("cb") || value->isName("pb") || value->isName("tv");
}

static bool isFieldCheckedName(Object *value)
{
    return value->isName("on") || value->isName("off") || value->isName("neutral");
}

static bool isTableScopeName(Object *value)
{
    return value->isName("Row") || value->isName("Column") || value->isName("Both");
}

static bool isRGBColor(Object *value)
{
    if (!(value->isArray() && value->arrayGetLength() == 3)) {
        return false;
    }

    bool okay = true;
    for (int i = 0; i < 3; i++) {
        Object obj = value->arrayGet(i);
        if (!obj.isNum()) {
            okay = false;
            break;
        }
        if (obj.getNum() < 0.0 || obj.getNum() > 1.0) {
            okay = false;
            break;
        }
    }

    return okay;
}

static bool isNatural(Object *value)
{
    return (value->isInt() && value->getInt() > 0) || (value->isInt64() && value->getInt64() > 0);
}

static bool isPositive(Object *value)
{
    return value->isNum() && value->getNum() >= 0.0;
}

static bool isNumberOrAuto(Object *value)
{
    return isNumber(value) || value->isName("Auto");
}

static bool isTextString(Object *value)
{
    // XXX: Shall isName() also be checked?
    return value->isString();
}

#define ARRAY_CHECKER(name, checkItem, length, allowSingle, allowNulls)                                                                                                                                                                        \
    static bool name(Object *value)                                                                                                                                                                                                            \
    {                                                                                                                                                                                                                                          \
        if (!value->isArray())                                                                                                                                                                                                                 \
            return allowSingle ? checkItem(value) : false;                                                                                                                                                                                     \
                                                                                                                                                                                                                                               \
        if (length && value->arrayGetLength() != length)                                                                                                                                                                                       \
            return false;                                                                                                                                                                                                                      \
                                                                                                                                                                                                                                               \
        bool okay = true;                                                                                                                                                                                                                      \
        for (int i = 0; i < value->arrayGetLength(); i++) {                                                                                                                                                                                    \
            Object obj = value->arrayGet(i);                                                                                                                                                                                                   \
            if ((!allowNulls && obj.isNull()) || !checkItem(&obj)) {                                                                                                                                                                           \
                okay = false;                                                                                                                                                                                                                  \
                break;                                                                                                                                                                                                                         \
            }                                                                                                                                                                                                                                  \
        }                                                                                                                                                                                                                                      \
        return okay;                                                                                                                                                                                                                           \
    }

ARRAY_CHECKER(isRGBColorOrOptionalArray4, isRGBColor, 4, true, true)
ARRAY_CHECKER(isPositiveOrOptionalArray4, isPositive, 4, true, true)
ARRAY_CHECKER(isPositiveOrArray4, isPositive, 4, true, false)
ARRAY_CHECKER(isBorderStyle, isBorderStyleName, 4, true, true)
ARRAY_CHECKER(isNumberArray4, isNumber, 4, false, false)
ARRAY_CHECKER(isNumberOrArrayN, isNumber, 0, true, false)
ARRAY_CHECKER(isTableHeaders, isTextString, 0, false, false)

// Type of functions used to do type-checking on attribute values
typedef bool (*AttributeCheckFunc)(Object *);

// Maps attributes to their names and whether the attribute can be inherited.
struct AttributeMapEntry
{
    Attribute::Type type;
    const char *name;
    const Object *defval;
    bool inherit;
    AttributeCheckFunc check;
};

struct AttributeDefaults
{
    AttributeDefaults() {}; // needed to support old clang

    Object Inline = Object(objName, "Inline");
    Object LrTb = Object(objName, "LrTb");
    Object Normal = Object(objName, "Normal");
    Object Distribute = Object(objName, "Distribute");
    Object off = Object(objName, "off");
    Object Zero = Object(0.0);
    Object Auto = Object(objName, "Auto");
    Object Start = Object(objName, "Start");
    Object None = Object(objName, "None");
    Object Before = Object(objName, "Before");
    Object Nat1 = Object(1);
};

static const AttributeDefaults attributeDefaults;

#define ATTR_LIST_END                                                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                          \
        Attribute::Unknown, nullptr, nullptr, false, nullptr                                                                                                                                                                                   \
    }

#define ATTR_WITH_DEFAULT(name, inherit, check, defval)                                                                                                                                                                                        \
    {                                                                                                                                                                                                                                          \
        Attribute::name, #name, &attributeDefaults.defval, inherit, check                                                                                                                                                                      \
    }

#define ATTR(name, inherit, check)                                                                                                                                                                                                             \
    {                                                                                                                                                                                                                                          \
        Attribute::name, #name, nullptr, inherit, check                                                                                                                                                                                        \
    }

static const AttributeMapEntry attributeMapCommonShared[] = { ATTR_WITH_DEFAULT(Placement, false, isPlacementName, Inline),
                                                              ATTR_WITH_DEFAULT(WritingMode, true, isWritingModeName, LrTb),
                                                              ATTR(BackgroundColor, false, isRGBColor),
                                                              ATTR(BorderColor, true, isRGBColorOrOptionalArray4),
                                                              ATTR_WITH_DEFAULT(BorderStyle, false, isBorderStyle, None),
                                                              ATTR(BorderThickness, true, isPositiveOrOptionalArray4),
                                                              ATTR_WITH_DEFAULT(Padding, false, isPositiveOrArray4, Zero),
                                                              ATTR(Color, true, isRGBColor),
                                                              ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonBlock[] = { ATTR_WITH_DEFAULT(SpaceBefore, false, isPositive, Zero),
                                                             ATTR_WITH_DEFAULT(SpaceAfter, false, isPositive, Zero),
                                                             ATTR_WITH_DEFAULT(StartIndent, true, isNumber, Zero),
                                                             ATTR_WITH_DEFAULT(EndIndent, true, isNumber, Zero),
                                                             ATTR_WITH_DEFAULT(TextIndent, true, isNumber, Zero),
                                                             ATTR_WITH_DEFAULT(TextAlign, true, isTextAlignName, Start),
                                                             ATTR(BBox, false, isNumberArray4),
                                                             ATTR_WITH_DEFAULT(Width, false, isNumberOrAuto, Auto),
                                                             ATTR_WITH_DEFAULT(Height, false, isNumberOrAuto, Auto),
                                                             ATTR_WITH_DEFAULT(BlockAlign, true, isBlockAlignName, Before),
                                                             ATTR_WITH_DEFAULT(InlineAlign, true, isInlineAlignName, Start),
                                                             ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonInline[] = { ATTR_WITH_DEFAULT(BaselineShift, false, isNumber, Zero),
                                                              ATTR_WITH_DEFAULT(LineHeight, true, isLineHeight, Normal),
                                                              ATTR(TextDecorationColor, true, isRGBColor),
                                                              ATTR(TextDecorationThickness, true, isPositive),
                                                              ATTR_WITH_DEFAULT(TextDecorationType, false, isTextDecorationName, None),
                                                              ATTR_WITH_DEFAULT(GlyphOrientationVertical, true, isGlyphOrientationName, Auto),
                                                              ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonRubyText[] = { ATTR_WITH_DEFAULT(RubyPosition, true, isRubyPositionName, Before), ATTR_WITH_DEFAULT(RubyAlign, true, isRubyAlignName, Distribute), ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonColumns[] = { ATTR_WITH_DEFAULT(ColumnCount, false, isNatural, Nat1), ATTR(ColumnGap, false, isNumberOrArrayN), ATTR(ColumnWidths, false, isNumberOrArrayN), ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonList[] = { ATTR_WITH_DEFAULT(ListNumbering, true, isListNumberingName, None), ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonPrintField[] = { ATTR(Role, false, isFieldRoleName), ATTR_WITH_DEFAULT(checked, false, isFieldCheckedName, off), ATTR(Desc, false, isTextString), ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonTable[] = { ATTR(Headers, false, isTableHeaders), ATTR(Scope, false, isTableScopeName), ATTR(Summary, false, isTextString), ATTR_LIST_END };

static const AttributeMapEntry attributeMapCommonTableCell[] = { ATTR_WITH_DEFAULT(RowSpan, false, isNatural, Nat1), ATTR_WITH_DEFAULT(ColSpan, false, isNatural, Nat1), ATTR_WITH_DEFAULT(TBorderStyle, true, isBorderStyle, None),
                                                                 ATTR_WITH_DEFAULT(TPadding, true, isPositiveOrArray4, Zero), ATTR_LIST_END };

#undef ATTR_WITH_DEFAULT
#undef ATTR

static const AttributeMapEntry *attributeMapAll[] = {
    attributeMapCommonShared, attributeMapCommonBlock,      attributeMapCommonInline, attributeMapCommonRubyText,  attributeMapCommonColumns,
    attributeMapCommonList,   attributeMapCommonPrintField, attributeMapCommonTable,  attributeMapCommonTableCell, nullptr,
};

static const AttributeMapEntry *attributeMapShared[] = {
    attributeMapCommonShared,
    nullptr,
};

static const AttributeMapEntry *attributeMapBlock[] = {
    attributeMapCommonShared,
    attributeMapCommonBlock,
    nullptr,
};

static const AttributeMapEntry *attributeMapInline[] = {
    attributeMapCommonShared,
    attributeMapCommonInline,
    nullptr,
};

static const AttributeMapEntry *attributeMapTableCell[] = {
    attributeMapCommonShared, attributeMapCommonBlock, attributeMapCommonTable, attributeMapCommonTableCell, nullptr,
};

static const AttributeMapEntry *attributeMapRubyText[] = {
    attributeMapCommonShared,
    attributeMapCommonInline,
    attributeMapCommonRubyText,
    nullptr,
};

static const AttributeMapEntry *attributeMapColumns[] = {
    attributeMapCommonShared,
    attributeMapCommonInline,
    attributeMapCommonColumns,
    nullptr,
};

static const AttributeMapEntry *attributeMapList[] = {
    attributeMapCommonShared,
    attributeMapCommonList,
    nullptr,
};

static const AttributeMapEntry *attributeMapTable[] = {
    attributeMapCommonShared,
    attributeMapCommonBlock,
    attributeMapCommonTable,
    nullptr,
};

static const AttributeMapEntry *attributeMapIllustration[] = {
    // XXX: Illustrations may have some attributes from the "shared", "inline",
    //      the "block" sets. This is a loose specification; making it better
    //      means duplicating entries from the sets. This seems good enough...
    attributeMapCommonShared,
    attributeMapCommonBlock,
    attributeMapCommonInline,
    nullptr,
};

// Table mapping owners of attributes to their names.
static const struct OwnerMapEntry
{
    Attribute::Owner owner;
    const char *name;
} ownerMap[] = {
    // XXX: Those are sorted in the owner priority resolution order. If the
    //      same attribute is defined with two owners, the order in the table
    //      can be used to know which one has more priority.
    { Attribute::XML_1_00, "XML-1.00" },     { Attribute::HTML_3_20, "HTML-3.20" }, { Attribute::HTML_4_01, "HTML-4.01" }, { Attribute::OEB_1_00, "OEB-1.00" },
    { Attribute::RTF_1_05, "RTF-1.05" },     { Attribute::CSS_1_00, "CSS-1.00" },   { Attribute::CSS_2_00, "CSS-2.00" },   { Attribute::Layout, "Layout" },
    { Attribute::PrintField, "PrintField" }, { Attribute::Table, "Table" },         { Attribute::List, "List" },           { Attribute::UserProperties, "UserProperties" },
};

static bool ownerHasMorePriority(Attribute::Owner a, Attribute::Owner b)
{
    size_t aIndex, bIndex, i;

    for (i = aIndex = bIndex = 0; i < sizeof(ownerMap) / sizeof(ownerMap[0]); i++) {
        if (ownerMap[i].owner == a) {
            aIndex = i;
        }
        if (ownerMap[i].owner == b) {
            bIndex = i;
        }
    }

    return aIndex < bIndex;
}

// Maps element types to their names and also serves as lookup table
// for additional element type attributes.

enum ElementType
{
    elementTypeUndefined,
    elementTypeGrouping,
    elementTypeInline,
    elementTypeBlock,
};

static const struct TypeMapEntry
{
    StructElement::Type type;
    const char *name;
    ElementType elementType;
    const AttributeMapEntry **attributes;
} typeMap[] = {
    { StructElement::Document, "Document", elementTypeGrouping, attributeMapShared },
    { StructElement::Part, "Part", elementTypeGrouping, attributeMapShared },
    { StructElement::Art, "Art", elementTypeGrouping, attributeMapColumns },
    { StructElement::Sect, "Sect", elementTypeGrouping, attributeMapColumns },
    { StructElement::Div, "Div", elementTypeGrouping, attributeMapColumns },
    { StructElement::BlockQuote, "BlockQuote", elementTypeGrouping, attributeMapInline },
    { StructElement::Caption, "Caption", elementTypeGrouping, attributeMapInline },
    { StructElement::NonStruct, "NonStruct", elementTypeGrouping, attributeMapInline },
    { StructElement::Index, "Index", elementTypeGrouping, attributeMapInline },
    { StructElement::Private, "Private", elementTypeGrouping, attributeMapInline },
    { StructElement::Span, "Span", elementTypeInline, attributeMapInline },
    { StructElement::Quote, "Quote", elementTypeInline, attributeMapInline },
    { StructElement::Note, "Note", elementTypeInline, attributeMapInline },
    { StructElement::Reference, "Reference", elementTypeInline, attributeMapInline },
    { StructElement::BibEntry, "BibEntry", elementTypeInline, attributeMapInline },
    { StructElement::Code, "Code", elementTypeInline, attributeMapInline },
    { StructElement::Link, "Link", elementTypeInline, attributeMapInline },
    { StructElement::Annot, "Annot", elementTypeInline, attributeMapInline },
    { StructElement::Ruby, "Ruby", elementTypeInline, attributeMapRubyText },
    { StructElement::RB, "RB", elementTypeUndefined, attributeMapRubyText },
    { StructElement::RT, "RT", elementTypeUndefined, attributeMapRubyText },
    { StructElement::RP, "RP", elementTypeUndefined, attributeMapShared },
    { StructElement::Warichu, "Warichu", elementTypeInline, attributeMapRubyText },
    { StructElement::WT, "WT", elementTypeUndefined, attributeMapShared },
    { StructElement::WP, "WP", elementTypeUndefined, attributeMapShared },
    { StructElement::P, "P", elementTypeBlock, attributeMapBlock },
    { StructElement::H, "H", elementTypeBlock, attributeMapBlock },
    { StructElement::H1, "H1", elementTypeBlock, attributeMapBlock },
    { StructElement::H2, "H2", elementTypeBlock, attributeMapBlock },
    { StructElement::H3, "H3", elementTypeBlock, attributeMapBlock },
    { StructElement::H4, "H4", elementTypeBlock, attributeMapBlock },
    { StructElement::H5, "H5", elementTypeBlock, attributeMapBlock },
    { StructElement::H6, "H6", elementTypeBlock, attributeMapBlock },
    { StructElement::L, "L", elementTypeBlock, attributeMapList },
    { StructElement::LI, "LI", elementTypeBlock, attributeMapBlock },
    { StructElement::Lbl, "Lbl", elementTypeBlock, attributeMapBlock },
    { StructElement::LBody, "LBody", elementTypeBlock, attributeMapBlock },
    { StructElement::Table, "Table", elementTypeBlock, attributeMapTable },
    { StructElement::TR, "TR", elementTypeUndefined, attributeMapShared },
    { StructElement::TH, "TH", elementTypeUndefined, attributeMapTableCell },
    { StructElement::TD, "TD", elementTypeUndefined, attributeMapTableCell },
    { StructElement::THead, "THead", elementTypeUndefined, attributeMapShared },
    { StructElement::TFoot, "TFoot", elementTypeUndefined, attributeMapShared },
    { StructElement::TBody, "TBody", elementTypeUndefined, attributeMapShared },
    { StructElement::Figure, "Figure", elementTypeUndefined, attributeMapIllustration },
    { StructElement::Formula, "Formula", elementTypeUndefined, attributeMapIllustration },
    { StructElement::Form, "Form", elementTypeUndefined, attributeMapIllustration },
    { StructElement::TOC, "TOC", elementTypeGrouping, attributeMapShared },
    { StructElement::TOCI, "TOCI", elementTypeGrouping, attributeMapShared },
};

//------------------------------------------------------------------------
// Helpers for the attribute and structure type tables
//------------------------------------------------------------------------

static inline const AttributeMapEntry *getAttributeMapEntry(const AttributeMapEntry **entryList, Attribute::Type type)
{
    assert(entryList);
    while (*entryList) {
        const AttributeMapEntry *entry = *entryList;
        while (entry->type != Attribute::Unknown) {
            assert(entry->name);
            if (type == entry->type) {
                return entry;
            }
            entry++;
        }
        entryList++;
    }
    return nullptr;
}

static inline const AttributeMapEntry *getAttributeMapEntry(const AttributeMapEntry **entryList, const char *name)
{
    assert(entryList);
    while (*entryList) {
        const AttributeMapEntry *entry = *entryList;
        while (entry->type != Attribute::Unknown) {
            assert(entry->name);
            if (strcmp(name, entry->name) == 0) {
                return entry;
            }
            entry++;
        }
        entryList++;
    }
    return nullptr;
}

static inline const OwnerMapEntry *getOwnerMapEntry(Attribute::Owner owner)
{
    for (const OwnerMapEntry &entry : ownerMap) {
        if (owner == entry.owner) {
            return &entry;
        }
    }
    return nullptr;
}

static inline const OwnerMapEntry *getOwnerMapEntry(const char *name)
{
    for (const OwnerMapEntry &entry : ownerMap) {
        if (strcmp(name, entry.name) == 0) {
            return &entry;
        }
    }
    return nullptr;
}

static const char *ownerToName(Attribute::Owner owner)
{
    const OwnerMapEntry *entry = getOwnerMapEntry(owner);
    return entry ? entry->name : "UnknownOwner";
}

static Attribute::Owner nameToOwner(const char *name)
{
    const OwnerMapEntry *entry = getOwnerMapEntry(name);
    return entry ? entry->owner : Attribute::UnknownOwner;
}

static inline const TypeMapEntry *getTypeMapEntry(StructElement::Type type)
{
    for (const TypeMapEntry &entry : typeMap) {
        if (type == entry.type) {
            return &entry;
        }
    }
    return nullptr;
}

static inline const TypeMapEntry *getTypeMapEntry(const char *name)
{
    for (const TypeMapEntry &entry : typeMap) {
        if (strcmp(name, entry.name) == 0) {
            return &entry;
        }
    }
    return nullptr;
}

static const char *typeToName(StructElement::Type type)
{
    if (type == StructElement::MCID) {
        return "MarkedContent";
    }
    if (type == StructElement::OBJR) {
        return "ObjectReference";
    }

    const TypeMapEntry *entry = getTypeMapEntry(type);
    return entry ? entry->name : "Unknown";
}

static StructElement::Type nameToType(const char *name)
{
    const TypeMapEntry *entry = getTypeMapEntry(name);
    return entry ? entry->type : StructElement::Unknown;
}

//------------------------------------------------------------------------
// Attribute
//------------------------------------------------------------------------

Attribute::Attribute(GooString &&nameA, Object *valueA) : type(UserProperty), owner(UserProperties), revision(0), name(std::move(nameA)), value(), hidden(false), formatted(nullptr)
{
    assert(valueA);
    value = valueA->copy();
}

Attribute::Attribute(Type typeA, Object *valueA)
    : type(typeA),
      owner(UserProperties), // TODO: Determine corresponding owner from Type
      revision(0),
      name(),
      value(),
      hidden(false),
      formatted(nullptr)
{
    assert(valueA);

    value = valueA->copy();

    if (!checkType()) {
        type = Unknown;
    }
}

Attribute::~Attribute()
{
    delete formatted;
}

const char *Attribute::getTypeName() const
{
    if (type == UserProperty) {
        return name.c_str();
    }

    const AttributeMapEntry *entry = getAttributeMapEntry(attributeMapAll, type);
    if (entry) {
        return entry->name;
    }

    return "Unknown";
}

const char *Attribute::getOwnerName() const
{
    return ownerToName(owner);
}

Object *Attribute::getDefaultValue(Attribute::Type type)
{
    const AttributeMapEntry *entry = getAttributeMapEntry(attributeMapAll, type);
    return entry ? const_cast<Object *>(entry->defval) : nullptr;
}

void Attribute::setFormattedValue(const char *formattedA)
{
    if (formattedA) {
        if (formatted) {
            formatted->Set(formattedA);
        } else {
            formatted = new GooString(formattedA);
        }
    } else {
        delete formatted;
        formatted = nullptr;
    }
}

bool Attribute::checkType(StructElement *element)
{
    // If an element is passed, tighter type-checking can be done.
    if (!element) {
        return true;
    }

    const TypeMapEntry *elementTypeEntry = getTypeMapEntry(element->getType());
    if (elementTypeEntry && elementTypeEntry->attributes) {
        const AttributeMapEntry *entry = getAttributeMapEntry(elementTypeEntry->attributes, type);
        if (entry) {
            if (entry->check && !((*entry->check)(&value))) {
                return false;
            }
        } else {
            // No entry: the attribute is not valid for the containing element.
            return false;
        }
    }

    return true;
}

Attribute::Type Attribute::getTypeForName(const char *name, StructElement *element)
{
    const AttributeMapEntry **attributes = attributeMapAll;
    if (element) {
        const TypeMapEntry *elementTypeEntry = getTypeMapEntry(element->getType());
        if (elementTypeEntry && elementTypeEntry->attributes) {
            attributes = elementTypeEntry->attributes;
        }
    }

    const AttributeMapEntry *entry = getAttributeMapEntry(attributes, name);
    return entry ? entry->type : Unknown;
}

Attribute *Attribute::parseUserProperty(Dict *property)
{
    Object obj, value;
    GooString name;

    obj = property->lookup("N");
    if (obj.isString()) {
        name.Set(obj.getString());
    } else if (obj.isName()) {
        name.Set(obj.getName());
    } else {
        error(errSyntaxError, -1, "N object is wrong type ({0:s})", obj.getTypeName());
        return nullptr;
    }

    value = property->lookup("V");
    if (value.isNull()) {
        error(errSyntaxError, -1, "V object is wrong type ({0:s})", value.getTypeName());
        return nullptr;
    }

    Attribute *attribute = new Attribute(std::move(name), &value);
    obj = property->lookup("F");
    if (obj.isString()) {
        attribute->setFormattedValue(obj.getString()->c_str());
    } else if (!obj.isNull()) {
        error(errSyntaxWarning, -1, "F object is wrong type ({0:s})", obj.getTypeName());
    }

    obj = property->lookup("H");
    if (obj.isBool()) {
        attribute->setHidden(obj.getBool());
    } else if (!obj.isNull()) {
        error(errSyntaxWarning, -1, "H object is wrong type ({0:s})", obj.getTypeName());
    }

    return attribute;
}

//------------------------------------------------------------------------
// StructElement
//------------------------------------------------------------------------

StructElement::StructData::StructData() : altText(nullptr), actualText(nullptr), id(nullptr), title(nullptr), expandedAbbr(nullptr), language(nullptr), revision(0) { }

StructElement::StructData::~StructData()
{
    delete altText;
    delete actualText;
    delete id;
    delete title;
    delete language;
    for (StructElement *element : elements) {
        delete element;
    }
    for (Attribute *attribute : attributes) {
        delete attribute;
    }
}

StructElement::StructElement(Dict *element, StructTreeRoot *treeRootA, StructElement *parentA, RefRecursionChecker &seen) : type(Unknown), treeRoot(treeRootA), parent(parentA), s(new StructData())
{
    assert(treeRoot);
    assert(element);

    parse(element);
    parseChildren(element, seen);
}

StructElement::StructElement(int mcid, StructTreeRoot *treeRootA, StructElement *parentA) : type(MCID), treeRoot(treeRootA), parent(parentA), c(new ContentData(mcid))
{
    assert(treeRoot);
    assert(parent);
}

StructElement::StructElement(const Ref ref, StructTreeRoot *treeRootA, StructElement *parentA) : type(OBJR), treeRoot(treeRootA), parent(parentA), c(new ContentData(ref))
{
    assert(treeRoot);
    assert(parent);
}

StructElement::~StructElement()
{
    if (isContent()) {
        delete c;
    } else {
        delete s;
    }
}

bool StructElement::isBlock() const
{
    const TypeMapEntry *entry = getTypeMapEntry(type);
    return entry ? (entry->elementType == elementTypeBlock) : false;
}

bool StructElement::isInline() const
{
    const TypeMapEntry *entry = getTypeMapEntry(type);
    return entry ? (entry->elementType == elementTypeInline) : false;
}

bool StructElement::isGrouping() const
{
    const TypeMapEntry *entry = getTypeMapEntry(type);
    return entry ? (entry->elementType == elementTypeGrouping) : false;
}

bool StructElement::hasPageRef() const
{
    return pageRef.isRef() || (parent && parent->hasPageRef());
}

bool StructElement::getPageRef(Ref &ref) const
{
    if (pageRef.isRef()) {
        ref = pageRef.getRef();
        return true;
    }

    if (parent) {
        return parent->getPageRef(ref);
    }

    return false;
}

bool StructElement::getStmRef(Ref &ref) const
{
    if (stmRef.isRef()) {
        ref = stmRef.getRef();
        return true;
    }
    return false;
}

const char *StructElement::getTypeName() const
{
    return typeToName(type);
}

const Attribute *StructElement::findAttribute(Attribute::Type attributeType, bool inherit, Attribute::Owner attributeOwner) const
{
    if (isContent()) {
        return parent->findAttribute(attributeType, inherit, attributeOwner);
    }

    if (attributeType == Attribute::Unknown || attributeType == Attribute::UserProperty) {
        return nullptr;
    }

    const Attribute *result = nullptr;

    if (attributeOwner == Attribute::UnknownOwner) {
        // Search for the attribute, no matter who the owner is
        for (unsigned i = 0; i < getNumAttributes(); i++) {
            const Attribute *attr = getAttribute(i);
            if (attributeType == attr->getType()) {
                if (!result || ownerHasMorePriority(attr->getOwner(), result->getOwner())) {
                    result = attr;
                }
            }
        }
    } else {
        // Search for the attribute, with a specific owner
        for (unsigned i = 0; i < getNumAttributes(); i++) {
            const Attribute *attr = getAttribute(i);
            if (attributeType == attr->getType() && attributeOwner == attr->getOwner()) {
                result = attr;
                break;
            }
        }
    }

    if (result) {
        return result;
    }

    if (inherit && parent) {
        const AttributeMapEntry *entry = getAttributeMapEntry(attributeMapAll, attributeType);
        assert(entry);
        // TODO: Take into account special inheritance cases, for example:
        //       inline elements which have been changed to be block using
        //       "/Placement/Block" have slightly different rules.
        if (entry->inherit) {
            return parent->findAttribute(attributeType, inherit, attributeOwner);
        }
    }

    return nullptr;
}

GooString *StructElement::appendSubTreeText(GooString *string, bool recursive) const
{
    if (isContent() && !isObjectRef()) {
        MarkedContentOutputDev mcdev(getMCID(), stmRef);
        const TextSpanArray &spans(getTextSpansInternal(mcdev));

        if (!string) {
            string = new GooString();
        }

        for (const TextSpan &span : spans) {
            string->append(span.getText());
        }

        return string;
    }

    if (!recursive) {
        return nullptr;
    }

    // Do a depth-first traversal, to get elements in logical order
    if (!string) {
        string = new GooString();
    }

    for (unsigned i = 0; i < getNumChildren(); i++) {
        getChild(i)->appendSubTreeText(string, recursive);
    }

    return string;
}

const TextSpanArray &StructElement::getTextSpansInternal(MarkedContentOutputDev &mcdev) const
{
    assert(isContent());

    int startPage = 0, endPage = 0;

    Ref ref;
    if (getPageRef(ref)) {
        startPage = endPage = treeRoot->getDoc()->findPage(ref);
    }

    if (!(startPage && endPage)) {
        startPage = 1;
        endPage = treeRoot->getDoc()->getNumPages();
    }

    treeRoot->getDoc()->displayPages(&mcdev, startPage, endPage, 72.0, 72.0, 0, true, false, false);
    return mcdev.getTextSpans();
}

static StructElement::Type roleMapResolve(Dict *roleMap, const char *name, const char *curName)
{
    // Circular reference
    if (curName && !strcmp(name, curName)) {
        return StructElement::Unknown;
    }

    Object resolved = roleMap->lookup(curName ? curName : name);
    if (resolved.isName()) {
        StructElement::Type type = nameToType(resolved.getName());
        return type == StructElement::Unknown ? roleMapResolve(roleMap, name, resolved.getName()) : type;
    }

    if (!resolved.isNull()) {
        error(errSyntaxWarning, -1, "RoleMap entry is wrong type ({0:s})", resolved.getTypeName());
    }
    return StructElement::Unknown;
}

void StructElement::parse(Dict *element)
{
    Object obj;

    // Type is optional, but if present must be StructElem
    obj = element->lookup("Type");
    if (!obj.isNull() && !obj.isName("StructElem")) {
        error(errSyntaxError, -1, "Type of StructElem object is wrong");
        return;
    }

    // Parent object reference (required).
    const Object &objP = element->lookupNF("P");
    if (!objP.isRef()) {
        error(errSyntaxError, -1, "P object is wrong type ({0:s})", obj.getTypeName());
        return;
    }
    s->parentRef = objP.getRef();

    // Check whether the S-type is valid for the top level
    // element and create a node of the appropriate type.
    obj = element->lookup("S");
    if (!obj.isName()) {
        error(errSyntaxError, -1, "S object is wrong type ({0:s})", obj.getTypeName());
        return;
    }

    // Type name may not be standard, resolve through RoleMap first.
    if (treeRoot->getRoleMap()) {
        type = roleMapResolve(treeRoot->getRoleMap(), obj.getName(), nullptr);
    }

    // Resolving through RoleMap may leave type as Unknown, e.g. for types
    // which are not present in it, yet they are standard element types.
    if (type == Unknown) {
        type = nameToType(obj.getName());
    }

    // At this point either the type name must have been resolved.
    if (type == Unknown) {
        error(errSyntaxError, -1, "StructElem object is wrong type ({0:s})", obj.getName());
        return;
    }

    // Object ID (optional), to be looked at the IDTree in the tree root.
    obj = element->lookup("ID");
    if (obj.isString()) {
        s->id = obj.getString()->copy();
    }

    // Page reference (optional) in which at least one of the child items
    // is to be rendered in. Note: each element stores only the /Pg value
    // contained by it, and StructElement::getPageRef() may look in parent
    // elements to find the page where an element belongs.
    pageRef = element->lookupNF("Pg").copy();

    // Revision number (optional).
    obj = element->lookup("R");
    if (obj.isInt()) {
        s->revision = obj.getInt();
    }

    // Element title (optional).
    obj = element->lookup("T");
    if (obj.isString()) {
        s->title = obj.getString()->copy();
    }

    // Language (optional).
    obj = element->lookup("Lang");
    if (obj.isString()) {
        s->language = obj.getString()->copy();
    }

    // Alternative text (optional).
    obj = element->lookup("Alt");
    if (obj.isString()) {
        s->altText = obj.getString()->copy();
    }

    // Expanded form of an abbreviation (optional).
    obj = element->lookup("E");
    if (obj.isString()) {
        s->expandedAbbr = obj.getString()->copy();
    }

    // Actual text (optional).
    obj = element->lookup("ActualText");
    if (obj.isString()) {
        s->actualText = obj.getString()->copy();
    }

    // Attributes directly attached to the element (optional).
    obj = element->lookup("A");
    if (obj.isDict()) {
        parseAttributes(obj.getDict());
    } else if (obj.isArray()) {
        unsigned attrIndex = getNumAttributes();
        for (int i = 0; i < obj.arrayGetLength(); i++) {
            Object iobj = obj.arrayGet(i);
            if (iobj.isDict()) {
                attrIndex = getNumAttributes();
                parseAttributes(iobj.getDict());
            } else if (iobj.isInt()) {
                const int revision = iobj.getInt();
                // Set revision numbers for the elements previously created.
                for (unsigned j = attrIndex; j < getNumAttributes(); j++) {
                    getAttribute(j)->setRevision(revision);
                }
            } else {
                error(errSyntaxWarning, -1, "A item is wrong type ({0:s})", iobj.getTypeName());
            }
        }
    } else if (!obj.isNull()) {
        error(errSyntaxWarning, -1, "A is wrong type ({0:s})", obj.getTypeName());
    }

    // Attributes referenced indirectly through the ClassMap (optional).
    if (treeRoot->getClassMap()) {
        Object classes = element->lookup("C");
        if (classes.isName()) {
            Object attr = treeRoot->getClassMap()->lookup(classes.getName());
            if (attr.isDict()) {
                parseAttributes(attr.getDict(), true);
            } else if (attr.isArray()) {
                for (int i = 0; i < attr.arrayGetLength(); i++) {
                    unsigned attrIndex = getNumAttributes();
                    Object iobj = attr.arrayGet(i);
                    if (iobj.isDict()) {
                        attrIndex = getNumAttributes();
                        parseAttributes(iobj.getDict(), true);
                    } else if (iobj.isInt()) {
                        // Set revision numbers for the elements previously created.
                        const int revision = iobj.getInt();
                        for (unsigned j = attrIndex; j < getNumAttributes(); j++) {
                            getAttribute(j)->setRevision(revision);
                        }
                    } else {
                        error(errSyntaxWarning, -1, "C item is wrong type ({0:s})", iobj.getTypeName());
                    }
                }
            } else if (!attr.isNull()) {
                error(errSyntaxWarning, -1, "C object is wrong type ({0:s})", classes.getTypeName());
            }
        }
    }
}

StructElement *StructElement::parseChild(const Object *ref, Object *childObj, RefRecursionChecker &seen)
{
    assert(childObj);
    assert(ref);

    StructElement *child = nullptr;

    if (childObj->isInt()) {
        child = new StructElement(childObj->getInt(), treeRoot, this);
    } else if (childObj->isDict("MCR")) {
        /*
         * TODO: The optional StmOwn attribute is not handled.
         */

        Object mcidObj = childObj->dictLookup("MCID");
        if (!mcidObj.isInt()) {
            error(errSyntaxError, -1, "MCID object is wrong type ({0:s})", mcidObj.getTypeName());
            return nullptr;
        }

        child = new StructElement(mcidObj.getInt(), treeRoot, this);

        Object pageRefObj = childObj->dictLookupNF("Pg").copy();
        if (pageRefObj.isRef()) {
            child->pageRef = std::move(pageRefObj);
        }

        const Object &stmObj = childObj->dictLookupNF("Stm");
        if (stmObj.isRef()) {
            child->stmRef = stmObj.copy();
        } else if (!stmObj.isNull()) {
            error(errSyntaxError, -1, "Stm object is wrong type ({0:s})", stmObj.getTypeName());
            delete child;
            return nullptr;
        }

    } else if (childObj->isDict("OBJR")) {
        const Object &refObj = childObj->dictLookupNF("Obj");
        if (refObj.isRef()) {

            child = new StructElement(refObj.getRef(), treeRoot, this);

            Object pageRefObj = childObj->dictLookupNF("Pg").copy();
            if (pageRefObj.isRef()) {
                child->pageRef = std::move(pageRefObj);
            }
        } else {
            error(errSyntaxError, -1, "Obj object is wrong type ({0:s})", refObj.getTypeName());
        }
    } else if (childObj->isDict()) {
        if (!ref->isRef()) {
            error(errSyntaxError, -1, "Structure element dictionary is not an indirect reference ({0:s})", ref->getTypeName());
        } else if (seen.insert(ref->getRef())) {
            child = new StructElement(childObj->getDict(), treeRoot, this, seen);
        } else {
            error(errSyntaxWarning, -1, "Loop detected in structure tree, skipping subtree at object {0:d}:{1:d}", ref->getRefNum(), ref->getRefGen());
        }
    } else {
        error(errSyntaxWarning, -1, "K has a child of wrong type ({0:s})", childObj->getTypeName());
    }

    if (child) {
        if (child->isOk()) {
            appendChild(child);
            if (ref->isRef()) {
                treeRoot->parentTreeAdd(ref->getRef(), child);
            }
        } else {
            delete child;
            child = nullptr;
        }
    }

    return child;
}

void StructElement::parseChildren(Dict *element, RefRecursionChecker &seen)
{
    Object kids = element->lookup("K");
    if (kids.isArray()) {
        for (int i = 0; i < kids.arrayGetLength(); i++) {
            Object obj = kids.arrayGet(i);
            const Object &ref = kids.arrayGetNF(i);
            parseChild(&ref, &obj, seen);
        }
    } else if (kids.isDict() || kids.isInt()) {
        const Object &ref = element->lookupNF("K");
        parseChild(&ref, &kids, seen);
    }
}

void StructElement::parseAttributes(Dict *attributes, bool keepExisting)
{
    Object owner = attributes->lookup("O");
    if (owner.isName("UserProperties")) {
        // In this case /P is an array of UserProperty dictionaries
        Object userProperties = attributes->lookup("P");
        if (userProperties.isArray()) {
            for (int i = 0; i < userProperties.arrayGetLength(); i++) {
                Object property = userProperties.arrayGet(i);
                if (property.isDict()) {
                    Attribute *attribute = Attribute::parseUserProperty(property.getDict());
                    if (attribute && attribute->isOk()) {
                        appendAttribute(attribute);
                    } else {
                        error(errSyntaxWarning, -1, "Item in P is invalid");
                        delete attribute;
                    }
                } else {
                    error(errSyntaxWarning, -1, "Item in P is wrong type ({0:s})", property.getTypeName());
                }
            }
        }
    } else if (owner.isName()) {
        // In this case /P contains standard attributes.
        // Check first if the owner is a valid standard one.
        Attribute::Owner ownerValue = nameToOwner(owner.getName());
        if (ownerValue != Attribute::UnknownOwner) {
            // Iterate over the entries of the "attributes" dictionary.
            // The /O entry (owner) is skipped.
            for (int i = 0; i < attributes->getLength(); i++) {
                const char *key = attributes->getKey(i);
                if (strcmp(key, "O") != 0) {
                    Attribute::Type t = Attribute::getTypeForName(key, this);

                    // Check if the attribute is already defined.
                    if (keepExisting) {
                        bool exists = false;
                        for (unsigned j = 0; j < getNumAttributes(); j++) {
                            if (getAttribute(j)->getType() == t) {
                                exists = true;
                                break;
                            }
                        }
                        if (exists) {
                            continue;
                        }
                    }

                    if (t != Attribute::Unknown) {
                        Object value = attributes->getVal(i);
                        bool typeCheckOk = true;
                        Attribute *attribute = new Attribute(t, &value);

                        if (attribute->isOk() && (typeCheckOk = attribute->checkType(this))) {
                            appendAttribute(attribute);
                        } else {
                            // It is not needed to free "value", the Attribute instance
                            // owns the contents, so deleting "attribute" is enough.
                            if (!typeCheckOk) {
                                error(errSyntaxWarning, -1, "Attribute {0:s} value is of wrong type ({1:s})", attribute->getTypeName(), attribute->getValue()->getTypeName());
                            }
                            delete attribute;
                        }
                    } else {
                        error(errSyntaxWarning, -1, "Wrong Attribute '{0:s}' in element {1:s}", key, getTypeName());
                    }
                }
            }
        } else {
            error(errSyntaxWarning, -1, "O object is invalid value ({0:s})", owner.getName());
        }
    } else if (!owner.isNull()) {
        error(errSyntaxWarning, -1, "O is wrong type ({0:s})", owner.getTypeName());
    }
}

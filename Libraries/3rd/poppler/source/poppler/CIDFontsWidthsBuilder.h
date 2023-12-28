//========================================================================
//
// CIDFontsWidthsBuilder.h
//
// This file is licensed under the GPLv2 or later
//
// Copyright 2023 g10 Code GmbH, Author: Sune Stolborg Vuorela <sune@vuorela.dk>
//========================================================================

#ifndef CIDFontsWidthsBuilder_H
#define CIDFontsWidthsBuilder_H

#include <optional>
#include <vector>
#include <variant>
#include <algorithm>
#include <cassert>

/** Class to help build the widths array as defined in
    pdf standard 9.7.4.3 Glyph Metrcis in CIDFonts in
    ISO 32000-2:2020

    The way to use this is to create a builder, then add all the widths
    and their attached code in order using \ref addWidth and finally call \ref takeSegments

    The resulting value is a list of segments of either \ref ListSegment or
    \ref RangeSegment
    */
class CIDFontsWidthsBuilder
{
public:
    /// Segment that should be encoded as a first index and a list of n number specifying the next n widths
    class ListSegment
    {
    public:
        int first;
        std::vector<int> widths;
    };
    /// Segment that should be encoded as 3 integers, first, last (included) and the width for that group.
    class RangeSegment
    {
    public:
        int first;
        int last;
        int width;
    };
    using Segment = std::variant<RangeSegment, ListSegment>;

    /**
     * Adds a width for a given index.
     *
     * Must be called with ever increasing indices until \ref takeSegments
     * has been called
     */
    void addWidth(int index, int width)
    {
        if (m_currentSegment.m_lastIndex.has_value() && index <= m_currentSegment.m_lastIndex) {
            assert(false); // this is likely a error originating from the user of this code that this function gets called twice with the same or decreasing value.
            return;
        }
        while (!m_currentSegment.accept(index, width)) {
            segmentDone();
        }
    }

    /**
     * \return the resulting segments and resets this font builder
     */
    [[nodiscard]] std::vector<Segment> takeSegments()
    {
        finish();
        auto rv = std::move(m_segments);
        m_segments = {};
        return rv;
    }

private:
    void finish()
    {
        while (m_currentSegment.m_values.size()) {
            segmentDone();
        }
        m_currentSegment = {};
    }
    class SegmentBuilder
    {
        // How many elements at the end has this
        int uniqueElementsFromEnd(int value)
        {
            auto lastDifferent = std::find_if(m_values.rbegin(), m_values.rend(), [value](auto &&element) { return element != value; });
            return std::distance(m_values.rbegin(), lastDifferent);
        }

    public:
        /** Tries to add a index/width combo.
         * If a value is not accepted, caller should
         * build a segment and repeat the accept call.
         *
         * \return if accepted or not
         */
        bool accept(int index, int value)
        {
            if (m_lastIndex.has_value() && m_lastIndex != index - 1) {
                // we have gaps. That's okay. We just need to ensure to finish the segment
                return false;
            }
            if (!m_firstIndex) {
                m_firstIndex = index;
            }
            if (m_values.size() < 4) {
                m_values.push_back(value);
                if (m_values.front() != value) {
                    differentValues = true;
                }
                m_lastIndex = index;
                return true;
            }
            if (!differentValues) {
                if (m_values.back() == value) {
                    m_values.push_back(value);
                    m_lastIndex = index;
                    return true;
                } else {
                    // We need to end a range segment
                    // to start a new segment with different value
                    return false;
                }
            } else {
                if (uniqueElementsFromEnd(value) >= 3) {
                    // We now have at least 3 unique elements
                    // at the end, so we should finish the previous
                    // list segment and then start a range segment
                    return false;
                } else {
                    m_values.push_back(value);
                    m_lastIndex = index;
                    return true;
                }
            }
        }
        /**
         * Builds the segment of the values so far.
         */
        Segment build()
        {
            if (differentValues || m_values.size() < 4) {
                std::vector<int> savedValues;
                if (m_values.size() >= 4) {
                    auto lastDifferent = std::find_if(m_values.rbegin(), m_values.rend(), [value = m_values.back()](auto &&element) { return element != value; });
                    if (std::distance(m_values.rbegin(), lastDifferent) >= 3) {
                        savedValues.push_back(m_values.back());
                        m_values.pop_back();
                        while (m_values.size() && m_values.back() == savedValues.back()) {
                            savedValues.push_back(m_values.back());
                            m_values.pop_back();
                        }
                    }
                }

                ListSegment segment { m_firstIndex.value(), std::move(m_values) };
                if (!savedValues.empty()) {
                    m_firstIndex = m_lastIndex.value() - savedValues.size() + 1;
                } else {
                    m_firstIndex = {};
                    m_lastIndex = {};
                }
                m_values = std::move(savedValues);
                differentValues = false;
                return segment;
            } else {
                auto segment = RangeSegment { m_firstIndex.value(), m_lastIndex.value(), m_values.back() };
                m_values.clear();
                m_firstIndex = {};
                m_lastIndex = {};
                differentValues = false;
                return segment;
            }
        }
        std::vector<int> m_values;
        std::optional<int> m_lastIndex;
        std::optional<int> m_firstIndex;
        bool differentValues = false;
    };
    std::vector<Segment> m_segments;
    SegmentBuilder m_currentSegment;

    void segmentDone() { m_segments.push_back(m_currentSegment.build()); }
};

#endif // CIDFontsWidthsBuilder_H

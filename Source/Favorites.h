/*
    Lumora Synth - polyphonic synthesizer plugin
    Copyright (C) 2026 danielpluta2002-alt

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    SPDX-License-Identifier: AGPL-3.0-or-later
*/


#pragma once

#include <juce_core/juce_core.h>

namespace lumora
{
/** Favourite presets, shared by all plugin instances through a small text file.
    Each line is a key: "F:<factory preset name>" or "U:<user preset file name>". */
class Favorites
{
public:
    explicit Favorites (juce::File storageFile);

    /** The default file, next to the user preset folder. */
    static juce::File defaultFile();

    static juce::String factoryKey (const juce::String& name) { return "F:" + name; }
    static juce::String userKey (const juce::String& name) { return "U:" + name; }
    static bool isUserKey (const juce::String& key) { return key.startsWith ("U:"); }
    static juce::String nameOf (const juce::String& key) { return key.substring (2); }

    /** Re-reads the file, so changes made by other instances show up. */
    void reload();

    bool contains (const juce::String& key) const { return keys.contains (key); }
    const juce::StringArray& getKeys() const noexcept { return keys; }

    /** Adds or removes the key and saves; returns the new favourite state. */
    bool toggle (const juce::String& key);

private:
    void save() const;

    juce::File file;
    juce::StringArray keys;
};
} // namespace lumora

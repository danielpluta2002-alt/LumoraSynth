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


#include "Favorites.h"

namespace lumora
{
Favorites::Favorites (juce::File storageFile) : file (std::move (storageFile))
{
    reload();
}

juce::File Favorites::defaultFile()
{
    return juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
               .getChildFile ("Lumora Synth").getChildFile ("Favorites.txt");
}

void Favorites::reload()
{
    keys.clear();
    if (! file.existsAsFile())
        return;

    juce::StringArray lines;
    file.readLines (lines);
    for (const auto& line : lines)
    {
        const auto key = line.trim();
        if (key.length() > 2 && (key.startsWith ("F:") || key.startsWith ("U:")))
            keys.addIfNotAlreadyThere (key);
    }
}

bool Favorites::toggle (const juce::String& key)
{
    reload();
    const bool nowFavourite = ! keys.contains (key);
    if (nowFavourite)
        keys.add (key);
    else
        keys.removeString (key);

    save();
    return nowFavourite;
}

void Favorites::save() const
{
    file.getParentDirectory().createDirectory();
    file.replaceWithText (keys.joinIntoString ("\n") + (keys.isEmpty() ? "" : "\n"));
}
} // namespace lumora

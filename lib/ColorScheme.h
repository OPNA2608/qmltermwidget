/*
    This source file is part of Konsole, a terminal emulator.

    Copyright 2007-2008 by Robert Knight <robertknight@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
    02110-1301  USA.
*/

#ifndef COLORSCHEME_H
#define COLORSCHEME_H

// Qt
#include <QHash>
#include <QList>
#include <QMetaType>
#include <QIODevice>
#include <QSet>
#include <QSettings>

// Konsole
#include "CharacterColor.h"

class QIODevice;
//class KConfig;

namespace Konsole
{

/**
 * Represents a color scheme for a terminal display.
 *
 * The color scheme includes the palette of colors used to draw the text and character backgrounds
 * in the display and the opacity level of the display background.
 */
class ColorScheme : public QObject
{
    Q_OBJECT

public:
    /**
     * Constructs a new color scheme which is initialised to the default color set
     * for Konsole.
     */
    ColorScheme(QObject *parent = nullptr);
    ~ColorScheme();

    /** Sets the descriptive name of the color scheme. */
    void setDescription(const QString& description);
    /** Returns the descriptive name of the color scheme. */
    QString description() const;

    /** Sets the name of the color scheme */
    void setName(const QString& name);
    /** Returns the name of the color scheme */
    QString name() const;

#if 0
// Implemented upstream - in user apps
    /** Reads the color scheme from the specified configuration source */
    void read(KConfig& config);
    /** Writes the color scheme to the specified configuration source */
    void write(KConfig& config) const;
#endif
    void read(const QString & filename);
    Q_INVOKABLE void write(const QString & filename) const;

    /** Sets a single entry within the color palette. */
    void setColorTableEntry(int index , const ColorEntry& entry);

    /**
     * Copies the color entries which form the palette for this color scheme
     * into @p table.  @p table should be an array with TABLE_COLORS entries.
     *
     * @param table Array into which the color entries for this color scheme
     * are copied.
     * @param randomSeed Color schemes may allow certain colors in their
     * palette to be randomized.  The seed is used to pick the random color.
     */
    void getColorTable(ColorEntry* table, uint randomSeed = 0) const;

    /**
     * Retrieves a single color entry from the table.
     *
     * See getColorTable()
     */
    ColorEntry colorEntry(int index , uint randomSeed = 0) const;

    Q_INVOKABLE QColor getColor(int index) const;
    Q_INVOKABLE void setColor(int index, QColor color);
    Q_SIGNAL void colorChanged(int index);

    /**
     * Convenience method.  Returns the
     * foreground color for this scheme,
     * this is the primary color used to draw the
     * text in this scheme.
     */
    QColor foregroundColor() const;
    /**
     * Convenience method.  Returns the background color for
     * this scheme, this is the primary color used to
     * draw the terminal background in this scheme.
     */
    QColor backgroundColor() const;

    /**
     * Returns true if this color scheme has a dark background.
     * The background color is said to be dark if it has a value of less than 127
     * in the HSV color space.
     */
    bool hasDarkBackground() const;

    /**
     * Sets the opacity level of the display background. @p opacity ranges
     * between 0 (completely transparent background) and 1 (completely
     * opaque background).
     *
     * Defaults to 1.
     *
     * TODO: More documentation
     */
    Q_INVOKABLE void setOpacity(qreal opacity);
    /**
     * Returns the opacity level for this color scheme, see setOpacity()
     * TODO: More documentation
     */
    Q_INVOKABLE qreal opacity() const;
    Q_SIGNAL void opacityChanged();

    /**
     * Enables randomization of the background color.  This will cause
     * the palette returned by getColorTable() and colorEntry() to
     * be adjusted depending on the value of the random seed argument
     * to them.
     */
    void setRandomizedBackgroundColor(bool randomize);

    /** Returns true if the background color is randomized. */
    bool randomizedBackgroundColor() const;

    static QString colorNameForIndex(int index);
    static QString translatedColorNameForIndex(int index);

private:
    // specifies how much a particular color can be randomized by
    class RandomizationRange
    {
    public:
        RandomizationRange() : hue(0) , saturation(0) , value(0) {}

        bool isNull() const
        {
            return ( hue == 0 && saturation == 0 && value == 0 );
        }

        quint16 hue;
        quint8  saturation;
        quint8  value;
    };

    // returns the active color table.  if none has been set specifically,
    // this is the default color table.
    const ColorEntry* colorTable() const;

#if 0
// implemented upstream - user apps
    // reads a single colour entry from a KConfig source
    // and sets the palette entry at 'index' to the entry read.
    void readColorEntry(KConfig& config , int index);
    // writes a single colour entry to a KConfig source
    void writeColorEntry(KConfig& config , const QString& colorName, const ColorEntry& entry,const RandomizationRange& range) const;
#endif
    void readColorEntry(QSettings *s, int index);
    void writeColorEntry(QSettings *s, int index, const ColorEntry& entry) const;

    // sets the amount of randomization allowed for a particular color
    // in the palette.  creates the randomization table if
    // it does not already exist
    void setRandomizationRange( int index , quint16 hue , quint8 saturation , quint8 value );

    QString _description;
    QString _name;
    qreal _opacity;
    ColorEntry* _table; // pointer to custom color table or 0 if the default
                        // color scheme is being used


    static const quint16 MAX_HUE = 340;

    RandomizationRange* _randomTable;   // pointer to randomization table or 0
                                        // if no colors in the color scheme support
                                        // randomization

    static const char* const colorNames[TABLE_COLORS];
    static const char* const translatedColorNames[TABLE_COLORS];

    static const ColorEntry defaultTable[]; // table of default color entries
};

/**
 * A color scheme which uses colors from the standard KDE color palette.
 *
 * This is designed primarily for the benefit of users who are using specially
 * designed colors.
 *
 * TODO Implement and make it the default on systems with specialized KDE
 * color schemes.
 */
class AccessibleColorScheme : public ColorScheme
{
public:
    AccessibleColorScheme();
};

/**
 * Manages the color schemes available for use by terminal displays.
 * See ColorScheme
 */
class ColorSchemeManager : public QObject
{
    Q_OBJECT

public:

    /**
     * Constructs a new ColorSchemeManager and loads the list
     * of available color schemes.
     *
     * The color schemes themselves are not loaded until they are first
     * requested via a call to findColorScheme()
     */
    ColorSchemeManager();
    /**
     * Destroys the ColorSchemeManager and saves any modified color schemes to disk.
     */
    ~ColorSchemeManager();

    /**
     * Returns the default color scheme for Konsole
     */
    const ColorScheme* defaultColorScheme() const;

    /**
     * Returns the color scheme with the given name or 0 if no
     * scheme with that name exists.  If @p name is empty, the
     * default color scheme is returned.
     *
     * The first time that a color scheme with a particular name is
     * requested, the configuration information is loaded from disk.
     */
    Q_INVOKABLE const Konsole::ColorScheme* findColorScheme(const QString& name);

#if 0
    /**
     * Adds a new color scheme to the manager.  If @p scheme has the same name as
     * an existing color scheme, it replaces the existing scheme.
     *
     * TODO - Ensure the old color scheme gets deleted
     */
    void addColorScheme(ColorScheme* scheme);
#endif
    /**
     * Deletes a color scheme.  Returns true on successful deletion or false otherwise.
     */
    bool deleteColorScheme(const QString& name);

    /**
     * Returns a list of the all the available color schemes.
     * This may be slow when first called because all of the color
     * scheme resources on disk must be located, read and parsed.
     *
     * Subsequent calls will be inexpensive.
     */
    QList<const ColorScheme*> allColorSchemes();

    /** Returns the global color scheme manager instance. */
    static ColorSchemeManager* instance();

    /** @brief Loads a custom color scheme under given \em path.
     *
     * The \em path may refer to either KDE 4 .colorscheme or KDE 3
     * .schema file
     *
     * The loaded color scheme is available under the name equal to
     * the base name of the \em path via the allColorSchemes() and
     * findColorScheme() methods after this call if loaded successfully.
     *
     * @param[in] path The path to KDE 4 .colorscheme or KDE 3 .schema.
     * @return Whether the color scheme is loaded successfully.
     */
    Q_INVOKABLE bool loadCustomColorScheme(const QString& path);

    /**
     * @brief Allows to add a custom location of color schemes.
     *
     * @param[in] custom_dir Custom location of color schemes (must end with /).
     */
    void addCustomColorSchemeDir(const QString& custom_dir);

private:
    // loads a color scheme from a KDE 4+ .colorscheme file
    bool loadColorScheme(const QString& path);
    // returns a list of paths of color schemes in the KDE 4+ .colorscheme file format
    QList<QString> listColorSchemes();
    // loads all of the color schemes
    void loadAllColorSchemes();
    // finds the path of a color scheme
    QString findColorSchemePath(const QString& name) const;

    QHash<QString,const ColorScheme*> _colorSchemes;
    QSet<ColorScheme*> _modifiedSchemes;

    bool _haveLoadedAll;

    static const ColorScheme _defaultColorScheme;
};

}

Q_DECLARE_METATYPE(const Konsole::ColorScheme*)

#endif //COLORSCHEME_H

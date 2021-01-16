/**
 * @TODO I'm hardcoding this to ANSI for now, as it's the most common.
 *       Ideally we should have configurable control characters for various
 *       types of terminals that could be emulated via a standard termcap file.
 * @NOTE ANSI escape sequence reference: 
 *       https://en.wikipedia.org/wiki/ANSI_escape_code#Escape_sequences
 *       Note that escape sequences for other terminal types are a thing, and
 *       currently aren't implemented yet as mentioned in the TODO above, eg:
 *       https://en.wikipedia.org/wiki/VT52#Escape_sequences
 */
#include "renderhelpers.hpp"

#include <QByteArray>
#include <QString>
#include <QTextCursor>

#include <string>
#include <iostream>

static constexpr const char escapeString = '\033';
bool inverted{false};

void setColor(QTextCharFormat& format, unsigned char n, bool bg=false)
{
    QColor color;

    if (n < 16)
    {
        // The normal pre-defined typical 16 colors.
        /// @TODO these should probably be configurable.
        static const Qt::GlobalColor colorcodes[] = {
            // The normal pre-defined typical 8 colors.
            Qt::black,
            Qt::darkRed,
            Qt::darkGreen,
            Qt::darkYellow,
            Qt::darkBlue,
            Qt::darkMagenta,
            Qt::darkCyan,
            Qt::lightGray,

            // bold/intense? versions of the normal 8 colors.
            Qt::gray,
            Qt::red,
            Qt::green,
            Qt::yellow,
            Qt::blue,
            Qt::magenta,
            Qt::cyan,
            Qt::white
        };

        color = QColor(colorcodes[n]);
    }
    else if (n < 232)
    {
        // indexed 8-bit rgb color pallete.
        unsigned int index_R = ((n - 16) / 36);
        unsigned char r = index_R > 0 ? 55 + index_R * 40 : 0;
        unsigned int index_G = (((n - 16) % 36) / 6);
        unsigned char g = index_G > 0 ? 55 + index_G * 40 : 0;
        unsigned int index_B = ((n - 16) % 6);
        unsigned char b = index_B > 0 ? 55 + index_B * 40 : 0;

        color = QColor(r, g, b);
    }
    else
    {
        // grayscale pallete.
        unsigned char g = (n - 232) * 10 + 8;
        color = QColor(g, g, g);
    }

    if (bg)
        format.setBackground(color);
    else
        format.setForeground(color);
}

QString parseNumber(const QString& input, QString::const_iterator& it)
{
    QString result;
    while (it != input.cend())
    {
        const auto currentCharacter = *it;
        if (!currentCharacter.isNumber()) break;
        result += currentCharacter;
        ++it;
    }
    return result;
}

void parseSGR(
    std::vector<unsigned char>& args,
    const QString& input,
    QString::const_iterator& it,
    QTextCharFormat& format,
    const QTextCharFormat& defaultFormat,
    QTextCursor& cursor)
{
    if (args.empty()) return;
    for (auto it = args.cbegin(); it != args.cend(); ++it)
    {
        const auto arg = *it;
        switch(arg)
        {
            /// @TODO A whole bunch of unimplemented SGR codes are unimplemented 
            ///       yet (eg: blink or font switching)
            case 0: // Reset.
                format = defaultFormat;
                break;
            case 1: // Bold.
                format.setFontWeight(QFont::Bold);
                break;
            case 2: // Light.
                format.setFontWeight(QFont::Light);
                break;
            case 3: // Italic.
                format.setFontItalic(true);
                break;
            case 4: // Underline.
                /// @TODO Underline style should be configurable?
                format.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                break;
            case 7: // Reverse video (invert).
                if (!inverted)
                {
                    const auto fg = format.foreground();
                    const auto bg = format.background();
                    format.setForeground(bg);
                    format.setBackground(fg);
                    inverted = true;
                }
                break;
            case 21: // Double underline (or bold off?)
                format.setUnderlineStyle(QTextCharFormat::WaveUnderline);
                break;
            case 22: // Normal weight.
                format.setFontWeight(QFont::Normal);
                break;
            case 23: // Not italic.
                format.setFontItalic(false);
                break;
            case 24: // Not underlined.
                format.setFontUnderline(QTextCharFormat::NoUnderline);
                break;
            case 27: // Not inverted.
                if (inverted)
                {
                    const auto fg = format.foreground();
                    const auto bg = format.background();
                    format.setForeground(bg);
                    format.setBackground(fg);
                    inverted = false;
                }
                break;
            case 29: // Not crossed out.
                format.setFontStrikeOut(false);
                break;
            case 38: // Set foreground (RGB)
                if (args.size() > 2)
                {
                    ++it;
                    const auto colMode = *it;
                    if (colMode == 5)
                    {
                        ++it;
                        const auto colNum = *it;
                        if (colNum == 124)
                        {
                            setColor(format, colNum);
                        }
                        setColor(format, colNum);
                    }
                    else if (colMode == 2)
                    {
                        ++it;
                        if (args.size() >= 4)
                        {
                            const auto red = *it;
                            ++it;
                            const auto green = *it;
                            ++it;
                            const auto blue = *it;
                            format.setForeground(QColor(red, green, blue));
                        }
                    }
                }
                break;
            case 39: // Default foreground color.
                format.setForeground(defaultFormat.foreground());
                break;
            case 48: // Set background (RGB)
                if (args.size() > 2)
                {
                    ++it;
                    const auto colMode = *it;
                    if (colMode == 5)
                    {
                        ++it;
                        const auto colNum = *it;
                        setColor(format, colNum, true);
                    }
                    else if (colMode == 2)
                    {
                        ++it;
                        if (args.size() >= 4)
                        {
                            const auto red = *it;
                            ++it;
                            const auto green = *it;
                            ++it;
                            const auto blue = *it;
                            format.setBackground(QColor(red, green, blue));
                        }
                    }
                }
                break;
            case 49: // Default background color.
                format.setBackground(defaultFormat.background());
                break;
            default:
                // foreground, background and their bright equivalents
                if (arg >= 30 && arg < 38)
                    setColor(format, arg - 30);
                else if (arg >= 40 && arg < 48)
                    setColor(format, arg - 40, true);
                else if (arg >= 90 && arg < 98)
                    setColor(format, arg - 90 + 8);
                else if (arg >= 100 && arg < 108)
                    setColor(format, arg - 100 + 8, true);

                break;
        }
    }
}

std::vector<unsigned char> parseNumericArguments(const QString& input, QString::const_iterator& it)
{
    std::vector<unsigned char> result;
    while (it != input.end())
    {
        const auto currentCharacter = *it;
        const auto numStr = parseNumber(input, it);
        if (numStr.isEmpty())
        {
            if (!(currentCharacter == ' ' || currentCharacter == ';'))
            {
                break;
            }
        }
        else
        {
            result.emplace_back(numStr.toShort());
            continue;
        }
        ++it;
    }
    return result;
}

void parseCSI(
    const QString& input,
    QString::const_iterator& it,
    QTextCharFormat& format,
    const QTextCharFormat& defaultFormat,
    QTextCursor& cursor)
{
    std::vector<unsigned char> numericArguments = parseNumericArguments(input, it);
    char numericArgument = numericArguments.empty() ? 1 : numericArguments[0];
    if (it != input.cend())
    {
        const auto code = (*it).unicode();
        switch(code)
        {
            case 'A': // cursor up
                cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, numericArgument);
                break;
            case 'B': // cursor down
                cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, numericArgument);
                break;
            case 'C': // cursor forward
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, numericArgument);
                break;
            case 'D': // cursor back
                cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, numericArgument);
                break;
            case 'E': // cursor next line
                cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, numericArgument);
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                break;
            case 'F': // cursor previous line
                cursor.movePosition(QTextCursor::Up, QTextCursor::MoveAnchor, numericArgument);
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                break;
            case 'G': // cursor horizontal absolute position set
                cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::MoveAnchor);
                cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, numericArgument);
                break;
            case 'H': // Set cursor position ( row;col )
                if (numericArguments.size() > 1)
                {
                    cursor.setPosition(0, QTextCursor::MoveAnchor);
                    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, numericArguments[0]);
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, numericArguments[1]);
                }
                break;
            case 'J': // Erase in display
                /// @TODO ANSI escape: implement erase in display.
                break;
            case 'K': // Erase in line
                /// @TODO ANSI escape: implement erase in line.
                break;
            case 'S': // Scroll up
                /// @TODO ANSI escape: implement scroll up.
                break;
            case 'T': // Scroll down
                /// @TODO ANSI escape: implement scroll down.
                break;
            case 'f': // Horizontal vertical position
                if (numericArguments.size() > 1)
                {
                    cursor.setPosition(0, QTextCursor::MoveAnchor);
                    cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, numericArguments[0]);
                    cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, numericArguments[1]);
                }
                break;
            case 'm': // SGR
                // CSI m is treated as CSI 0 m. (and thus, SGR 0 m)
                if (numericArguments.size() == 1) numericArguments.insert(numericArguments.begin(), 0);
                parseSGR(numericArguments, input, it, format, defaultFormat, cursor);
                break;
            default:
                // Stuff we ignore: CSI 5i, CSI 4i, CSI 6n.
                // We do care about SGR (CSI n <m>)
                break;
        }
    }
}

void RenderEscapeCodes(const QByteArray &input, const QTextCharFormat& format, QTextCursor& cursor)
{
    auto textFormat = format;
    const auto tokens = input.split(escapeString);
    const auto inputString = QString::fromUtf8(input);
    for (QString::const_iterator it = inputString.cbegin(); it != inputString.cend(); ++it)
    {
        const auto currentCharacter = *it;;
        if (currentCharacter == escapeString)
        {
            it++;
            const auto escSequence = *it;
            if (escSequence == "[")
            {
                it++;
                parseCSI(input, it, textFormat, format, cursor);
            }
        }
        else
        {
            cursor.insertText(currentCharacter, textFormat);
        }
    }
}


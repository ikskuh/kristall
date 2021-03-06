#include "kristalltextbrowser.hpp"

#include "kristall.hpp"

#include <QMouseEvent>
#include <QScroller>
#include <QTouchDevice>
#include <QRegularExpression>
#include <QLineEdit>

const Qt::CursorShape KristallTextBrowser::NORMAL_CURSOR = Qt::IBeamCursor;

KristallTextBrowser::KristallTextBrowser(QWidget *parent) :
    QTextBrowser(parent)
{
    connect(this, &QTextBrowser::anchorClicked, this, &KristallTextBrowser::on_anchorClicked);

    // Enable touch scrolling on touchscreen devices
    for (int i = 0; i < QTouchDevice::devices().length(); ++i)
    {
        if (QTouchDevice::devices()[i]->type() == QTouchDevice::TouchScreen)
        {
            this->viewport()->setAttribute(Qt::WA_AcceptTouchEvents);
            QScroller::grabGesture(this, QScroller::LeftMouseButtonGesture);
            break;
        }
    }
}

void KristallTextBrowser::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::MiddleButton) {
        // Fake a middle-click event here
        QMouseEvent fake_event {
            event->type(),
            event->localPos(),
            Qt::LeftButton,
            event->buttons(),
            event->modifiers()
        };

        this->signal_new_tab = true;
        QTextBrowser::mouseReleaseEvent(&fake_event);
    } else {
        this->signal_new_tab = event->modifiers().testFlag(Qt::ControlModifier);
        QTextBrowser::mouseReleaseEvent(event);
    }
}

void KristallTextBrowser::on_anchorClicked(const QUrl &url)
{
    emit this->anchorClicked(url, this->signal_new_tab);
}


void KristallTextBrowser::mouseMoveEvent(QMouseEvent *event)
{
    QTextBrowser::mouseMoveEvent(event);
    this->updateCursor();
}

void KristallTextBrowser::setDefaultCursor(const QCursor &cur)
{
    this->wanted_cursor = cur;
    this->updateCursor();
}

void KristallTextBrowser::focusInEvent(QFocusEvent *event)
{
    QTextBrowser::focusInEvent(event);
    this->updateCursor();
}

void KristallTextBrowser::updateCursor()
{
    // This slight hack allows us to set the "default" cursor,
    // (i.e when not hovering over links) We need to do this
    // because QTextBrowser for some reason resets viewport cursor
    // to ArrowCursor after we hover over links
    const QCursor& cur = this->viewport()->cursor();
    if (cur.shape() != this->wanted_cursor.shape() &&
        cur.shape() != Qt::PointingHandCursor)
    {
        this->viewport()->setCursor(wanted_cursor);
    }
}

void KristallTextBrowser::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier &&
        event->key() == Qt::Key_C) {
        this->betterCopy();
    } else {
        QTextBrowser::keyPressEvent(event);
    }
}

void KristallTextBrowser::keyReleaseEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier &&
        event->key() == Qt::Key_C) {
        // Eat the event
    } else {
        QTextBrowser::keyReleaseEvent(event);
    }
}


void KristallTextBrowser::betterCopy()
{
    // Our own implementation of a copy. All we need to do is get the
    // selected text, adjust it if needed, and copy it.
    //
    // The main adjustment we make here is stripping "fancy" quotes from the text,
    // as these quotes are usually an annoyance when copying text.
    //
    // There is a little trick here though: if a user selects a piece of text
    // which consists only of whitespace and fancy quotes
    // the fancy quotes will be included in the copy.
    //
    // In all other cases the quotes will be replaced. This can be said to be
    // a usability feature for the few people that may want to actually copy
    // the quotes themselves.

    QTextCursor cursor = QTextBrowser::textCursor();
    QString text = cursor.selectedText();

    if (text.isEmpty()) return;

    // Check if text only consists of fancy quotes:
    static const QRegularExpression REGEX_ONLY_QUOTES(R"(^[\s“”‘’]+$)",
        QRegularExpression::CaseInsensitiveOption);
    if (text.contains(REGEX_ONLY_QUOTES))
    {
        // Copy the original text.
        kristall::clipboard->setText(text);
        return;
    }

    // Replace fancy quotes
    static const QRegularExpression REGEX_QUOTES_D("(“|”)"), REGEX_QUOTES_S("(‘|’)");
    text.replace(REGEX_QUOTES_D, "\"").replace(REGEX_QUOTES_S, "'");

    // From docs:
    // "If the selection obtained from an editor spans a line break, the text will
    // contain a Unicode U+2029 paragraph separator character instead of a newline \n character"
    // Hence, we replace with \r\n on Win$hit, and \n on all other platforms
#ifdef Q_OS_WIN32
    text.replace(QChar(0x2029), "\r\n");
#else
    text.replace(QChar(0x2029), "\n");
#endif

    kristall::clipboard->setText(text);
}

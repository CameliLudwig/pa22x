#ifndef QTCOMPAT_H
#define QTCOMPAT_H

#include <QTextStream>
#include <QFontMetrics>
#include <QtGlobal>

namespace QtCompat {

inline void setStreamUtf8(QTextStream& stream)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    stream.setCodec("UTF-8");
#else
    Q_UNUSED(stream);
#endif
}

inline int fontWidth(const QFontMetrics& fm, const QString& text)
{
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    return fm.width(text);
#else
    return fm.horizontalAdvance(text);
#endif
}

} // namespace QtCompat

#endif // QTCOMPAT_H

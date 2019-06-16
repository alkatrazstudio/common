/****************************************************************************}
{ qtnotify.cpp - notification system                                         }
{                                                                            }
{ Copyright (c) 2018 Alexey Parfenov <zxed@alkatrazstudio.net>               }
{                                                                            }
{ This library is free software: you can redistribute it and/or modify it    }
{ under the terms of the GNU General Public License as published by          }
{ the Free Software Foundation, either version 3 of the License,             }
{ or (at your option) any later version.                                     }
{                                                                            }
{ This library is distributed in the hope that it will be useful,            }
{ but WITHOUT ANY WARRANTY; without even the implied warranty of             }
{ MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU           }
{ General Public License for more details: https://gnu.org/licenses/gpl.html }
{****************************************************************************/

// GTK headers cannot be loaded after Qt headers
#include <libnotify/notify.h>

#include "qtnotify.h"

#include <QMutexLocker>
#include <QGuiApplication>
#include <QSharedPointer>
#include <QIcon>
#include <QPainter>

static constexpr int ICON_SIZE = 128;

//
// PRIVATE
//

class Impl : public QSharedData {
protected:
    using NotificationPtr = QSharedPointer<NotifyNotification>;

    static void setAppIcon(NotificationPtr n)
    {
        static QMutex mutex;
        QMutexLocker lock(&mutex);

        static QSharedPointer<GdkPixbuf> icon;
        static qint64 iconCacheKey = 0;

        QIcon i = qGuiApp->windowIcon();

        if(i.cacheKey() != iconCacheKey)
        {
            QImage i = qGuiApp->windowIcon()
                       .pixmap(ICON_SIZE, ICON_SIZE)
                       .toImage();
            switch(i.format())
            {
                case QImage::Format_ARGB32:
                case QImage::Format_ARGB32_Premultiplied:
                    break;

                default:
                    i = i.convertToFormat(QImage::Format_ARGB32);
            }
            if(!i.isNull() && i.byteCount() && !i.size().isEmpty())
            {
                guchar* bits = new guchar[static_cast<size_t>(i.byteCount())];
                memcpy(bits, i.constBits(), static_cast<size_t>(i.byteCount()));
                icon.reset(gdk_pixbuf_new_from_data(
                    bits,
                    GDK_COLORSPACE_RGB,
                    i.hasAlphaChannel(),
                    i.pixelFormat().bitsPerPixel() / i.pixelFormat().channelCount(),
                    i.width(),
                    i.height(),
                    i.bytesPerLine(),
                    [](guchar *pixels, gpointer data){
                        Q_UNUSED(data);
                        delete[] pixels;
                    },
                    nullptr), g_object_unref);
            }
            notify_notification_set_image_from_pixbuf(n.data(), icon.data());
            iconCacheKey = i.cacheKey();
        }
    }

    static void init()
    {
        static QMutex mutex;
        QMutexLocker lock(&mutex);

        if(!notify_is_initted())
            notify_init(qGuiApp->applicationDisplayName().toUtf8().constData());
    }

public:
    explicit Impl(bool isCritical = false)
    {
        init();
        n.reset(notify_notification_new("", "", nullptr), g_object_unref);
        if(isCritical)
        {
            notify_notification_set_urgency(n.data(), NOTIFY_URGENCY_CRITICAL);
            notify_notification_set_timeout(n.data(), NOTIFY_EXPIRES_NEVER);
        }
    }

    void show(const QString &title, const QString &body)
    {
        QMutexLocker lock(&mutex);
        setAppIcon(n);
        notify_notification_update(n.data(), title.toUtf8().constData(), body.toUtf8().constData(), nullptr);
        notify_notification_show(n.data(), nullptr);
    }

    void show(const QString &body)
    {
        show(qGuiApp->applicationDisplayName(), body);
    }

    static Impl& global()
    {
        static Impl instance;
        return instance;
    }

protected:
    NotificationPtr n;
    QMutex mutex;
};

//
// PUBLIC
//

namespace Notification {
void show(const QString &title, const QString &body)
{
    Impl::global().show(title, body);
}

void show(const QString &body)
{
    Impl::global().show(body);
}

void showDetached(const QString &title, const QString &body, bool isCritical)
{
    Impl n(isCritical);
    n.show(title, body);
}

void showDetached(const QString &body, bool isCritical)
{
    Impl n(isCritical);
    n.show(body);
}
}

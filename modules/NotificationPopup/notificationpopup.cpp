/****************************************************************************}
{ notificationpopup.cpp - notification popup                                 }
{                                                                            }
{ Copyright (c) 2017 Alexey Parfenov <zxed@alkatrazstudio.net>               }
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

#include "notificationpopup.h"

#include <QScreen>
#include <QDesktopWidget>
#include <QApplication>
#include <QTimer>

bool NotificationPopup::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched)
    if(event->type() == QEvent::MouseButtonPress)
    {
        timer->stop();
        root->hide();
        return true;
    }
    else
    {
        return false;
    }
}

NotificationPopup::NotificationPopup(QObject *parent, QMainWindow *mainWindow) : QObject(parent)
{
    this->mainWindow = mainWindow;

    QFont f;
    f.setPointSize(12);
    QFontMetrics m(f);
    int charWidth = m.averageCharWidth();
    int icoSize = charWidth * 5;
    int minWidth = charWidth * 40;

    QPalette p = QApplication::palette();
    p.setColor(QPalette::Window, p.color(QPalette::ToolTipBase));
    p.setColor(QPalette::WindowText, p.color(QPalette::ToolTipText));

    root = new QWidget(mainWindow, Qt::ToolTip);
    root->installEventFilter(this);
    root->setAttribute(Qt::WA_ShowWithoutActivating);
    root->setPalette(p);

    hLayout = new QHBoxLayout(root);
    hLayout->setSizeConstraint(QLayout::SetFixedSize);
    root->setLayout(hLayout);
    hLayout->addWidget(icoLabel = new QLabel(), 0, Qt::AlignTop);
    hLayout->addLayout(vLayout = new QVBoxLayout(), 1);
    vLayout->addWidget(headerLabel = new QLabel());
    vLayout->addWidget(textLabel = new QLabel(), 1);
    vLayout->setSizeConstraint(QLayout::SetFixedSize);

    icoLabel->resize(icoSize, icoSize);
    icoLabel->setScaledContents(true);
    icoLabel->setMaximumHeight(icoSize);
    icoLabel->setMaximumWidth(icoSize);
    headerLabel->setFont(f);
    headerLabel->setPalette(p);
    headerLabel->setMinimumWidth(minWidth);
    textLabel->setFont(f);
    textLabel->setPalette(p);
    textLabel->setWordWrap(true);
    textLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    textLabel->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    textLabel->setMinimumWidth(minWidth);

    timer = new QTimer();
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, [this]{
        root->hide();
    });
}

void NotificationPopup::setIcon(const QIcon &icon)
{
    qreal scaleRatio = QApplication::desktop()->devicePixelRatioF();
    QPixmap p = icon.pixmap(icoLabel->size() * scaleRatio);
    icoLabel->setPixmap(p);
}

NotificationPopup::~NotificationPopup()
{
    delete timer;
    if(!mainWindow) // if root had parent then do not try to delete it here
        delete root;
}

void NotificationPopup::show(const QString &title, const QString &message, int millisecondsTimeoutHint, const QSystemTrayIcon* tray)
{
    headerLabel->setText(title);
    textLabel->setText(message);

    const QList<QScreen*> screens = QGuiApplication::screens();
    const QScreen* screen = screens.first();
    QRect fullRect = screen->geometry();
    QRect availRect = screen->availableGeometry();
    QPoint fullCenter = fullRect.center();
    QRect trayRect = tray ? tray->geometry() : QRect();
    if(trayRect.size().isEmpty())
    {
        // guess the corner in which the system tray icon may be
        int x = ((availRect.left() - fullRect.left()) > (fullRect.right() - availRect.right()))
                ? availRect.left() : availRect.right();
#ifdef Q_OS_OSX
        int y = availRect.top();
#else
        int y = ((availRect.top() - fullRect.top()) > (fullRect.bottom() - availRect.top()))
                ? availRect.top() : availRect.bottom();
#endif
        trayRect = QRect(x, y, 0, 0);
    }
    QPoint trayCenter = trayRect.center();
    bool isRight = trayCenter.x() > fullCenter.x();
    bool isBottom = trayCenter.y() > fullCenter.y();

    timer->start(millisecondsTimeoutHint);

    root->updateGeometry(); // widget geometry is only updated on next event loop cycle
    QTimer::singleShot(0, [trayRect, availRect, isRight, isBottom, this]{
        root->show(); // need to show it first because otherwise
                      // some measurements will be incorrect
        int w = root->width();
        int h = root->height();

        int left = isRight
                ? qMin(trayRect.left(), availRect.right() - w)
                : qMax(trayRect.right() - w, availRect.left());
        int top = isBottom
                ? qMin(trayRect.top(), availRect.bottom() - h)
                : qMax(trayRect.bottom() - h, availRect.top());

        root->move(left, top);
    });
}

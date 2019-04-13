/****************************************************************************}
{ appsettings.cpp - hassle-free app settings manager                         }
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

#include "appsettings.h"

#ifdef QT_QML_LIB
#include <QtQml/qqml.h>
#endif

AppSettings::AppSettings(QObject* parent) : QSettings (parent)
{
    setIniCodec("UTF-8");
    setFallbacksEnabled(false);
}

unsigned int AppSettings::getUInt(const QString &key, unsigned int def) const
{
    bool ok;
    unsigned int result = value(key, def).toUInt(&ok);
    if(!ok)
        result = def;
    return result;
}

int AppSettings::getInt(const QString &key, int def) const
{
    bool ok;
    int result = value(key, def).toInt(&ok);
    if(!ok)
        result = def;
    return result;
}

QString AppSettings::getString(const QString &key, const QString &def) const
{
    return value(key, def).toString();
}

QString AppSettings::getString(const QString &key) const
{
    return value(key).toString();
}

QColor AppSettings::getColor(const QString &key, const QColor &def) const
{
    QColor color = getColor(key);
    return color.isValid() ? color : def;
}

QColor AppSettings::getColor(const QString &key) const
{
    if(!contains(key))
        return QColor();
    QString str = getString(key);
    return QColor(str);
}

void AppSettings::setColor(const QString &key, const QColor &val)
{
    setValue(key, val.name(QColor::HexArgb));
}

bool AppSettings::getBool(const QString &key, bool def) const
{
    return value(key, def).toBool();
}

#ifdef QT_QML_LIB
void AppSettings::registerInQml()
{
    qmlRegisterType<AppSettings>("net.alkatrazstudio.AppSettingsImpl", 1, 0, "AppSettingsImpl");
}
#endif

void AppSettings::setCategory(const QString &cat)
{
    if(!group().isEmpty())
        endGroup();
    beginGroup(cat);
}

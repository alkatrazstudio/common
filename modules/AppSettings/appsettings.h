/****************************************************************************}
{ appsettings.h - hassle-free app settings manager                           }
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

#pragma once

#include <QObject>
#include <QSettings>
#include <QColor>

class AppSettings : public QSettings
{
    Q_OBJECT

public:
    explicit AppSettings(QObject* parent = nullptr);

    Q_INVOKABLE unsigned int getUInt(const QString& key, unsigned int def = 0) const;
    Q_INVOKABLE int getInt(const QString& key, int def = 0) const;

    Q_INVOKABLE QString getString(const QString& key, const QString& def) const;
    Q_INVOKABLE QString getString(const QString& key) const;

    Q_INVOKABLE QColor getColor(const QString& key, const QColor& def) const;
    Q_INVOKABLE QColor getColor(const QString& key) const;
    Q_INVOKABLE void setColor(const QString& key, const QColor& val);

    Q_INVOKABLE bool getBool(const QString& key, bool def = false) const;

    void beginGroup(const QString &prefix)
        {QSettings::beginGroup(prefix); emit onCategoryChanged();}
    Q_INVOKABLE void setValue(const QString &key, const QVariant &value){QSettings::setValue(key, value);}
    Q_INVOKABLE QVariant value(const QString &key, const QVariant &defaultValue = QVariant()) const
        {return QSettings::value(key, defaultValue);}
    Q_INVOKABLE void sync() {QSettings::sync();}

    Q_PROPERTY(QString category READ group WRITE setCategory NOTIFY onCategoryChanged)

#ifdef QT_QML_LIB
    static void registerInQml();
#endif

protected:
    void setCategory(const QString& cat);

signals:
    void onCategoryChanged();
};

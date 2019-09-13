/****************************************************************************}
{ qiodevicehelper.cpp - shortcut functions for QIODevice children            }
{                                                                            }
{ Copyright (c) 2011 Alexey Parfenov <zxed@alkatrazstudio.net>               }
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

#include "qiodevicehelper.h"

QFileEx::QFileEx(): QIODeviceHelper<QFile>()
  ,doRestore(true)
  ,backupSuffix("~")
{
}

QFileEx::QFileEx(const QString &filename): QIODeviceHelper<QFile>()
  ,doRestore(true)
  ,backupSuffix("~")
{
    setFileName(filename);
}

QFileEx::~QFileEx()
{
    close();
}

bool QFileEx::moveToBackup()
{
    QString backupFilename(fileName()+backupSuffix);
    if(!QFile::remove(backupFilename))
        return false;
    if(!QFile::rename(fileName(), backupFilename))
        return false;
    return true;
}

bool QFileEx::openWithBackup(OpenMode mode)
{
    QString backupFilename(fileName()+backupSuffix);
    QFile::remove(backupFilename);
    if(QFile::rename(fileName(), backupFilename))
        doRestore = true;
    if(!((QIODevice*)this)->open(mode))
    {
        if(doRestore)
        {
            QFile::remove(fileName());
            QFile::rename(backupFilename, fileName());
            doRestore = false;
        }
        return false;
    }
    return true;
}

bool QFileEx::restoreFromBackup()
{
    close(false);
    QString backupFilename(fileName()+backupSuffix);
    QFile::remove(fileName());
    return QFile::rename(backupFilename, fileName());
}

void QFileEx::removeBackup()
{
    QString backupFilename(fileName()+backupSuffix);
    QFile::remove(backupFilename);
}

void QFileEx::close(bool doRemoveBackupOnClose)
{
    if(doRestore)
    {
        doRestore = false;
        if(doRemoveBackupOnClose)
            removeBackup();
    }
    QIODeviceHelper<QFile>::close();
}

bool QFileEx::throwError()
{
    if(doRestore)
        restoreFromBackup();
    return QIODeviceHelper<QFile>::throwError();
}

QString QFileEx::getErrDataStr()
{
    QString err = QIODeviceHelper<QFile>::getErrDataStr();
    if(!this->fileName().isEmpty())
        err.prepend("file: "+this->fileName()+"; ");
    return err;
}


QSaveFileEx::QSaveFileEx(const QString &filename, QObject* parent)
    : QIODeviceHelper<QSaveFile>(parent)
{
    setFileName(filename);
}

QSaveFileEx::~QSaveFileEx()
{
    commit();
}

bool QSaveFileEx::throwError()
{
    cancelWriting();
    return QIODeviceHelper<QSaveFile>::throwError();
}

QString QSaveFileEx::getErrDataStr()
{
    QString err = QIODeviceHelper<QSaveFile>::getErrDataStr();
    if(!this->fileName().isEmpty())
        err.prepend("file: "+this->fileName()+"; ");
    return err;
}

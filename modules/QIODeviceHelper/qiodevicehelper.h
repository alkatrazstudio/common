/****************************************************************************}
{ qiodevicehelper.h - shortcut functions for QIODevice children              }
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

#pragma once

#include "errormanager.h"
#include <QtCore>

#ifdef QT_NETWORK_LIB
    #include <QtNetwork>
#endif

namespace QIODeviceHelperErr {
    Q_NAMESPACE

    enum class Err {
        read,
        write
    };
    Q_ENUM_NS(Err)
}

template <typename T> class QIODeviceHelper : public T
{
public:
    inline QIODeviceHelper() : T(), throwOnError(false){}
    inline QIODeviceHelper(QObject* parent) : T(parent), throwOnError(false){}

    inline QByteArray readUntilChar(char stopChar = 0, bool allowEof = false)
    {
        QByteArray buf;
        char c;
        while(this->getChar(&c))
        {
            if(c==stopChar)
                return buf;
            buf.append(c);
        }
        if(!this->atEnd() || !allowEof || buf.isEmpty())
            throwReadError();
        return buf;
    }

    inline QByteArray readUntilReturn(bool allowEof = true)
    {
        QByteArray buf;
        char c;
        while(this->getChar(&c))
        {
            if(c=='\n')
            {
                this->peek(&c, 1);
                if(c=='\r')
                    this->getChar(&c);
                return buf;
            }
            else
            {
                if(c=='\r')
                {
                    this->peek(&c, 1);
                    if(c=='\n')
                        this->getChar(&c);
                    return buf;
                }
            }
            buf.append(c);
        }
        if(!this->atEnd() || !allowEof || buf.isEmpty())
            throwReadError();
        return buf;
    }

    inline bool writeStringUTF8(const QString& string, char stopChar = 0)
    {
        if(this->write(string.toUtf8()) == -1)
            return throwWriteError();
        if(!this->putChar(stopChar))
            return throwWriteError();
        return true;
    }

    inline bool writeString(const QString& string, char stopChar = 0){return writeStringUTF8(string, stopChar);}

    inline bool writeStringASCII(const QString& string, char stopChar = 0)
    {
        if(this->write(string.toLatin1()) == -1)
            return throwWriteError();
        return this->putChar(stopChar) ? true:throwWriteError();
    }

    inline static bool isNotUtf8(const unsigned char* line, int len = - 1)
    {
        int a = -1;
        quint8 cCheck = 0xC0;
        quint8 cMask = 0xE0;
        quint8 curCheck;
        quint8 curMask;
        int i, b;
        forever{
            a++;
            if(!line[a] || (a == len))
                break;
            if(line[a] < 0x80)
                continue;
            curCheck = cCheck;
            curMask = cMask;
            for(i=0; i<4; i++)
            {
                if((line[a] & curMask) == curCheck)
                {
                    for(b=0;b<=i;b++)
                    {
                        a++;
                        if(!line[a] || (a == len))
                            return true;
                        if((line[a] & 0xC0) != 0x80)
                            return true;
                    }
                    break;
                }
                curCheck = (curCheck >> 1) | 0x80;
                curMask = (curMask >> 1) | 0x80;
            }
        }
        return false;
    }

    inline static bool isNotUtf8(const char* line, int len = - 1)
    {
        return isNotUtf8(reinterpret_cast<const unsigned char*>(line), len);
    }

    inline static bool isNotUtf8(const QByteArray &line)
    {
        const char* data = line.constData();
        return isNotUtf8(data, line.size());
    }

    inline QString readStringUTF8(char stopChar = 0)
    {
        QByteArray utf = readUntilChar(stopChar);
        return QString::fromUtf8(utf.constData());
    }

    inline QString readStringASCII(char stopChar = 0)
    {
        QByteArray ascii = readUntilChar(stopChar);
        return  QString::fromLatin1(ascii.constData());
    }

    inline QString readString(char stopChar = 0){return readStringUTF8(stopChar);}

    inline QString readLineUTF8()
    {
        QByteArray utf = readUntilReturn();
        return QString::fromUtf8(utf.constData());
    }

    inline QString readLineASCII()
    {
        QByteArray ascii = readUntilReturn();
        return QString::fromLatin1(ascii.constData());
    }

    inline bool readLn(QByteArray& data)
    {
        data = this->readLine();
        char c;
        if(data.isEmpty())
        {
            throwReadError();
            return false;
        }
        while(!data.isEmpty())
        {
            c = data.at(data.size()-1);
            if((c=='\n')||(c=='\r')||(c=='\0'))
                data.chop(1);
            else
                break;
        }
        return true;
    }

    inline bool readLnASCII(QString& dstString)
    {
        QByteArray data;
        if(!readLn(data))
            return false;
        dstString = QString::fromLatin1(data.constData());
        return true;
    }

    inline bool readLnUTF8(QString& dstString)
    {
        QByteArray data;
        if(!readLn(data))
            return false;
        dstString = QString::fromUtf8(data.constData());
        return true;
    }

    inline bool writeLnASCII(const QString& dstString)
    {
        QByteArray data = dstString.toLatin1();
        data.append('\n');
        return this->write(data) == data.size() ? true:throwWriteError();
    }

    inline bool writeLnUTF8(const QString& dstString)
    {
        QByteArray data = dstString.toUtf8();
        data.append('\n');
        return this->write(data) == data.size() ? true:throwWriteError();
    }

    inline QStringList readLinesASCII()
    {
        QStringList lines;
        QString line;
        while(readLnASCII(line))
            lines.append(line);
        return lines;
    }

    inline QStringList readLinesUTF8()
    {
        QStringList lines;
        QString line;
        while(readLnUTF8(line))
            lines.append(line);
        return lines;
    }

    inline bool writeLinesASCII(const QStringList& lines)
    {
        foreach(QString line, lines)
            if(!writeLnASCII(line))
                return false;
        return true;
    }

    inline bool writeLinesUTF8(const QStringList& lines)
    {
        foreach(QString line, lines)
            if(!writeLnUTF8(line))
                return false;
        return true;
    }

    inline bool writeInt(qint8 value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(quint8 value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(qint16 value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(quint16 value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(qint32 value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(quint32 value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(const qint64& value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt(const quint64& value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeInt8(qint8 value){return writeInt(value);}
    inline bool writeUint8(quint8 value){return writeInt(value);}
    inline bool writeInt16(qint16 value){return writeInt(value);}
    inline bool writeUint16(quint16 value){return writeInt(value);}
    inline bool writeInt32(qint32 value){return writeInt(value);}
    inline bool writeUint32(quint32 value){return writeInt(value);}
    inline bool writeInt64(const qint64& value){return writeInt(value);}
    inline bool writeUint64(const quint64& value){return writeInt(value);}

    inline bool writeFloat(float value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writeDouble(double value)
    {
        return this->write(reinterpret_cast<const char*>(&value), sizeof(value)) == sizeof(value) ? true:throwWriteError();
    }

    inline bool writePoint(const QPoint& p)
    {
        return writeInt32(p.x()) && writeInt32(p.y());
    }

    inline qint8 readInt8()
    {
        qint8 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline quint8 readUint8()
    {
        quint8 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline qint16 readInt16()
    {
        qint16 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline quint16 readUint16()
    {
        quint16 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline qint32 readInt32()
    {
        qint32 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline quint32 readUint32()
    {
        quint32 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline qint64 readInt64()
    {
        qint64 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline quint64 readUint64()
    {
        quint64 value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline float readFloat()
    {
        float value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline double readDouble()
    {
        double value;
        this->read(reinterpret_cast<char*>(&value), sizeof(value)) == sizeof(value) ? true:throwReadError();
        return value;
    }

    inline QPoint readPoint()
    {
        int x = readInt32();
        int y = readInt32();
        return QPoint(x, y);
    }

    inline bool writeBool(bool value){return writeUint8(value);}
    inline bool readBool(){return readUint8();}

    inline bool atUtf8BOM(){bool isError; return atUtf8BOM(isError);}
    inline bool atUtf8BOM(bool& isError)
    {
        unsigned char bom[3];
        bom[0] = bom[1] = bom[2] = 0;

        qint64 bytesRead = this->peek(reinterpret_cast<char*>(&bom[0]), 3);
        if(bytesRead == -1)
        {
            isError = true;
            return throwReadError();
        }
        isError = false;
        if(bytesRead != 3)
            return false;

        return (
            (bom[0] == 0xEF)
                &&
            (bom[1] == 0xBB)
                &&
            (bom[2] == 0xBF)
                );
    }

    inline bool skipUtf8BOM(){bool wasAtBOM; return skipUtf8BOM(wasAtBOM);}
    inline bool skipUtf8BOM(bool& wasAtBOM)
    {
        bool isError;
        wasAtBOM = atUtf8BOM(isError);
        if(isError)
            return false;
        if(!wasAtBOM)
            return true;

        uchar bom[3];
        return this->read(reinterpret_cast<char*>(&bom[0]), 3) == 3 ? true:throwReadError();
    }

    static QString errorCodeToString(QIODeviceHelperErr::Err errorCode)
    {
        switch(errorCode)
        {
            case QIODeviceHelperErr::Err::read: return QStringLiteral("Read error");
            case QIODeviceHelperErr::Err::write: return QStringLiteral("Write error");
            default: return QStringLiteral("Undefined error");
        }
    }

    inline void setThrowOnError(bool val){throwOnError = val;}
    inline bool isThrowOnError() const {return throwOnError;}

protected:
    bool throwOnError;

    virtual bool throwError() {
#ifdef __EXCEPTIONS
        if(throwOnError)
            throw std::exception();
#endif
        return false;
    }

    bool throwReadError()
    {
        QString errDataStr = getErrDataStr();
        SETERROR(QIODeviceHelperErr::Err::read, errDataStr);
        return throwError();
    }

    bool throwWriteError()
    {
        QString errDataStr = getErrDataStr();
        SETERROR(QIODeviceHelperErr::Err::write, errDataStr);
        return throwError();
    }

    virtual QString getErrDataStr()
    {
        QString err;
        err.append("pos: ");
        err.append(QString::number(this->pos()));
        return err;
    }
};

class QIODeviceEx: public QIODeviceHelper<QIODevice> {
public:
    QIODeviceEx():QIODeviceHelper<QIODevice>(){}
    QIODeviceEx(QObject *parent):QIODeviceHelper<QIODevice>(parent){}
};

class QIODeviceExDec: public QIODeviceEx {
public:
    inline QIODeviceExDec(QIODevice *slave): QIODeviceEx(),dev(slave){
        open(ReadWrite | Unbuffered);
    }

    inline QIODevice* getSlave() const {return dev;}

    inline virtual bool atEnd() const {return dev->atEnd();}
    inline virtual qint64 bytesAvailable() const {return dev->bytesAvailable();}
    inline virtual qint64 bytesToWrite() const {return dev->bytesToWrite();}
    inline virtual bool canReadLine() const {return dev->canReadLine();}
    virtual bool isSequential() const {return dev->isSequential();}
    //virtual bool open(OpenMode mode) {return dev->open(mode);}
    virtual qint64 pos() const {return dev->pos();}
    virtual bool reset() {return dev->reset();}
    virtual bool seek(qint64 pos) {return dev->seek(pos);}
    virtual qint64 size() const {return dev->size();}
    virtual bool waitForBytesWritten(int msecs) {return dev->waitForBytesWritten(msecs);}
    virtual bool waitForReadyRead(int msecs) {return dev->waitForReadyRead(msecs);}

protected:
    inline virtual qint64 readData(char * data, qint64 maxSize){
        return dev->read(data, maxSize);
    }
    inline virtual qint64 writeData(const char * data, qint64 maxSize){
        return dev->write(data, maxSize);
    }
    virtual qint64 readLineData(char * data, qint64 maxSize) {return dev->readLine(data, maxSize);}

    QIODevice* dev;
};

class QBufferEx: public QIODeviceHelper<QBuffer>{};

class QFileEx: public QIODeviceHelper<QFile> {
public:
    QFileEx();
    QFileEx(const QString& filename);
    ~QFileEx();
    bool moveToBackup();
    bool openWithBackup(OpenMode mode = WriteOnly);
    bool restoreFromBackup();
    void removeBackup();
    void close(bool doRemoveBackupOnClose);
    virtual void close(){close(true);}
    inline const QString& getBackupSuffix() const {return backupSuffix;}
protected:
    bool doRestore;
    QString backupSuffix;
    virtual bool throwError();
    virtual QString getErrDataStr();
};

class QSaveFileEx: public QIODeviceHelper<QSaveFile> {
public:
    QSaveFileEx(QObject* parent = nullptr) : QIODeviceHelper<QSaveFile>(parent){}
    QSaveFileEx(const QString& filename, QObject *parent = nullptr);
    ~QSaveFileEx();
    virtual bool throwError();
    virtual QString getErrDataStr();
};

class QProcessEx: public QIODeviceHelper<QProcess>{};

#ifdef QT_NETWORK_LIB
    class QAbstractSocketEx: public QIODeviceHelper<QAbstractSocket>{};
    class QLocalSocketEx: public QIODeviceHelper<QLocalSocket>{};
#endif

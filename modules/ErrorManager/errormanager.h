/****************************************************************************}
{ errormanager.h - centralized error handling                                }
{                                                                            }
{ Copyright (c) 2013 Alexey Parfenov <zxed@alkatrazstudio.net>               }
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

#include <QtCore>
#include <cerrno>
#include <stdexcept>
#include <typeindex>

#define SETERROR(err, ...) ErrorManager::setError(false, this, typeid(this), __FILE__, __PRETTY_FUNCTION__, __LINE__, err, errorCodeToString(err), ## __VA_ARGS__)
#define SETERROR_S(type, err, ...) ErrorManager::setError(false, nullptr, typeid(type*), __FILE__, __PRETTY_FUNCTION__, __LINE__, err,  errorCodeToString(err), ## __VA_ARGS__)
#define SETERROR_E(err, ...) ErrorManager::setError(true, this, typeid(this), __FILE__, __PRETTY_FUNCTION__, __LINE__, err,  errorCodeToString(err), ## __VA_ARGS__)
#define SETERROR_ES(type, err, ...) ErrorManager::setError(true, nullptr, typeid(type*), __FILE__, __PRETTY_FUNCTION__, __LINE__, err, errorCodeToString(err), ## __VA_ARGS__)

#define CHECK(s, err, ...) {if(!(s)) {SETERROR(err, ## __VA_ARGS__); return false;}}
#define CHECKV(s, err, ...) {if(!(s)) {SETERROR(err, ## __VA_ARGS__); return;}}
#define CHECKB(s, err, ...) {if(!(s)) {SETERROR(err, ## __VA_ARGS__); break;}}
#define CHECKN(s, err, ...) {if(!(s)) {SETERROR(err, ## __VA_ARGS__); return -1;}}
#define CHECKZ(s, err, ...) {if(!(s)) {SETERROR(err, ## __VA_ARGS__); return 0;}}
#define CHECKP(s, err, ...) {if(!(s)) {SETERROR(err, ## __VA_ARGS__); return nullptr;}}
#define CHECKE(s, err, ...) {if(!(s)) {SETERROR_E(err, ## __VA_ARGS__);}}

#define CHECK_S(type, s, err, ...) {if(!(s)) {SETERROR_S(type, err, ## __VA_ARGS__); return false;}}
#define CHECKV_S(type, s, err, ...) {if(!(s)) {SETERROR_S(type, err, ## __VA_ARGS__); return;}}
#define CHECKB_S(type, s, err, ...) {if(!(s)) {SETERROR_S(type, err, ## __VA_ARGS__); break;}}
#define CHECKN_S(type, s, err, ...) {if(!(s)) {SETERROR_S(type, err, ## __VA_ARGS__); return -1;}}
#define CHECKZ_S(type, s, err, ...) {if(!(s)) {SETERROR_S(type, err, ## __VA_ARGS__); return 0;}}
#define CHECKP_S(type, s, err, ...) {if(!(s)) {SETERROR_S(type, err, ## __VA_ARGS__); return nullptr;}}
#define CHECKE_S(type, s, err, ...) {if(!(s)) {SETERROR_ES(type, err, ## __VA_ARGS__);}}

/*
    Should a class use error handling, it must implement the following function:
    static QString errorCodeToString(Err errorCode);
*/

class AppException;

class ErrorManager : public QObject
{
    Q_OBJECT

public:
    struct ErrorStruct
    {
        bool isException {};
        const void* sender {};
        const std::type_info* senderTypeInfo {};
        const char* sourceFile {};
        const char* funcSig {};
        int sourceLine {};
        int errorVal {};
        const std::type_info* codeTypeInfo {};
        const char* errorScope {};
        const char* errorName {};
        QString description;

        ErrorStruct() = default;
        inline ErrorStruct(const ErrorStruct& obj) = default;
        inline ErrorStruct(ErrorStruct&& obj) = default;

        inline ErrorStruct(
                bool isException,
                const void* sender,
                const std::type_info& senderTypeInfo,
                const char* sourceFile,
                const char* funcSig,
                int sourceLine,
                const QString& description,
                int errorVal,
                const std::type_info& codeTypeInfo,
                const char* errorScope,
                const char* errorName):
            isException(isException),
            sender(sender),
            senderTypeInfo(&senderTypeInfo),
            sourceFile(sourceFile),
            funcSig(funcSig),
            sourceLine(sourceLine),
            errorVal(errorVal),
            codeTypeInfo(&codeTypeInfo),
            errorScope(errorScope),
            errorName(errorName),
            description(description)
        {
        }

        virtual ~ErrorStruct() = default;
        QString logLine() const;

        template<typename T>
        bool codeIs(T errorCode) const
        {
            return codeIs<T>()
                && ErrorManager::getErrorVal(errorCode) == errorVal;
        }

        template<typename T>
        bool codeIs() const
        {
            return codeIs(typeid(T));
        }

        bool codeIs(const std::type_info& typeInfo) const
        {
            return std::type_index(typeInfo) == std::type_index(*codeTypeInfo);
        }

        template<typename T>
        bool senderIs() const
        {
            return std::type_index(typeid(T*)) == std::type_index(*senderTypeInfo);
        }

    protected:
        QString getDescription() const;
    };

    class Exception : public std::runtime_error
    {
    public:
        explicit inline Exception(const ErrorManager::ErrorStruct &err)
            : std::runtime_error(err.description.toUtf8().constData())
            , err_(err)
        {
        }
        Exception(const Exception&) = default;

        const ErrorStruct& err() const {return err_;}

    protected:
        ErrorStruct err_;
    };

    inline static int getLastError() {return errno &~ errnoMask;}
    static const int errnoMask = 0x0f000000;
    inline static ErrorManager *instance(){return errMan;}

    inline void setError(const ErrorStruct& err)
    {
        errno = err.errorVal | errnoMask;
        emit onError(err);
        if(err.isException)
            throw Exception(err);
    }

    template<typename Code>
    static int getErrorVal(Code errorCode)
    {
        return static_cast<std::underlying_type_t<Code>>(errorCode);
    }

    template<typename Code>
    inline static void setError(
            bool isException,
            const void* sender,
            const std::type_info& senderTypeInfo,
            const char* sourceFile,
            const char* funcSig,
            int sourceLine,
            Code errorCode,
            const QString& errorString,
            const QString& dataString = QString())
    {
        int errorVal = getErrorVal(errorCode);
        QMetaEnum meta = QMetaEnum::fromType<Code>();
        ErrorStruct err(
                    isException,
                    sender,
                    senderTypeInfo,
                    sourceFile,
                    funcSig,
                    sourceLine,
                    getDescription(errorString, dataString),
                    errorVal,
                    typeid(errorCode),
                    meta.scope(),
                    meta.valueToKey(errorVal));
        ErrorManager::instance()->setError(err);
    }

    static QString getDescription(
            const QString& errorString,
            const QString& dataString)
    {
        if(errorString.isEmpty())
            return errorString;
        if(dataString.isEmpty())
            return errorString;
        return errorString + ": " + dataString;
    }

    static const QString& noDescription()
    {
        static QString s;
        return s;
    }

    static void createInstance();
    ~ErrorManager();

    inline static QString errorToString(const ErrorStruct &err);

protected:
    ErrorManager();
    static ErrorManager* errMan;

signals:
    void onError(const ErrorManager::ErrorStruct &err);
};

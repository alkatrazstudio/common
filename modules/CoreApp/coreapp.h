/****************************************************************************}
{ coreapp.h - base application class                                         }
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
#include "coreapp_defines.h"

#ifdef QT_GUI_LIB
    #ifdef QT_WIDGETS_LIB
        #include <QApplication>
    #else
        #include <QGuiApplication>
    #endif
#else
    #include <QCoreApplication>
#endif

#ifdef COREAPP_SINGLEKEY
    #include <QLocalServer>
#endif
#ifdef COREAPP_DUMMYWIN
    #include <QMainWindow>
#endif
#ifdef COREAPP_SESSHANDLE
    #include <QSessionManager>
#endif

#ifndef Q_OS_WIN
    #include <csignal>
#endif

#ifdef QT_GUI_LIB
    #ifdef QT_WIDGETS_LIB
        using CoreAppParentClass = QApplication;
    #else
        using CoreAppParentClass = QGuiApplication;
    #endif
#else
    using CoreAppParentClass = QCoreApplication;
#endif

class CoreAppNativeEventFilter : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit CoreAppNativeEventFilter(QObject* parent = nullptr):QObject(parent){}
    bool nativeEventFilter(const QByteArray& eventType, void* message, long* result) override;

signals:
    void onEvent(void* event, bool& stopPropagation);
};

#define qCoreApp (static_cast<CoreApp*>(qApp))

#define COREAPP_MIN_CUSTOM_ERR 100

class CoreApp : public CoreAppParentClass
{
    Q_OBJECT

public:
    enum class Err {
#ifdef COREAPP_SINGLEKEY
        createLocalServer = 1,
        freeLocalServer,
        readFromLocalSock,
        writeToLocalSock,
        sendToLocalServer,
        localSocketDisconnect,
#endif
        removeOldTranslation,
        addTranslation,
        createDir,
        openWrite,
        openRead
    };
    Q_ENUM(Err)

    static inline CoreApp* instance() {return appInstance;}

    CoreApp(int& argc, char** argv, bool exitAfterMain = true);
    ~CoreApp() override;
    CoreApp(const CoreApp&) = delete;

    inline bool isQuitting() const {return quitting;}
    virtual void interrupt(int sigNum);
    bool loadTranslations(const QString& dirname, const QLocale &locale = QLocale());
    bool loadTranslations();
    void unloadTranslations();

    static const QStringList &userDataDirs();
    static const QStringList &sysDataDirs();
    static bool isAppImage();

    static const QDateTime &buildDate();

    static const QVersionNumber& version();
    static unsigned int majorVersion(){return static_cast<unsigned int>(version().majorVersion());}
    static unsigned int minorVersion(){return static_cast<unsigned int>(version().minorVersion());}
    static unsigned int patchVersion(){return static_cast<unsigned int>(version().microVersion());}
    Q_PROPERTY(unsigned int majorVersion READ majorVersion CONSTANT)
    Q_PROPERTY(unsigned int minorVersion READ minorVersion CONSTANT)
    Q_PROPERTY(unsigned int patchVersion READ patchVersion CONSTANT)
    Q_PROPERTY(QString version READ applicationVersion CONSTANT)
    Q_PROPERTY(QDateTime buildDate READ buildDate CONSTANT)

    static QString errorCodeToString(Err errorCode);

#ifdef COREAPP_DUMMYWIN
    QMainWindow* getDummyWindow() const {return dummyWindow;}
#endif

protected:
    static CoreApp* appInstance;

    bool exitAfterMain;
#ifdef COREAPP_DUMMYWIN
    bool createDummyWindow;
    QMainWindow *dummyWindow;
#endif
    bool quitting;
    bool logToFile;
    QFile *logFile;
    QMutex logMutex;
    const QDateTime timestampStarted;
    CoreAppNativeEventFilter eventFilter;
    QList<QTranslator*> translators;
    bool isDataSaved;

#ifdef COREAPP_SINGLEKEY
    bool singleInstance;
    QLocalServer* localServer;
    QLocalSocket* localSock;
    QStringList localSockArgs;
    QByteArray localSockBuffer;
    QString localSockName;
    virtual bool startLocalServer();
    void restartLocalServer();
    virtual bool passCommandsToLocalServer(QLocalSocket &sock);
    virtual void closeLocalSocket();
    virtual bool validateSingleInstance(int *exitCode = nullptr);
#endif
    virtual int main();

    void logLineSync(const QString &line);
    void logLine(const QString &line);

    bool addTranslator(const QLocale & locale,
                   const QString & filename,
                   const QString & prefix,
                   const QString & directory);

private slots:
    void _run();
    void _onQuit();
#ifdef COREAPP_SESSHANDLE
    void onCommitDataRequest(QSessionManager &manager);
#endif

protected slots:
    virtual void onError(const ErrorManager::ErrorStruct &err);
#ifdef COREAPP_SINGLEKEY
    virtual void onNewLocalServerConnection();
    virtual void onLocalSocketClientData();
    virtual void onLocalSockDisconnect();
    virtual void onNewInstanceArgs(){}
#endif

    virtual void onNativeEvent(void *event, bool &stopPropagation);

    virtual void onSaveData(){}
    virtual void onQuit(){}
};

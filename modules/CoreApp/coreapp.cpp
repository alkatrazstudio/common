/****************************************************************************}
{ coreapp.cpp - base application class                                       }
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

#include "coreapp.h"
#include "common_defines.h"

#ifdef COREAPP_SINGLEKEY
    #include <QLocalSocket>
#endif

#ifdef Q_OS_WIN
    #include <windows.h>
#endif

CoreApp* CoreApp::appInstance = nullptr;

#ifndef Q_OS_WIN
void sigHandler(int signo)
{
    switch(signo)
    {
        case SIGINT:
            qCoreApp->interrupt(signo);
            break;
    }
}
#else
BOOL CtrlHandler(DWORD fdwCtrlType)
{
    switch(fdwCtrlType)
    {
        case CTRL_C_EVENT:
            ((CoreApp*)qApp)->interrupt(fdwCtrlType);
            return true;

        default:
            return false;
    }
}
#endif

static char** staticInit(char** dummy)
{
    CoreApp::setAttribute(Qt::AA_EnableHighDpiScaling);
    return dummy;
}

CoreApp::CoreApp(int& argc, char** argv, bool exitAfterMain):CoreAppParentClass(argc, staticInit(argv))
  ,exitAfterMain(exitAfterMain)
#ifdef COREAPP_DUMMYWIN
  ,createDummyWindow(false)
  ,dummyWindow(nullptr)
#endif
  ,quitting(false)
  ,logToFile(false)
  ,logFile(nullptr)
  ,timestampStarted(QDateTime::currentDateTime())
  ,isDataSaved(false)
#ifdef COREAPP_SINGLEKEY
  ,singleInstance(true)
  ,localServer(nullptr)
  ,localSock(nullptr)
  ,localSockName(COREAPP_SINGLEKEY)
#endif
{
    appInstance = this;

    setApplicationName(APP_NAME);
#ifdef QT_GUI_LIB
    setApplicationDisplayName(APP_TITLE);
#endif
    setApplicationVersion(QString::number(VER_MAJ)+'.'+QString::number(VER_MIN)+'.'+QString::number(VER_PAT));
    setOrganizationName(APP_ORG);
    setOrganizationDomain(APP_ORG_DOMAIN);

    ErrorManager::createInstance();
    connect(
        ErrorManager::instance(),
        &ErrorManager::onError,
        this,
        &CoreApp::onError,
        Qt::DirectConnection);

    connect(this, &CoreApp::aboutToQuit, this, &CoreApp::_onQuit);
    QTimer::singleShot(0, this, &CoreApp::_run);

#ifndef Q_OS_WIN
    for(int i=0; i<NSIG; i++)
        signal(i, sigHandler);
#else
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)CtrlHandler, true);
#endif

    QAbstractEventDispatcher::instance()->installNativeEventFilter(&eventFilter);
    connect(&eventFilter, &CoreAppNativeEventFilter::onEvent, this, &CoreApp::onNativeEvent);

#ifdef COREAPP_SESSHANDLE
    connect(
        this, CoreApp::commitDataRequest,
        this, CoreApp::onCommitDataRequest,
        Qt::DirectConnection);
#endif


#ifdef QT_QML_LIB
    if(!COREAPP_QML_IMPORT_PATHS.empty())
    {
        QList<QString> qmlCommonPaths;
        for(const char* relPath : COREAPP_QML_IMPORT_PATHS)
            qmlCommonPaths.append(applicationDirPath()+"/"+relPath);
        QString qmlEnvImportPath = qEnvironmentVariable("QML2_IMPORT_PATH");
        if(!qmlEnvImportPath.isEmpty())
            qmlCommonPaths.prepend(qmlEnvImportPath);
        QString qmlCommonPath = qmlCommonPaths.join(
#ifdef Q_OS_WIN
            ';'
#else
            ':'
#endif
        );
        qputenv("QML2_IMPORT_PATH", qmlCommonPath.toUtf8().constData());
    }
#endif
}

CoreApp::~CoreApp()
{
    unloadTranslations();
    QAbstractEventDispatcher::instance()->removeNativeEventFilter(&eventFilter);
    delete ErrorManager::instance();
    delete logFile;
#ifdef COREAPP_DUMMYWIN
    delete dummyWindow;
#endif
#ifdef COREAPP_SINGLEKEY
    delete localSock;
#endif

    appInstance = nullptr;
}

QString CoreApp::errorCodeToString(Err errorCode)
{
    switch(errorCode)
    {
#ifdef COREAPP_SINGLEKEY
        case Err::createLocalServer: return "Cannot create a local server";
        case Err::freeLocalServer: return "Cannot free the local server";
        case Err::readFromLocalSock: return "Error reading from local socket";
        case Err::writeToLocalSock: return "Error writing to local socket";
        case Err::sendToLocalServer: return "Error sending commands to the local server";
        case Err::localSocketDisconnect: return "Local socket have disconnected unexpentedly";
#endif
        case Err::removeOldTranslation: return "Unable to release the old translation";
        case Err::addTranslation: return "Unable to load a translation";
        case Err::createDir: return "Cannot create the directory";
        case Err::openWrite: return "Cannot open the file for writing";
        case Err::openRead: return "Cannot open the file for reading";
    }
    return QString();
}

void CoreApp::logLineSync(const QString &line)
{
    QMutexLocker mutexLocker(&logMutex);
    logLine(line);
}

void CoreApp::logLine(const QString &line)
{
    QString str;
    str.append("[");
    str.append(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
    str.append("]");

    if(logToFile)
    {
        if(!logFile)
        {
            QString logsDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"logs/";
            QDir d(logsDir);
            if(!d.mkpath(logsDir))
            {
                logToFile = false;
                SETERROR(Err::createDir, logsDir);
                return;
            }
            logFile = new QFile();
            QString fName = logsDir+timestampStarted.toString("yyyy-MM-dd_hh-mm-ss")+".log";
            logFile->setFileName(fName);
            bool ok = logFile->open(QIODevice::WriteOnly);
            if(ok)
            {
                QString s(str);
                s.append(" Debug log has been opened: ").append(fName);
                qDebug("%s", qPrintable(s));
            }

            logLine(QStringLiteral("System info. OS: ")+QSysInfo::prettyProductName()+"; app version: "+applicationVersion()+"; started at: "+timestampStarted.toString("yyyy-MM-dd hh:mm:ss"));
            if(!ok)
            {
                logToFile = false;
                delete logFile;
                logFile = nullptr;
                SETERROR(Err::openWrite, fName);
            }
        }
        str.append(" ");
        str.append(line);
        if(logFile)
        {
            logFile->write(str.toUtf8());
            logFile->putChar('\n');
            logFile->flush();
        }
    }
    else
    {
        str.append(" ");
        str.append(line);
    }

    qWarning("%s", qPrintable(str));
}

bool CoreApp::addTranslator(const QLocale &locale, const QString &filename, const QString &prefix, const QString &directory)
{
    QTranslator* translator = new QTranslator();
    for(QString lang : locale.uiLanguages())
    {
        // if one of UI langs is English then consider its translator loaded
        // this will allow to not load next UI langs (since there will be no en.qm)
        if(!canHaveTranslation(lang))
            return true;
        lang.replace('-', '_');
        QString qmBasename = filename + prefix + lang;
        if(translator->load(qmBasename, directory))
        {
            if(qApp->installTranslator(translator))
            {
                translators.append(translator);
                return true;
            }
        }
    }

    delete translator;
    return false;
}

void CoreApp::interrupt(int sigNum)
{
    Q_UNUSED(sigNum);
    quit();
}

bool CoreApp::canHaveTranslation(const QString& lang)
{
    return lang != "en" && !lang.startsWith("en-");
}

bool CoreApp::loadTranslations(const QString &dirname, const QLocale& locale)
{
    QStringList systemLibs = {
        "qt",
        "qtbase",
#ifdef QT_QML_LIB
        "qtdeclarative",
#endif
#ifdef QT_QUICKCONTROLS2_LIB
        "qtquickcontrols2",
#endif
    };

    QString translationsPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QStringList sysDirs = sysDataDirs();
    std::reverse(sysDirs.begin(), sysDirs.end());

    foreach(const QString& lib, systemLibs)
    {
        addTranslator(locale, lib, "_", translationsPath);
        addTranslator(locale, lib, "_", dirname);
        for(const QString& dir : qAsConst(sysDirs))
            addTranslator(locale, lib, "_", dir);
    }

    bool loaded = addTranslator(locale, "", "", dirname);
    for(const QString& dir : qAsConst(sysDirs))
        if(addTranslator(locale, "", "", dir+"/translations"))
            loaded = true; // no break => load from all locations

    if(!loaded)
    {
        // do not pollute a console
        // since English is not supposed to have a translation in the first place
        QStringList langs = locale.uiLanguages();
        foreach(const QString& lang, langs)
            CHECK(!canHaveTranslation(lang), Err::addTranslation, langs.join(","));
        return false;
    }

    return true;
}

bool CoreApp::loadTranslations()
{
    unloadTranslations();
    return loadTranslations(qApp->applicationDirPath() + "/translations");
}

void CoreApp::unloadTranslations()
{
    foreach(QTranslator* t, translators)
    {
        qApp->removeTranslator(t);
        delete t;
    }
    translators.clear();
}

const QStringList &CoreApp::userDataDirs()
{
    static bool isFilled = false;
    static QStringList dirs;
    if(!isFilled)
    {
        QStringList locals;
        QStringList locs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        QStringList homes =
#ifdef Q_OS_ANDROID
            QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
#else
            QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
#endif
        for(const QString& loc : qAsConst(locs))
        {
#ifndef Q_OS_ANDROID
            if(loc.startsWith(qApp->applicationDirPath()))
                continue;
#endif
            for(const QString& home : qAsConst(homes))
                if(loc.startsWith(home))
                    dirs.append(loc);
            isFilled = true;
        }
    }
    return dirs;
}

const QStringList &CoreApp::sysDataDirs()
{
    static bool isFilled = false;
    static QStringList dirs;
    if(!isFilled)
    {
#ifdef Q_OS_LINUX
        QStringList nonLocals;
#endif
        QStringList locs = QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
        QStringList homes =
#ifdef Q_OS_ANDROID
            QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
#else
            QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
#endif
        for(const QString& loc : qAsConst(locs))
        {
#ifndef Q_OS_ANDROID
            if(loc.startsWith(qApp->applicationDirPath()))
                continue;
#endif
            bool ok = true;
            for(const QString& home : qAsConst(homes))
            {
                if(loc.startsWith(home))
                {
                    ok = false;
                    break;
                }
            }
            if(!ok)
                continue;
#ifdef Q_OS_LINUX
            QString newLoc = loc;
            // assuming organization name is not something like "usr" or "local"
            newLoc.replace(QStringLiteral("/")+organizationName()+"/", "/");
            // put /usr/local first
            if(newLoc.startsWith("/usr/local"))
                dirs.append(newLoc);
            else
                nonLocals.append(newLoc);
#else
            dirs.append(loc);
#endif
        }
#ifdef Q_OS_LINUX
        for(const QString& loc : qAsConst(nonLocals))
            dirs.append(loc);
#endif
        isFilled = true;
    }
    return dirs;
}

bool CoreApp::isAppImage()
{
#ifdef Q_OS_LINUX
    return !qEnvironmentVariableIsEmpty("APPIMAGE") && !qEnvironmentVariableIsEmpty("ARGV0");
#else
    return false;
#endif
}

const QDateTime &CoreApp::buildDate()
{
    static QDateTime dateTime = QDateTime::fromSecsSinceEpoch(BUILD_TS);
    return dateTime;
}

const QVersionNumber &CoreApp::version()
{
    static QVersionNumber v(VER_MAJ, VER_MIN, VER_PAT);
    return v;
}

void CoreApp::_run()
{
#ifdef COREAPP_SINGLEKEY
    if(singleInstance)
    {
        SingleInstanceValidationResult result;
        if(!validateSingleInstance(result))
        {
            switch(result)
            {
                case SingleInstanceValidationResult::runningInstanceFound:
                    exit(0);
                    break;

                default:
                    exit(-1);
            }
            return;
        }
    }
#endif

#ifdef COREAPP_DUMMYWIN
    if(createDummyWindow)
    {
        // If Qt app has no windows then it will be terminated immediately when user logs out.
        // To receive any shutdown notifications (aboutToClose, onCommitDataRequest)
        // the application must have a window.
        setQuitOnLastWindowClosed(false); // in case other windows will be created and closed
        dummyWindow = new QMainWindow; // simple QWindow doesn't have setWindowFlags that allows to hide it from taskbar
        QRect geom = dummyWindow->geometry(); // get default size, because setting wrong size in setGeometry may alter the position as well
        dummyWindow->setGeometry(-geom.width(), -geom.height(), geom.width(), geom.height()); // drag it off screen to avoid flashing
        //dummyWindow->setWindowFlags(dummyWindow->windowFlags() | Qt::Tool); // tool windows don't display their icons in taskbar
        dummyWindow->setAttribute(Qt::WA_MacAlwaysShowToolWindow, true); // just to be sure that the window will be created
        dummyWindow->show(); // the window is not actually created until it's shown
        dummyWindow->hide(); // hide the window immediately since it shouldn't be there in the first place
        connect(dummyWindow, &QMainWindow::destroyed, [this](){
            quit(); // quit the app if this window is destroyed
            dummyWindow = nullptr; // clear non-existent pointer
        });
        dummyWindow->setAttribute(Qt::WA_DeleteOnClose, true); // close event will destroy the window and trigger the above callback
    }
#endif

    int result = main();
    if(result || quitting || exitAfterMain)
        exit(result);
}

int CoreApp::main()
{
    return 0;
}

#ifdef COREAPP_SINGLEKEY
bool CoreApp::startLocalServer()
{
    if(!localServer)
    {
        localServer = new QLocalServer(this);
        connect(localServer, &QLocalServer::newConnection, this, &CoreApp::onNewLocalServerConnection);
    }
    if(!localServer->isListening())
    {
        if(!localServer->listen(localSockName))
        {
            CHECK(localServer->serverError() == QAbstractSocket::AddressInUseError, Err::createLocalServer);
            CHECK(QLocalServer::removeServer(localSockName), Err::freeLocalServer);
            CHECK(localServer->listen(localSockName), Err::createLocalServer);
        }
    }
    return true;
}

bool CoreApp::passCommandsToLocalServer(QLocalSocket &sock)
{
    QStringList args = arguments();
    if(args.size() <= 1)
        return true;
    args.removeFirst();
    QByteArray data;
    for(const QString& arg : qAsConst(args))
    {
        data.append(arg.toUtf8());
        data.append('\0');
    }
    data.append('\0');
    if(sock.write(data) == -1)
    {
        SETERROR(Err::sendToLocalServer);
        return true;
    }
    if(!sock.waitForBytesWritten())
    {
        SETERROR(Err::sendToLocalServer);
        return true;
    }
    sock.waitForDisconnected();
    data = sock.readAll();
    if(!data.isEmpty())
        QTextStream(stdout) << data;
    return true;
}

void CoreApp::closeLocalSocket()
{
    if(localSock)
    {
        localSock->disconnect(this);
        localSock->disconnectFromServer(); // Qt BUG? This causes the server to stop accepting connections
        localSockArgs.clear();
        localSockBuffer.clear();
        localSock->deleteLater();
        localSock = nullptr;
    }
}

bool CoreApp::validateSingleInstance(SingleInstanceValidationResult &result)
{
    QLocalSocket s;
    s.connectToServer(localSockName, QIODevice::ReadWrite);
    if(s.waitForConnected())
    {
        if(passCommandsToLocalServer(s))
        {
            result = SingleInstanceValidationResult::runningInstanceFound;
            return false;
        }
        else
        {
            result = SingleInstanceValidationResult::runningInstanceIgnored;
        }
    }
    else
    {
        result = SingleInstanceValidationResult::runningInstanceNotFound;
    }
    if(!startLocalServer())
    {
        result = SingleInstanceValidationResult::cannotStartLocalServer;
        return false;
    }
    return true;
}

void CoreApp::onNewLocalServerConnection()
{
    closeLocalSocket();
    localSock = localServer->nextPendingConnection();
    if(!localSock)
        return;
    connect(localSock, &QLocalSocket::readyRead, this, &CoreApp::onLocalSocketClientData);
}

void CoreApp::onLocalSocketClientData()
{
    if(!localSock)
        return;
    if(qobject_cast<QLocalSocket*>(sender()) != localSock)
        return;
    qint64 n = localSock->bytesAvailable();
    char x;

    for(qint64 a=0; a<n; a++)
    {
        if(!localSock->getChar(&x))
        {
            closeLocalSocket();
            SETERROR(Err::readFromLocalSock);
            return;
        }
        if(x == 0)
        {
            if(localSockBuffer.isEmpty())
            {
                onNewInstanceArgs();
                bool ok;
                if(localSock->bytesToWrite())
                    ok = localSock->waitForBytesWritten();
                else
                    ok = true;
                closeLocalSocket();
                CHECKV(ok, Err::writeToLocalSock);
                return;
            }
            localSockArgs.append(QString::fromUtf8(localSockBuffer.constData()));
            localSockBuffer.clear();
        }
        else
        {
            localSockBuffer.append(x);
        }
    }
}

void CoreApp::onLocalSockDisconnect()
{
    closeLocalSocket();
    SETERROR(Err::localSocketDisconnect);
}
#endif

void CoreApp::onNativeEvent(void *event, bool &stopPropagation)
{
    Q_UNUSED(event);
    Q_UNUSED(stopPropagation);
}

void CoreApp::_onQuit()
{
    if(!isDataSaved)
    {
        onSaveData();
        isDataSaved = true;
    }
    quitting = true;
    onQuit();
}

#ifdef COREAPP_SESSHANDLE
void CoreApp::onCommitDataRequest(QSessionManager &manager)
{
    Q_UNUSED(manager);
    if(quitting)
        return;
    onSaveData();
    isDataSaved = true;
}
#endif

void CoreApp::onError(const ErrorManager::ErrorStruct &err)
{
    logLineSync(err.logLine());
}

bool CoreAppNativeEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);
    Q_UNUSED(result);
    bool stopPropagation = false;
    emit onEvent(message, stopPropagation);
    return stopPropagation;
}

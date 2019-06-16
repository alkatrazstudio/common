/****************************************************************************}
{ errormanager.cpp - centralized error handling                              }
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

#include "errormanager.h"
#include "common_defines.h"

ErrorManager* ErrorManager::errMan = nullptr;

void ErrorManager::createInstance()
{
    if(!errMan)
        errMan = new ErrorManager();
}

ErrorManager::~ErrorManager()
{
    errMan = nullptr;
}

ErrorManager::ErrorManager()
{
    qRegisterMetaType<ErrorManager::ErrorStruct>("ErrorManager::ErrorStruct");
}

QString ErrorManager::ErrorStruct::logLine() const
{
    QRegularExpression rx("((?:\\w+\\:\\:)?\\w+)\\(");
    QRegularExpressionMatch match = rx.match(QString(funcSig));
    QString fSig;
    QString errText(errorScope);
    if(match.hasMatch())
    {
        fSig = match.captured(1);
        QStringList fSigParts = fSig.split("::");
        if(!fSigParts.isEmpty())
            fSigParts.removeLast();
        QStringList errScopeParts = errText.split("::");
        for(const QString& part : qAsConst(fSigParts))
        {
            if(errScopeParts.isEmpty())
                break;
            if(errScopeParts.first() == part)
                errScopeParts.removeFirst();
        }
        errText = errScopeParts.join("::");
    }
    if(!errText.isEmpty())
        errText = errText + "::" + errorName;
    else
        errText = errorName;

    QString line = QStringLiteral("[") % fSig % " - " % errText % "] "
                    % description
                    % " [" % QDir(SOURCE_ROOT).relativeFilePath(sourceFile) % ":" % QString::number(sourceLine) % "]";
    return line;
}

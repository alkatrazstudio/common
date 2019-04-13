/****************************************************************************}
{ codehighlighter.cpp - Kate-like syntax highlighter                         }
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

#include "codehighlighter.h"
#include <QMutexLocker>
#include <KF5/KSyntaxHighlighting/Definition>
#include <KF5/KSyntaxHighlighting/Theme>

using namespace KSyntaxHighlighting;

CodeHighlighter::CodeHighlighter(QObject* parent) :
    SyntaxHighlighter(parent ? parent : new QObject())
{
    connectSignals();
}

CodeHighlighter::CodeHighlighter(QTextDocument* document) :
    SyntaxHighlighter(document)
{
    connectSignals();
}

void CodeHighlighter::setTheme(const Theme &theme)
{
    SyntaxHighlighter::setTheme(theme);
    rehighlight();
}

const QString &CodeHighlighter::defaultThemeId()
{
    static QString s(QStringLiteral("_DEFAULT"));
    return s;
}

const QString &CodeHighlighter::lightThemeId()
{
    static QString s(QStringLiteral("_LIGHT"));
    return s;
}

const QString &CodeHighlighter::darkThemeId()
{
    static QString s(QStringLiteral("_DARK"));
    return s;
}

QColor CodeHighlighter::getFG() const
{
    return theme().textColor(Theme::Normal);
}

QColor CodeHighlighter::getGutterBG() const
{
    return theme().editorColor(Theme::IconBorder);
}

QColor CodeHighlighter::getLineNumbersFG() const
{
    return theme().editorColor(Theme::LineNumbers);
}

QColor CodeHighlighter::getCurrentLineNumberFG() const
{
    return theme().editorColor(Theme::CurrentLineNumber);
}

QColor CodeHighlighter::getCurrentLineFG() const
{
    return theme().editorColor(Theme::CurrentLine);
}

QColor CodeHighlighter::getSelectionBG() const
{
    QRgb color = theme().selectedBackgroundColor(Theme::Normal);
    if(color == 0)
        return theme().editorColor(Theme::TextSelection);
    return color;
}

QColor CodeHighlighter::getSelectionFG() const
{
    QRgb color = theme().selectedTextColor(Theme::Normal);
    if(color == 0)
        getFG();
    return color;
}

QColor CodeHighlighter::getBG() const
{
    QRgb color = theme().backgroundColor(Theme::Normal);
    if(color == 0)
        color = theme().editorColor(Theme::BackgroundColor);
    return color;
}

QString CodeHighlighter::getThemeName() const
{
    return theme().name();
}

void CodeHighlighter::connectSignals()
{
    connect(this, &CodeHighlighter::on_syntax_changed, [this]{
        setDefinition(getRepo().definitionForName(syntax_));
        setDocFromQmlParent();
    });
    connect(this, &CodeHighlighter::on_themeName_changed, [this]{
        Theme t;
        if(themeName_ == defaultThemeId())
            t = getRepo().defaultTheme();
        else if(themeName_ == lightThemeId())
            t = getRepo().defaultTheme(Repository::LightTheme);
        else if(themeName_ == darkThemeId())
            t = getRepo().defaultTheme(Repository::DarkTheme);
        else
        {
            t = getRepo().theme(themeName_);
            if(!t.isValid())
                t = getRepo().defaultTheme();
        }
        setTheme(t);
        emit onThemeChanged();
        setDocFromQmlParent();
    });
    connect(this, &CodeHighlighter::on_doc_changed, [this]{
        setDocument(doc_->textDocument());
    });
}

void CodeHighlighter::setDocFromQmlParent()
{
    if(document())
        return;
    if(!parent())
        return;
    QQuickItem* qmlItem = qobject_cast<QQuickItem*>(parent());
    if(!qmlItem || !qmlItem->inherits("QQuickTextEdit"))
        return;
    QQuickTextDocument* doc = qmlItem->property("textDocument").value<QQuickTextDocument *>();
    if(!doc)
        return;
    set_doc(doc);
}

Repository &CodeHighlighter::getRepo()
{
    static QMutex mx;
    QMutexLocker lock(&mx);
    static Repository repo;
    return repo;
}

/****************************************************************************}
{ codehighlighter.h - Kate-like syntax highlighter                           }
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

#include <QQuickItem>
#include <QQuickTextDocument>
#include <KF5/KSyntaxHighlighting/SyntaxHighlighter>
#include <KF5/KSyntaxHighlighting/Repository>
#include "qmlsimple.h"

class CodeHighlighter : public KSyntaxHighlighting::SyntaxHighlighter
{
    Q_OBJECT
public:
    explicit CodeHighlighter(QObject *parent = nullptr);
    explicit CodeHighlighter(QTextDocument *document);

    SIMPLE_QML_OBJ_PROP(QString, syntax)
    signals: void on_syntax_changed();
    SIMPLE_QML_OBJ_PROP(QString, themeName)
    signals: void on_themeName_changed();
    SIMPLE_QML_POBJ_PROP(QQuickTextDocument, doc)
    signals: void on_doc_changed();

public:
    void setTheme(const KSyntaxHighlighting::Theme &theme) override;

    static void registerInQml() {
        qmlRegisterType<CodeHighlighter>(
                    "net.alkatrazstudio.components", 1, 0, "CodeHighlighter");
    }

    Q_PROPERTY(QColor fg READ getFG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor bg READ getBG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor gutterBG READ getGutterBG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor lineNumbersFG READ getLineNumbersFG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor currentLineNumberFG READ getCurrentLineNumberFG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor currentLineFG READ getCurrentLineFG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor selectionFG READ getSelectionFG NOTIFY onThemeChanged)
    Q_PROPERTY(QColor selectionBG READ getSelectionBG NOTIFY onThemeChanged)

    Q_PROPERTY(QString defaultThemeId READ defaultThemeId CONSTANT)
    Q_PROPERTY(QString lightThemeId READ lightThemeId CONSTANT)
    Q_PROPERTY(QString darkThemeId READ darkThemeId CONSTANT)

    static const QString& defaultThemeId();
    static const QString& lightThemeId();
    static const QString& darkThemeId();

    QColor getBG() const;
    QColor getFG() const;
    QColor getGutterBG() const;
    QColor getLineNumbersFG() const;
    QColor getCurrentLineNumberFG() const;
    QColor getCurrentLineFG() const;
    QColor getSelectionBG() const;
    QColor getSelectionFG() const;
    QString getThemeName() const;

signals:
    void onThemeChanged();

protected:
    void connectSignals();
    void setDocFromQmlParent();
    static KSyntaxHighlighting::Repository &getRepo();
};

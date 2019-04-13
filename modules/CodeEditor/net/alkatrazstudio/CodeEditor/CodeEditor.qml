/****************************************************************************}
{ CodeEditor.qml - basic code editor with code highlighting                  }
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

import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Layouts 1.11
import net.alkatrazstudio.components 1.0
import QtQuick.Controls.Material 2.1

Item {
    property int tabSpaces: 4
    property bool tabsAsSpaces: true
    property string spaceChar: ' '
    property string syntax: ''
    property color color: hl.fg
    property color backgroundColor: hl.bg
    property color currentLineBackgroundColor: hl.currentLineFG
    property color gutterBackgroundColor: hl.gutterBG
    property color lineNumbersColor: hl.lineNumbersFG
    property color currentLineNumberColor: hl.currentLineNumberFG
    property color selectionColor: hl.selectionFG
    property color selectionBackgroundColor: hl.selectionBG
    property alias font: textEdit.font
    property alias text: textEdit.text
    property alias readOnly: textEdit.readOnly
    property int minPtSize: 8
    property int maxPtSize: 32
    property string theme: systemTheme

    readonly property int lineNumber: 1 + Math.round(textEdit.cursorRectangle.y / fontMetrics.height)

    readonly property string systemTheme: {
        switch(Material.theme)
        {
            case Material.Light: return lightTheme
            case Material.Dark: return darkTheme
            default: return defaultTheme
        }
    }

    property alias defaultTheme: hl.defaultThemeId
    property alias lightTheme: hl.lightThemeId
    property alias darkTheme: hl.darkThemeId

    signal newPtSizeRequest(int newPtSize)

    id: root

    Rectangle {
        anchors.fill: parent
        color: backgroundColor
    }

    ScrollView {
        id: scroll
        anchors.fill: parent
        clip: true
        ScrollBar.vertical.policy: ScrollBar.AlwaysOn

        Flickable {
            id: container
            clip: true

            width: scroll.width
            height: scroll.height
            contentWidth: Math.max(
                               scroll.width,
                               textEdit.contentWidth + numbersCol.width + scroll.ScrollBar.vertical.width)
            contentHeight: Math.max(
                               scroll.height,
                               textEdit.contentHeight + scroll.ScrollBar.horizontal.height)

            boundsMovement: Flickable.StopAtBounds

            function ensureVisible(r)
            {
                if(contentX + numbersCol.width >= r.x)
                    contentX = r.x - numbersCol.width
                else if(contentX + width - scroll.ScrollBar.vertical.width <= r.x + r.width)
                    contentX = r.x + r.width - width + scroll.ScrollBar.vertical.width
                if(contentY >= r.y)
                    contentY = r.y
                else if(contentY + height - scroll.ScrollBar.horizontal.height <= r.y + r.height)
                    contentY = r.y + r.height - height + scroll.ScrollBar.horizontal.height
            }

            Rectangle {
                anchors.left: parent.left
                y: textEdit.cursorRectangle.y
                height: textEdit.cursorRectangle.height
                width: container.contentWidth
                color: currentLineBackgroundColor
            }

            TextEdit {
                id: textEdit
                selectByMouse: true
                font.family: 'monospace'
                color: root.color
                selectionColor: root.selectionBackgroundColor
                selectedTextColor: root.selectionColor
                width: Math.max(
                           scroll.width - scroll.ScrollBar.vertical.width,
                           textEdit.contentWidth)
                height: Math.max(
                            scroll.height - scroll.ScrollBar.horizontal.height,
                            textEdit.contentHeight)
                persistentSelection: true
                leftPadding: lineNumbers.width

                onCursorRectangleChanged: container.ensureVisible(cursorRectangle)

                MouseArea {
                    cursorShape: Qt.IBeamCursor
                    anchors.fill: parent
                    property int wheelStep: -1
                    onPressed: mouse.accepted = (mouse.button === Qt.RightButton)

                    acceptedButtons: Qt.LeftButton | Qt.RightButton
                    onClicked: {
                        if(mouse.button === Qt.RightButton)
                            contextMenu.popup()
                    }

                    onWheel: {
                        if(wheel.modifiers & Qt.ControlModifier)
                        {
                            const deltaY = wheel.angleDelta.y
                            if(wheelStep == -1 || deltaY < wheelStep)
                                wheelStep = Math.abs(deltaY)
                            const nSteps = Math.round(deltaY / wheelStep)
                            const targetPtSize = font.pointSize + nSteps
                            if(
                                (targetPtSize >= minPtSize || nSteps > 0)
                                && (targetPtSize <= maxPtSize || nSteps < 0)
                            )
                            {
                                newPtSizeRequest(targetPtSize)
                            }
                            wheel.accepted = true
                        }
                        else
                        {
                            wheel.accepted = false
                        }
                    }

                    Menu {
                        id: contextMenu

                        MenuItem
                        {
                            text: 'Cut'
                            enabled: textEdit.selectionStart !== textEdit.selectionEnd
                            onClicked: textEdit.cut()
                        }

                        MenuItem
                        {
                            text: 'Copy'
                            enabled: textEdit.selectionStart !== textEdit.selectionEnd
                            onClicked: textEdit.copy()
                        }

                        MenuItem
                        {
                            text: 'Paste'
                            enabled: textEdit.canPaste
                            onClicked: textEdit.paste()
                        }
                    }
                }

                CodeHighlighter {
                    id: hl
                    syntax: root.syntax
                    themeName: root.theme
                }

                FontMetrics {
                    id: fontMetrics
                    font: textEdit.font
                }
                tabStopDistance: fontMetrics.averageCharacterWidth * tabSpaces

                function getBlockRange()
                {
                    let start = selectionStart
                    while(text[start-1] !== '\n' && text[start-1] !== undefined)
                        start--
                    let end = selectionEnd
                    if(text[end-1] === '\n' && end > start)
                    {
                        end--
                    }
                    else
                    {
                        while(text[end] !== '\n' && text[end] !== undefined)
                            end++
                    }
                    return {
                        start,
                        end
                    }
                }

                function insertTab()
                {
                    if(!tabsAsSpaces)
                        return false

                    var ins = spaceChar.repeat(tabSpaces)

                    if(selectedText.length)
                    {
                        var range = getBlockRange()
                        var s = text.substring(range.start, range.end)
                        s = s.split('\n').map(function(line){return ins+line}).join('\n')
                        deselect()
                        replace(range.start, range.end, s)
                        select(range.start, range.start + s.length)
                    }
                    else
                    {
                        insert(cursorPosition, ins)
                    }

                    return true
                }

                function insertBacktab()
                {
                    if(selectedText.length)
                    {
                        const range = getBlockRange()
                        const spaceLen = spaceChar.length
                        const s = text
                            .substring(range.start, range.end)
                            .split('\n')
                            .map(line => {
                                if(line[0] === '\t')
                                    return line.substring(1)
                                let spacesLeft = tabSpaces
                                while(spacesLeft)
                                {
                                    const start = spaceLen*(tabSpaces-spacesLeft)
                                    if(line.substring(start, start + spaceLen) === spaceChar)
                                        spacesLeft--
                                    else
                                        break
                                }
                                const start = spaceLen*(tabSpaces-spacesLeft)
                                return line.substring(start)
                            })
                            .join('\n')
                        deselect()
                        replace(range.start, range.end, s)
                        select(range.start, range.start + s.length)
                    }
                    else
                    {
                        const end = cursorPosition
                        let start = end
                        let spacesLeft = tabSpaces
                        while(spacesLeft)
                        {
                            const newStart = start - 1
                            if(text.substring(newStart, newStart + 1) === '\t')
                            {
                                start = newStart
                                break
                            }
                            else
                            {
                                newStart = start - spaceChar.length
                                if(text.substring(newStart, newStart + spaceChar.length) !== spaceChar)
                                    break
                                start = newStart
                                spacesLeft--
                            }
                        }
                        if(start !== end)
                            remove(start, end)
                    }

                    return true
                }

                Keys.onPressed: {
                    switch(event.key)
                    {
                        case Qt.Key_Tab:
                            event.accepted = insertTab()
                            break

                        case Qt.Key_Backtab:
                            event.accepted = insertBacktab()
                            break
                    }
                }
            }

            Rectangle {
                id: lineNumbers

                color: gutterBackgroundColor
                Layout.alignment: Qt.AlignTop | Qt.AlignRight
                width: numbersCol.implicitWidth
                height: container.contentHeight
                x: container.contentX

                ColumnLayout {
                    id: numbersCol
                    spacing: 0
                    anchors.fill: parent

                    Repeater {
                        model: textEdit.lineCount
                        delegate: Text {
                            Layout.alignment: Qt.AlignVCenter | Qt.AlignRight
                            text: index + 1
                            font.pointSize: textEdit.font.pointSize
                            font.family: textEdit.font.pointSize
                            color: lineNumber === (index + 1) ? currentLineNumberColor : lineNumbersColor
                            rightPadding: 5
                            leftPadding: 5
                        }
                    }

                    Item {Layout.fillHeight: true}
                }
            }
        }
    }
}

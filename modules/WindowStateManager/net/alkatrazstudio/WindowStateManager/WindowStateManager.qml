/****************************************************************************}
{ WindowStateManager.qml - saves and restores window size and position       }
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

import QtQuick 2.0
import QtQuick.Window 2.2
import net.alkatrazstudio.AppSettings 1.0

QtObject {
    id: root

    property Window win
    property Window prevWin
    property rect lastNormalRect
    property bool needRestoreVisibility : true
    property bool needRestoreRect : true
    property var lastNormalVisibility : null
    readonly property int noVal: -13371337
    property bool isLoading : false
    property bool isRestoring : false
    readonly property var stringVisibilityMap: ({
        windowed: Window.Windowed,
        maximized: Window.Maximized,
        fullscreen: Window.FullScreen
    })

    readonly property AppSettings conf : AppSettings {
        category: win && win.objectName ? `WindowState-${win.objectName}` : null

        property int x : noVal
        property int y : noVal
        property int width : noVal
        property int height  : noVal
        property string visibility : noVal

        Component.onCompleted: root.load()
    }

    function checkWin()
    {
        if(!win)
            return false
        if(!win.objectName)
           return false
        return true
    }

    function load()
    {
        if(!checkWin())
            return

        updateLastNormalRect()

        if(conf.x != noVal)
            lastNormalRect.x = conf.x
        if(conf.y != noVal)
            lastNormalRect.y = conf.y
        if(conf.width != noVal)
            lastNormalRect.width = conf.width
        if(conf.height != noVal)
            lastNormalRect.height = conf.height

        isLoading = true

        lastNormalVisibility = stringToVisibility(conf.visibility)
        if(lastNormalVisibility === null)
            lastNormalVisibility = Window.AutomaticVisibility

        if(!isHidden())
        {
            restoreWinVisibility()
            restoreWinRect()
        }
        else
        {
            needRestoreRect = true
            needRestoreVisibility = true
        }

        if(isMaximized())
            needRestoreRect = true

        isLoading = false
    }

    function save()
    {
        conf.x = lastNormalRect.x
        conf.y = lastNormalRect.y
        conf.width = lastNormalRect.width
        conf.height = lastNormalRect.height
        conf.visibility = visibilityToString(lastNormalVisibility)
        conf.save()
    }

    function visibilityToString(v)
    {
        const res = Object.keys(stringVisibilityMap).find(s => stringVisibilityMap[s] === v)
        return res ? res : null
    }

    function stringToVisibility(s)
    {
        if(stringVisibilityMap.hasOwnProperty(s))
            return stringVisibilityMap[s]
        return null
    }

    function updateLastNormalRect()
    {
        lastNormalRect.x = win.x
        lastNormalRect.y = win.y
        lastNormalRect.width = win.width
        lastNormalRect.height = win.height
    }

    function restoreWinRect()
    {
        isRestoring = true

        win.x = lastNormalRect.x
        win.y = lastNormalRect.y
        win.width = lastNormalRect.width
        win.height = lastNormalRect.height
        needRestoreRect = false

        isRestoring = false
    }

    function restoreWinVisibility()
    {
        if(!checkWin())
            return

        isRestoring = true

        win.visibility = lastNormalVisibility
        needRestoreVisibility = false

        isRestoring = false
    }

    function isMaximized(v)
    {
        if(typeof(v) == 'undefined')
            v = win.visibility
        return v === Window.Maximized
            || v === Window.FullScreen
    }

    function isHidden()
    {
        return visibilityToString(win.visibility) === null
    }

    function onRectChange()
    {
        if(isLoading || isRestoring || isHidden())
            return

        if(needRestoreVisibility)
            restoreWinVisibility()
        else
            lastNormalVisibility = win.visibility

        const wasMax = isMaximized(lastNormalVisibility)

        const isMax = isMaximized()

        if(!isMax && needRestoreRect)
            restoreWinRect()

        if(!wasMax && !isMax)
            updateLastNormalRect()
    }

    function disconnectSignals(w)
    {
        if(!w)
            return
        w.onXChanged.disconnect(onRectChange)
        w.onYChanged.disconnect(onRectChange)
        w.onWidthChanged.disconnect(onRectChange)
        w.onHeightChanged.disconnect(onRectChange)
        w.onVisibilityChanged.disconnect(onRectChange)
    }

    onWinChanged: {
        disconnectSignals(prevWin)
        prevWin = win

        if(win)
        {
            win.onXChanged.connect(onRectChange)
            win.onYChanged.connect(onRectChange)
            win.onWidthChanged.connect(onRectChange)
            win.onHeightChanged.connect(onRectChange)
            win.onVisibilityChanged.connect(onRectChange)
        }
    }

    Component.onDestruction: {
        disconnectSignals(prevWin)
        save()
    }
}

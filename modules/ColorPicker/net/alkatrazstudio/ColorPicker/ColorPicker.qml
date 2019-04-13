/****************************************************************************}
{ ColorPicker.qml - color indicator with a color picker                      }
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
import QtQuick.Layouts 1.3
import QtQuick.Dialogs 1.3

Canvas {
    id: root

    property bool showAlpha: false
    property color curColor
    signal changed(color value)

    Layout.fillHeight: true
    width: 100

    readonly property int checkerSize: width / 10

    onPaint: { // checkerboard
        var ctx = getContext('2d')
        ctx.fillStyle = 'white'
        ctx.fillRect(0, 0, width, height)

        ctx.fillStyle = 'black'
        var yOffset = 0
        for(var x=0; x<width; x+=checkerSize)
        {
            yOffset = yOffset ? 0 : checkerSize
            for(var y=yOffset; y<width; y+=checkerSize*2)
                 ctx.fillRect(x, y, checkerSize, checkerSize)
        }
    }

    Rectangle {
        id: rectangle
        anchors.fill: parent
        border {
            color: 'black'
        }
        color: root.curColor
    }

    ColorDialog {
        property color prevColor

        id: colorNormal
        showAlphaChannel: showAlpha
        modality: Qt.ApplicationModal

        onCurrentColorChanged: changed(currentColor)
        onRejected: changed(prevColor)
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            var c = root.curColor.toString()
            // can't bind because of a bind loop on onRejected()
            colorNormal.prevColor = c
            colorNormal.currentColor = c
            colorNormal.color = c
            colorNormal.open()
            changed(c) // open() resets alpha channel for some reason
        }
    }
}

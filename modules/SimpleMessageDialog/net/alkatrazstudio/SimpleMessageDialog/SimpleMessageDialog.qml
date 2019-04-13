/****************************************************************************}
{ SimpleMessageDialog.qml - dialog box with predefined templates             }
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
import QtQuick.Dialogs 1.3

MessageDialog {
    id: dlg

    property var handlerYes
    property var handlerNo
    property bool isYes
    property bool rejectAsNo: false
    property bool acceptAsYes: false

    icon: StandardIcon.Question
    standardButtons: StandardButton.Yes | StandardButton.No

    onYes: {
        isYes = true
        processHandlers()
    }
    onNo: {
        isYes = false
        processHandlers()
    }
    onAccepted: {
        isYes = true
        processHandlers()
    }
    onRejected: {
        if(rejectAsNo)
        {
            isYes = false
            processHandlers()
        }
    }

    function processHandlers()
    {
        if(isYes)
        {
            if(handlerYes)
                handlerYes()
        }
        else
        {
            if(handlerNo)
                handlerNo()
        }
    }

    function _open(title, text, onYes, onNo)
    {
        dlg.title = title
        dlg.text = text
        handlerYes = onYes
        handlerNo = onNo
        open()
    }

    function warning(title, text, onAccept, onReject)
    {
        icon = StandardIcon.Warning
        standardButtons = StandardButton.Ok
        acceptAsYes = true
        rejectAsNo = true

        _open(title, text, onAccept, onReject)
    }

    function confirm(title, text, onYes, onNo)
    {
        icon = StandardIcon.Question
        standardButtons = StandardButton.Yes | StandardButton.No
        acceptAsYes = false

        _open(title, text, onYes, onNo)
    }
}


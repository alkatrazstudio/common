/****************************************************************************}
{ AppSettings.qml - settings manager QML component                           }
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
import net.alkatrazstudio.AppSettingsImpl 1.0

AppSettingsImpl {
    id: conf

    readonly property QtObject state: QtObject {
        property var changed: ({})
        readonly property var ignore: ['objectName', 'category', 'state']
    }

    function realKeys()
    {
        return Object.keys(conf).filter(key => {
            if(state.ignore.indexOf(key) >= 0)
                return false
            if(typeof(conf[key]) == 'function')
                return false
            return true
        })
    }

    function load()
    {
        realKeys().forEach(key => {
            const type = typeof(conf[key])
            switch(type)
            {
                case 'boolean':
                    conf[key] = getBool(key, conf[key])
                    break

                case 'number':
                    conf[key] = getInt(key, conf[key])
                    break

                case 'string':
                    conf[key] = getString(key, conf[key])
                    break

                default:
                    conf[key] = value(key, conf[key])
                    break
            }
        })

        state.changed = {}
    }

    function save()
    {
        const keys = Object.keys(state.changed)
        if(!keys.length)
            return
        keys.forEach(key => setValue(key, conf[key]))
        sync()
    }

    Component.onCompleted: {
        realKeys().forEach(key => {
            conf[key + 'Changed'].connect(function(){
                state.changed[key] = true
            })
        })
        load()
    }
}

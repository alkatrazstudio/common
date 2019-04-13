/****************************************************************************}
{ Version.qbs - get version info from git and version file                   }
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

import qbs
import qbs.Process

Module {
    readonly property string autoVersion: versionProbe.versionStr
    property string version: product.version || ''
    readonly property var parts: {
        var nParts = 3
        var parts = version.split('.')
        for(var a=0; a<nParts; a++)
            parts[a] = parseInt(parts[a]) || 0
        return parts.splice(0, nParts)
    }
    readonly property int major: parts[0]
    readonly property int minor: parts[1]
    readonly property int patch: parts[2]

    Probe {
        id: versionProbe
        property string versionStr
        configure: {
            var proc = new Process()
            proc.exec('python3', [path+'/version.py'])
            versionStr = proc.readStdOut()
            proc.close()
            found = true
        }
    }
}

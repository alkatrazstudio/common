/****************************************************************************}
{ SimpleMessageDialog.qbs - dialog box with predefined templates             }
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
import qbs.FileInfo

Module {
    Depends {
        name: 'Qt'
        submodules: ['qml']
    }
    Depends {name: 'Common'}

    readonly property string qmlImportPath: path

    Group {
        name: 'SimpleMessageDialog'
        files: ['net/alkatrazstudio/SimpleMessageDialog/SimpleMessageDialog.qml', 'net/alkatrazstudio/SimpleMessageDialog/qmldir']
        qbs.installSourceBase: SimpleMessageDialog.qmlImportPath
        qbs.installDir: FileInfo.relativePath(product.sourceDirectory, SimpleMessageDialog.qmlImportPath)
        qbs.install: Common.realInstall
    }
}

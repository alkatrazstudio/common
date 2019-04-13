/****************************************************************************}
{ PkgConfig.qbs - simplified use of pkg-config                               }
{                                                                            }
{ Copyright (c) 2019 Alexey Parfenov <zxed@alkatrazstudio.net>               }
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
import qbs.Probes

import 'funcs.js' as Funcs

Module {
    property var libName

    Depends {name: 'cpp'}

    Probes.PkgConfigProbe {
        id: lib
        packageNames: libName.constructor === Array ? libName : [libName]
    }

    cpp.includePaths: Funcs.extractPkgConfigItems(lib.cflags, 'I')
    cpp.libraryPaths: Funcs.extractPkgConfigItems(lib.libs, 'L')
    cpp.dynamicLibraries: Funcs.extractPkgConfigItems(lib.libs, 'l')
}

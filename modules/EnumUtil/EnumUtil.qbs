/****************************************************************************}
{ EnumUtil.qbs - converting enums to/from strings                            }
{                                                                            }
{ Copyright (c) 2013 Alexey Parfenov <zxed@alkatrazstudio.net>               }
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

import qbs.FileInfo

Module {
    Depends {name: 'cpp'}
    Depends {
        name: 'Qt'
        submodules: ['core']
    }

    cpp.includePaths: FileInfo.relativePath(product.sourceDirectory, path)

    Group {
        name: 'EnumUtil'
        files: ['enumutil.h']
    }
}

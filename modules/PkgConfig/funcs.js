/****************************************************************************}
{ funcs.js - helpers for PkgConfig.qbs                                       }
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

function extractPkgConfigItems(items, prefix)
{
    var fullPrefix = '-'+prefix
    var lenPrefix = fullPrefix.length
    return items
        .filter(function(s){
            return s.substring(0, lenPrefix) === fullPrefix
        })
        .map(function(s){
            return s.substring(lenPrefix)
        })
}

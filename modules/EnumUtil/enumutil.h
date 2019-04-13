/****************************************************************************}
{ enumutil.h - converting enums to/from strings                              }
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

#pragma once

#include <QMetaEnum>

namespace EnumUtil {
    template<typename T>
    const char* toString(T val, const char* def = "")
    {
        const char* key =
                QMetaEnum::fromType<T>()
                    .valueToKey(
                        static_cast<std::underlying_type_t<T>>(val)
                    );
        return key ? key : def;
    }

    template<typename T>
    const char* toString(T val, T def)
    {
        const char* key = toString(val, nullptr);
        return key ? key : toString(def);
    }

    template<typename T>
    T fromString(const char* key, T def)
    {
        bool ok;
        int val = QMetaEnum::fromType<T>().keyToValue(key, &ok);
        if(!ok)
            return def;
        return static_cast<T>(val);
    }

    template<typename T>
    T fromString(const QString& key, T def)
    {
        return fromString(key.toLatin1().constData(), def);
    }
};

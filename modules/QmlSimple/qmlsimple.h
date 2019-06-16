/****************************************************************************}
{ qmlsimple.h - simplification of properties declaration                     }
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

#pragma once

#define SIMPLE_QML_PROP(type, field) \
    protected: type field##_ {}; \
    public: \
        const type& field() const {return field##_;} \
        void set_##field(type field) { \
            if(field##_ != field) { \
                field##_ = field; \
                emit on_##field##_changed();\
            } \
        } \
        Q_PROPERTY(type field READ field WRITE set_##field NOTIFY on_##field##_changed) \
    private:

#define SIMPLE_QML_OBJ_PROP(type, field) \
    protected: type field##_ {}; \
    public: \
        const type& field() const {return field##_;} \
        void set_##field(const type& field) { \
            if(field##_ != field) { \
                field##_ = field; \
                emit on_##field##_changed();\
            } \
        } \
        Q_PROPERTY(type field READ field WRITE set_##field NOTIFY on_##field##_changed) \
    private:

#define SIMPLE_QML_RO_PROP(type, field) \
    protected: type field##_ {}; \
    public: \
        const type& field() const {return field##_;} \
        void set_##field(type field) { \
            if(field##_ != field) { \
                field##_ = field; \
                emit on_##field##_changed();\
            } \
        } \
        Q_PROPERTY(type field READ field NOTIFY on_##field##_changed) \
    private:

#define SIMPLE_QML_OBJ_RO_PROP(type, field) \
    protected: type field##_ {}; \
    public: \
        const type& field() const {return field##_;} \
        void set_##field(const type& field) { \
            if(field##_ != field) { \
                field##_ = field; \
                emit on_##field##_changed();\
            } \
        } \
        Q_PROPERTY(type field READ field NOTIFY on_##field##_changed) \
    private:

#define SIMPLE_QML_POBJ_PROP(type, field) \
    protected: type* field##_ {}; \
    public: \
        type* field() const {return field##_;} \
        void set_##field(type* field) { \
            if(field##_ != field) { \
                field##_ = field; \
                emit on_##field##_changed(); \
            } \
        } \
        Q_PROPERTY(type* field READ field WRITE set_##field NOTIFY on_##field##_changed) \
    private:

#define SIMPLE_QML_CONST(type, field, val) \
    protected: static constexpr type field = val; \
    public: \
        Q_PROPERTY(type field MEMBER field CONSTANT) \
    private:

#define SIMPLE_QML_CONST_CALC(type, field, expr) \
    protected: type field() const {return expr;} \
    public: \
        Q_PROPERTY(type field READ field CONSTANT) \
    private:

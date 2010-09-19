/* This file is part of the KDE project
 * Copyright (C) 2010 Sebastian Sauer <sebsauer@kdab.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef KACCESSIBLEINTERFACE_H
#define KACCESSIBLEINTERFACE_H

#include <QAccessibleInterface>
#include <QDBusArgument>

struct KAccessibleDBusInterface
{
    QString name;
    QString description;
    QString value;
    QString accelerator;
    QRect rect;

    //QString objectName;
    //QString className;

    enum Type {
        Object,
        Widget,
        Dialog,
        Menu,
        Button,
        Combobox,
        Checkbox,
        Radiobutton,
        Label,
        Listview
    };
    Type type;
    
    QAccessible::State state;
};

Q_DECLARE_METATYPE(KAccessibleDBusInterface)

QDBusArgument &operator<<(QDBusArgument &argument, const KAccessibleDBusInterface &a)
{
    argument.beginStructure();
    argument << a.name << a.description << a.value << a.accelerator << a.rect << int(a.type) << int(a.state);
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KAccessibleDBusInterface &a)
{
    argument.beginStructure();
    int type, state;
    argument >> a.name >> a.description >> a.value >> a.accelerator >> a.rect >> type >> state;
    a.type = (KAccessibleDBusInterface::Type) type;
    a.state = QAccessible::State(state);
    argument.endStructure();
    return argument;
}

#endif

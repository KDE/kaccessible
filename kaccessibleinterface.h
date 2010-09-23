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

/**
 * This class represents the QAccessibleInterface informations of
 * a QObject transported over dbus from the \a Bridge dbus-service to
 * the \a KAccessibleApp application.
 */
class KAccessibleInterface
{
    public:
        QString name;
        QString description;
        QString value;
        QString accelerator;
        QRect rect;

        QString objectName;
        QString className;

        enum Type {
            Object,
            Widget,
            Dialog,
            Menu,
            Button,
            Edit,
            Combobox,
            Checkbox,
            Radiobutton,
            Label,
            Listview
        };
        Type type;

        QAccessible::State state;

        explicit KAccessibleInterface() : type(Object), state(QFlags<QAccessible::StateFlag>()) {}
};

Q_DECLARE_METATYPE(KAccessibleInterface)

QDBusArgument &operator<<(QDBusArgument &argument, const KAccessibleInterface &a)
{
    argument.beginStructure();
    argument << a.name << a.description << a.value << a.accelerator << a.rect << a.objectName << a.className << int(a.type) << int(a.state);
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, KAccessibleInterface &a)
{
    argument.beginStructure();
    int type, state;
    argument >> a.name >> a.description >> a.value >> a.accelerator >> a.rect >> a.objectName >> a.className >> type >> state;
    a.type = (KAccessibleInterface::Type) type;
    a.state = QAccessible::State(state);
    argument.endStructure();
    return argument;
}

QString reasonToString(int reason)
{
    switch(reason) {
        case QAccessible::Focus: return QLatin1String( "Focus" );
        case QAccessible::MenuCommand: return QLatin1String( "MenuCommand" );
        case QAccessible::MenuStart: return QLatin1String( "MenuStart" );
        case QAccessible::MenuEnd: return QLatin1String( "MenuEnd" );
        case QAccessible::PopupMenuEnd: return QLatin1String( "PopupMenuEnd" );
        case QAccessible::PopupMenuStart: return QLatin1String( "PopupMenuStart" );
        case QAccessible::ScrollingEnd: return QLatin1String( "ScrollingEnd" );
        case QAccessible::ScrollingStart: return QLatin1String( "ScrollingStart" );
        case QAccessible::Selection: return QLatin1String( "Selection" );
        case QAccessible::StateChanged: return QLatin1String( "StateChanged" );
        case QAccessible::ValueChanged: return QLatin1String( "ValueChanged" );
        case QAccessible::NameChanged: return QLatin1String( "NameChanged" );
        case QAccessible::ObjectCreated: return QLatin1String( "ObjectCreated" );
        case QAccessible::ObjectDestroyed: return QLatin1String( "ObjectDestroyed" );
        case QAccessible::ObjectHide: return QLatin1String( "ObjectHide" );
        case QAccessible::ObjectReorder: return QLatin1String( "ObjectReorder" );
        case QAccessible::ObjectShow: return QLatin1String( "ObjectShow" );
        case QAccessible::ParentChanged: return QLatin1String( "ParentChanged" );
        case QAccessible::Alert: return QLatin1String( "Alert" );
        case QAccessible::DefaultActionChanged: return QLatin1String( "DefaultActionChanged" );
        case QAccessible::DialogEnd: return QLatin1String( "DialogEnd" );
        case QAccessible::DialogStart: return QLatin1String( "DialogStart" );
        case QAccessible::DragDropEnd: return QLatin1String( "DragDropEnd" );
        case QAccessible::DragDropStart: return QLatin1String( "DragDropStart" );
        case QAccessible::ForegroundChanged: return QLatin1String( "ForegroundChanged" );
        case QAccessible::LocationChanged: return QLatin1String( "LocationChanged" );
        case QAccessible::SelectionAdd: return QLatin1String( "SelectionAdd" );
        case QAccessible::SelectionRemove: return QLatin1String( "SelectionRemove" );
        case QAccessible::SelectionWithin: return QLatin1String( "SelectionWithin" );
    }
    return QString::number(reason);
}

QString stateToString(QAccessible::State flags)
{
    QString result;
    if(flags & QAccessible::Animated) result += QLatin1String( "Animated " );
    if(flags & QAccessible::Busy) result += QLatin1String( "Busy " );
    if(flags & QAccessible::Checked) result += QLatin1String( "Checked " );
    if(flags & QAccessible::Collapsed) result += QLatin1String( "Collapsed " );
    if(flags & QAccessible::DefaultButton) result += QLatin1String( "DefaultButton " );
    if(flags & QAccessible::Expanded) result += QLatin1String( "Expanded " );
    if(flags & QAccessible::ExtSelectable) result += QLatin1String( "ExtSelectable " );
    if(flags & QAccessible::Focusable) result += QLatin1String( "Focusable " );
    if(flags & QAccessible::Focused) result += QLatin1String( "Focused " );
    if(flags & QAccessible::HasPopup) result += QLatin1String( "HasPopup " );
    if(flags & QAccessible::HotTracked) result += QLatin1String( "HotTracked " );
    if(flags & QAccessible::Invisible) result += QLatin1String( "Invisible " );
    if(flags & QAccessible::Linked) result += QLatin1String( "Linked " );
    if(flags & QAccessible::Marqueed) result += QLatin1String( "Marqueed " );
    if(flags & QAccessible::Mixed) result += QLatin1String( "Mixed " );
    if(flags & QAccessible::Modal) result += QLatin1String( "Modal " );
    if(flags & QAccessible::Movable) result += QLatin1String( "Movable " );
    if(flags & QAccessible::MultiSelectable) result += QLatin1String( "MultiSelectable " );
    if(flags & QAccessible::Normal) result += QLatin1String( "Normal " );
    if(flags & QAccessible::Offscreen) result += QLatin1String( "Offscreen " );
    if(flags & QAccessible::Pressed) result += QLatin1String( "Pressed " );
    if(flags & QAccessible::Protected) result += QLatin1String( "Protected " );
    if(flags & QAccessible::ReadOnly) result += QLatin1String( "ReadOnly " );
    if(flags & QAccessible::Selectable) result += QLatin1String( "Selectable " );
    if(flags & QAccessible::Selected) result += QLatin1String( "Selected " );
    if(flags & QAccessible::SelfVoicing) result += QLatin1String( "SelfVoicing " );
    if(flags & QAccessible::Sizeable) result += QLatin1String( "Sizeable " );
    if(flags & QAccessible::Traversed) result += QLatin1String( "Traversed " );
    if(flags & QAccessible::Unavailable) result += QLatin1String( "Unavailable " );
    return result.trimmed();
}

QString typeToString(int type)
{
    switch(type) {
        case KAccessibleInterface::Object: return QLatin1String( "Object" );
        case KAccessibleInterface::Widget: return QLatin1String( "Widget" );
        case KAccessibleInterface::Dialog: return QLatin1String( "Dialog" );
        case KAccessibleInterface::Menu: return QLatin1String( "Menu" );
        case KAccessibleInterface::Button: return QLatin1String( "Button" );
        case KAccessibleInterface::Edit: return QLatin1String( "Edit" );
        case KAccessibleInterface::Combobox: return QLatin1String( "Combobox" );
        case KAccessibleInterface::Checkbox: return QLatin1String( "Checkbox" );
        case KAccessibleInterface::Radiobutton: return QLatin1String( "Radiobutton" );
        case KAccessibleInterface::Label: return QLatin1String( "Label" );
        case KAccessibleInterface::Listview: return QLatin1String( "Listview" );
    }
    return QString::number(type);
}

#endif

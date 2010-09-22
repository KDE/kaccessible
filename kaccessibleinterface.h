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
        case QAccessible::Focus: return "Focus";
        case QAccessible::MenuCommand: return "MenuCommand";
        case QAccessible::MenuStart: return "MenuStart";
        case QAccessible::MenuEnd: return "MenuEnd";
        case QAccessible::PopupMenuEnd: return "PopupMenuEnd";
        case QAccessible::PopupMenuStart: return "PopupMenuStart";
        case QAccessible::ScrollingEnd: return "ScrollingEnd";
        case QAccessible::ScrollingStart: return "ScrollingStart";
        case QAccessible::Selection: return "Selection";
        case QAccessible::StateChanged: return "StateChanged";
        case QAccessible::ValueChanged: return "ValueChanged";
        case QAccessible::NameChanged: return "NameChanged";
        case QAccessible::ObjectCreated: return "ObjectCreated";
        case QAccessible::ObjectDestroyed: return "ObjectDestroyed";
        case QAccessible::ObjectHide: return "ObjectHide";
        case QAccessible::ObjectReorder: return "ObjectReorder";
        case QAccessible::ObjectShow: return "ObjectShow";
        case QAccessible::ParentChanged: return "ParentChanged";
        case QAccessible::Alert: return "Alert";
        case QAccessible::DefaultActionChanged: return "DefaultActionChanged";
        case QAccessible::DialogEnd: return "DialogEnd";
        case QAccessible::DialogStart: return "DialogStart";
        case QAccessible::DragDropEnd: return "DragDropEnd";
        case QAccessible::DragDropStart: return "DragDropStart";
        case QAccessible::ForegroundChanged: return "ForegroundChanged";
        case QAccessible::LocationChanged: return "LocationChanged";
        case QAccessible::SelectionAdd: return "SelectionAdd";
        case QAccessible::SelectionRemove: return "SelectionRemove";
        case QAccessible::SelectionWithin: return "SelectionWithin";
    }
    return QString::number(reason);
}

QString stateToString(QAccessible::State flags)
{
    QString result;
    if(flags & QAccessible::Animated) result += "Animated ";
    if(flags & QAccessible::Busy) result += "Busy ";
    if(flags & QAccessible::Checked) result += "Checked ";
    if(flags & QAccessible::Collapsed) result += "Collapsed ";
    if(flags & QAccessible::DefaultButton) result += "DefaultButton ";
    if(flags & QAccessible::Expanded) result += "Expanded ";
    if(flags & QAccessible::ExtSelectable) result += "ExtSelectable ";
    if(flags & QAccessible::Focusable) result += "Focusable ";
    if(flags & QAccessible::Focused) result += "Focused ";
    if(flags & QAccessible::HasPopup) result += "HasPopup ";
    if(flags & QAccessible::HotTracked) result += "HotTracked ";
    if(flags & QAccessible::Invisible) result += "Invisible ";
    if(flags & QAccessible::Linked) result += "Linked ";
    if(flags & QAccessible::Marqueed) result += "Marqueed ";
    if(flags & QAccessible::Mixed) result += "Mixed ";
    if(flags & QAccessible::Modal) result += "Modal ";
    if(flags & QAccessible::Movable) result += "Movable ";
    if(flags & QAccessible::MultiSelectable) result += "MultiSelectable ";
    if(flags & QAccessible::Normal) result += "Normal ";
    if(flags & QAccessible::Offscreen) result += "Offscreen ";
    if(flags & QAccessible::Pressed) result += "Pressed ";
    if(flags & QAccessible::Protected) result += "Protected ";
    if(flags & QAccessible::ReadOnly) result += "ReadOnly ";
    if(flags & QAccessible::Selectable) result += "Selectable ";
    if(flags & QAccessible::Selected) result += "Selected ";
    if(flags & QAccessible::SelfVoicing) result += "SelfVoicing ";
    if(flags & QAccessible::Sizeable) result += "Sizeable ";
    if(flags & QAccessible::Traversed) result += "Traversed ";
    if(flags & QAccessible::Unavailable) result += "Unavailable ";
    return result.trimmed();
}

QString typeToString(int type)
{
    switch(type) {
        case KAccessibleInterface::Object: return "Object";
        case KAccessibleInterface::Widget: return "Widget";
        case KAccessibleInterface::Dialog: return "Dialog";
        case KAccessibleInterface::Menu: return "Menu";
        case KAccessibleInterface::Button: return "Button";
        case KAccessibleInterface::Edit: return "Edit";
        case KAccessibleInterface::Combobox: return "Combobox";
        case KAccessibleInterface::Checkbox: return "Checkbox";
        case KAccessibleInterface::Radiobutton: return "Radiobutton";
        case KAccessibleInterface::Label: return "Label";
        case KAccessibleInterface::Listview: return "Listview";
    }
    return QString::number(type);
}

#endif

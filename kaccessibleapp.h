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
#ifndef KACCESSIBLEAPP_H
#define KACCESSIBLEAPP_H

#include <QDBusAbstractAdaptor>
#include <QDebug>
#include <KUniqueApplication>

class Adaptor : public QObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.kaccessibleapp.Adaptor")
    public:
        explicit Adaptor(QObject* parent = 0) : QObject(parent) {}
        virtual ~Adaptor() {}
    Q_SIGNALS:
        void focusChanged(/*QObject *obj,*/ int x, int y);
    public Q_SLOTS:
        void setFocusChanged(/*QObject *obj,*/ int x, int y) { emit focusChanged(/*obj,*/ x, y); }
};

class KAccessibleApp : public KUniqueApplication
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.kaccessibleapp")
    public:
        KAccessibleApp();
        virtual ~KAccessibleApp();
};

#endif

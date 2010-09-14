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

/**
 * THe Adaptor class provides a dbus interface for the KAccessibleApp .
 */
class Adaptor : public QObject
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.kaccessibleapp.Adaptor")
    public:
        explicit Adaptor(QObject *parent = 0);
        virtual ~Adaptor();

    Q_SIGNALS:

        /**
         * This signal is emitted if the position of the focus changed. This can
         * be used by client-applications to implement focus tracking.
         *
         * The \p px and \p py arguments can be either undefined, both will be -1, or
         * can define the exact focus point. Additionally provided is a rectangle
         * defined with the start-point \p rx and \p rx and the dimension \p rwidth
         * and \p rheight . That rectangle defines the focus area.
         */
        void focusChanged(int px, int py, int rx, int ry, int rwidth, int rheight);

    public Q_SLOTS:

        /**
         * This method can be called to emit the \a focusChanged signal above.
         */
        void setFocusChanged(int px, int py, int rx, int ry, int rwidth, int rheight, const QString& text = QString());

        /**
         * Text-to-Speech interface.
         */
        void sayText(const QString& text);

        /**
         * Returns true if automatic text-to-speech is enabled or false if disabled.
         */
        bool speechEnabled() const;

        /**
         * Enable or disable automatic text-to-speech.
         * 
         * If enabled then for example the text passed as argument at the \a setFocusChanged method
         * will be read. Note that this has no effect on \a sayText which is always enabled. This
         * setting only enables/disables automatic screen reading.
         */
        void setSpeechEnabled(bool enabled);
        
    private:
        class Private;
        Private *const d;
};

/**
 * The unique application instance that will be responsible for redirecting
 * stuff around.
 */
class KAccessibleApp : public KUniqueApplication
{
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.kde.kaccessibleapp")
    public:
        KAccessibleApp();
        virtual ~KAccessibleApp();
};

#endif

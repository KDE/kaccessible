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
#include <KAction>
#include <KMainWindow>
#include <KUniqueApplication>

/**
 * Highlevel text-to-speech interface.
 */
class Speaker : public QObject
{
        Q_OBJECT
    public:
        static Speaker* instance();

        bool isConnected() const;
        void disconnect();
        bool reconnect();

        bool isSpeaking() const;
        void setSpeaking(bool speaking);

        void cancel();

        enum Priority {
            Important = 1,
            Message = 2,
            Text = 3,
            Notification = 4,
            Progress = 5
        };

        bool say(const QString& text, Priority priority = Text);

        char** modules() const;
        char** voices() const;
        QStringList languages() const;
        
        int voiceType() const;
        void setVoiceType(int type);

        explicit Speaker();
        ~Speaker();
    private slots:
        void sayNext();
        void clearSayStack();
    private:
        class Private;
        Private *const d;
};

class KAccessibleInterface;

/**
 * The Adaptor class provides a dbus interface for the KAccessibleApp .
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

        //void valueChanged(const QString& name, const QString& value);
        //void alert(const QString& name);

        /**
         * This signal is emitted if speech was enabled/disabled using \a setSpeechEnabled .
         */
        void speechEnabledChanged(bool enabled);

        /**
         *
         */
        void notified(int reason, const KAccessibleInterface& iface);

    public Q_SLOTS:

//void notify(int reason, const KAccessibleInterface& iface);

        /**
         * This method is called if the focus changed.
         * The method emits the \a focusChanged signal above.
         */
        void setFocusChanged(const KAccessibleInterface& iface);

        /**
         * This method is called if a value changed.
         */
        void setValueChanged(const KAccessibleInterface& iface);

        /**
         * This method is called if an alert happens.
         */
        void setAlert(const KAccessibleInterface& iface);

        /**
         * This method can be called to use the text-to-speech interface to say something.
         */
        void sayText(const QString& text, int priority = 3);

        /**
         * Returns true if automatic text-to-speech is enabled or false if disabled.
         */
        bool speechEnabled() const;

        /**
         * Enable or disable text-to-speech.
         * 
         * If enabled then for example the text passed as argument at the \a setFocusChanged method
         * will be read.
         */
        void setSpeechEnabled(bool enabled);

        int voiceType() const;
        void setVoiceType(int type);

        //void cancelSpeech();
        //void speechPaused();
        //void pauseSpeech();
        //void resumeSpeech();
        
    private:
        class Private;
        Private *const d;
};

class KAccessibleApp;

class MainWindow : public KMainWindow
{
        Q_OBJECT
    public:
        MainWindow(KAccessibleApp *app);
        virtual ~MainWindow();
        virtual void show();
        virtual void hide();
        bool queryExit();
        bool queryClose();
    private Q_SLOTS:
        void notified(int reason, const KAccessibleInterface& iface);
        void enableLogs(int state);
        void enableReaderChanged(int state);
        void voiceTypeChanged(int index);
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
        Adaptor* adaptor() const;
        KAction* action(const QString &name) const;
    private Q_SLOTS:
        void enableScreenreader(bool enabled);
        void speakClipboard();
        void speakText();
    private:
        class Private;
        Private *const d;
};

#endif

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

#include "kaccessibleapp.h"
#include "kaccessibleinterface.h"

#include <QMainWindow>
#include <QMenu>
#include <QLayout>
#include <QTimer>
#include <QStack>
#include <QTextStream>
#include <QLabel>
#include <QCheckBox>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QClipboard>
#include <QMutex>
#include <QMutexLocker>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <kmainwindow.h>
#include <kconfig.h>
#include <kcombobox.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <kicon.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kinputdialog.h>
#include <kaction.h>
#include <ktoggleaction.h>
#include <kactioncollection.h>
#include <ksystemtrayicon.h>
#include <kdebug.h>
#include <kpagewidget.h>
#include <kpagewidgetmodel.h>

#if defined(SPEECHD_FOUND)
#include <libspeechd.h>
#endif

Q_GLOBAL_STATIC(Speaker, speaker)

class Speaker::Private
{
    public:
        bool m_isSpeaking;
        int m_voiceType;
        QStack< QPair<QString,Speaker::Priority> > m_sayStack;
        QMutex m_mutex;
#if defined(SPEECHD_FOUND)
        SPDConnection *m_connection;
#endif
        explicit Private()
            : m_isSpeaking(false)
            , m_voiceType(1)
#if defined(SPEECHD_FOUND)
            , m_connection(0)
#endif
        {
        }
#if defined(SPEECHD_FOUND)
        static void speechdCallback(size_t msg_id, size_t client_id, SPDNotificationType state)
        {
            Q_UNUSED(msg_id);
            Q_UNUSED(client_id);
            switch(state) {
                case SPD_EVENT_BEGIN:
                    Speaker::instance()->setSpeaking(true);
                    break;
                case SPD_EVENT_END:
                    Speaker::instance()->setSpeaking(false);
                    QTimer::singleShot(0, Speaker::instance(), SLOT(sayNext()));
                    break;
                case SPD_EVENT_CANCEL:
                    Speaker::instance()->setSpeaking(false);
                    Speaker::instance()->clearSayStack();
                    break;
                case SPD_EVENT_PAUSE:
                    break;
                case SPD_EVENT_RESUME:
                    break;
                case SPD_EVENT_INDEX_MARK:
                    break;
            }
        }
#endif
};

Speaker::Speaker()
    : d(new Private)
{
}

Speaker::~Speaker()
{
    disconnect();
    delete d;
}

Speaker* Speaker::instance()
{
    return speaker();
}

bool Speaker::isConnected() const
{
#if defined(SPEECHD_FOUND)
    return d->m_connection;
#else
    return true;
#endif
}

void Speaker::disconnect()
{
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        spd_set_notification_off(d->m_connection, SPD_BEGIN);
        spd_set_notification_off(d->m_connection, SPD_END);
        spd_set_notification_off(d->m_connection, SPD_CANCEL);
        spd_set_notification_off(d->m_connection, SPD_PAUSE);
        spd_set_notification_off(d->m_connection, SPD_RESUME);
        d->m_connection->callback_begin = d->m_connection->callback_end = d->m_connection->callback_cancel = d->m_connection->callback_pause = d->m_connection->callback_resume = 0;
        spd_cancel_all(d->m_connection);
        spd_close(d->m_connection);
        d->m_connection = 0;
        d->m_isSpeaking = false;
        d->m_sayStack.clear();
    }
#endif
}

bool Speaker::reconnect()
{
    disconnect();

#if defined(SPEECHD_FOUND)
    d->m_connection = spd_open("kaccessible", "main", NULL, SPD_MODE_THREADED); //SPD_MODE_SINGLE);
    if( ! d->m_connection) {
        kWarning() << "Failed to connect with speech-dispatcher";
        return false;
    }

    d->m_connection->callback_begin = d->m_connection->callback_end = d->m_connection->callback_cancel = d->m_connection->callback_pause = d->m_connection->callback_resume = Private::speechdCallback;
    spd_set_notification_on(d->m_connection, SPD_BEGIN);
    spd_set_notification_on(d->m_connection, SPD_END);
    spd_set_notification_on(d->m_connection, SPD_CANCEL);
    spd_set_notification_on(d->m_connection, SPD_PAUSE);
    spd_set_notification_on(d->m_connection, SPD_RESUME);
#endif

    setVoiceType(d->m_voiceType);
    return true;
}

bool Speaker::isSpeaking() const
{
    return d->m_isSpeaking;
}

void Speaker::setSpeaking(bool speaking)
{
    d->m_isSpeaking = speaking;
}

void Speaker::cancel()
{
    QMutexLocker locker(&d->m_mutex);
    d->m_sayStack.clear();
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        spd_cancel_all(d->m_connection);
    }
#endif
}

bool Speaker::say(const QString& text, Priority priority)
{
    QMutexLocker locker(&d->m_mutex);
    if(!isConnected())
        return false;
    d->m_sayStack.push( QPair<QString,Speaker::Priority>(text,priority) );
    if(!d->m_isSpeaking)
        QTimer::singleShot(0, this, SLOT(sayNext()));
    return true;
}

void Speaker::sayNext()
{
    QMutexLocker locker(&d->m_mutex);
    if(d->m_sayStack.isEmpty()) {
        return;
    }
    QPair<QString,Speaker::Priority> p = d->m_sayStack.pop();
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        SPDPriority spdpriority = (SPDPriority) p.second;
        int msg_id  = spd_say(d->m_connection, spdpriority, p.first.toUtf8().data());
        if(msg_id == -1) {
            kWarning() << "Failed to say text=" << p.first;
        }
    }
#else
    //QDBusInterface iface("org.kde.jovie","/KSpeech");
    //iface.asyncCall("say", text, 0);
#endif
}

char** Speaker::modules() const
{
#if defined(SPEECHD_FOUND)
    if(d->m_connection)
        return spd_list_modules(d->m_connection);
#endif
    return NULL;
}

char** Speaker::voices() const
{
#if defined(SPEECHD_FOUND)
    if(d->m_connection)
        return spd_list_voices(d->m_connection);
#endif
    return NULL;
}

QStringList Speaker::languages() const
{
    QStringList result;
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        SPDVoice** voices = spd_list_synthesis_voices(d->m_connection);
        while(voices && voices[0]) {
            const QString lng = QString::fromLatin1(voices[0]->language);
            if(!lng.isEmpty() && !result.contains(lng)) result.append(lng);
            ++voices;
        }
    }
#endif
    return result;
}

int Speaker::voiceType() const
{
    return d->m_voiceType;
}

void Speaker::setVoiceType(int type)
{
    d->m_voiceType = type;
#if defined(SPEECHD_FOUND)
    if(d->m_connection) {
        spd_set_voice_type_all(d->m_connection, (SPDVoiceType) type);
    }
#endif
}

void Speaker::clearSayStack()
{
    d->m_sayStack.clear();
}

class Adaptor::Private
{
    public:
        bool m_speechEnabled;
        explicit Private() : m_speechEnabled(false) {}
};

Adaptor::Adaptor(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    KConfig config(QLatin1String( "kaccessibleapp" ));
    KConfigGroup group = config.group("Main");
    d->m_speechEnabled = group.readEntry("SpeechEnabled", d->m_speechEnabled);

    const int prevVoiceType = Speaker::instance()->voiceType();
    const int newVoiceType = group.readEntry("VoiceType", prevVoiceType);
    if(prevVoiceType != newVoiceType)
        Speaker::instance()->setVoiceType(newVoiceType);
}

Adaptor::~Adaptor()
{
    delete d;
}

void Adaptor::setFocusChanged(const KAccessibleInterface& iface)
{
    int px = -1;
    int py = -1;
    QRect r = iface.rect;
    emit focusChanged(px, py, r.x(), r.y(), r.width(), r.height());

    emit notified(QAccessible::Focus, iface);

    QString text = iface.name;
    /*
    if(!iface.accelerator.isEmpty()) {
        QString s = iface.accelerator;
        s = s.replace("+"," ");
        text += " " + s;
    }
    */
    sayText(text);
}

void Adaptor::setValueChanged(const KAccessibleInterface& iface)
{
    sayText(iface.value);
}

void Adaptor::setAlert(const KAccessibleInterface& iface)
{
    Speaker::instance()->cancel();
    sayText(iface.name, int(Speaker::Message));
}

void Adaptor::sayText(const QString& text, int priority)
{
    if(d->m_speechEnabled && !text.isEmpty() && (Speaker::instance()->isConnected() || Speaker::instance()->reconnect())) {
        Speaker::instance()->say(text, Speaker::Priority(priority));
    }
}

bool Adaptor::speechEnabled() const
{
    return d->m_speechEnabled;
}

void Adaptor::setSpeechEnabled(bool enabled)
{
    if(d->m_speechEnabled == enabled)
        return;

    d->m_speechEnabled = enabled;

    KConfig config(QLatin1String( "kaccessibleapp" ));
    KConfigGroup group = config.group("Main");
    group.writeEntry("SpeechEnabled", d->m_speechEnabled);

    if(!d->m_speechEnabled) {
        Speaker::instance()->cancel();
    }

    emit speechEnabledChanged(d->m_speechEnabled);
}

int Adaptor::voiceType() const
{
    return Speaker::instance()->voiceType();
}

void Adaptor::setVoiceType(int type)
{
    if(type == Speaker::instance()->voiceType())
        return;

    KConfig config(QLatin1String( "kaccessibleapp" ));
    KConfigGroup group = config.group("Main");
    group.writeEntry("VoiceType", type);

    Speaker::instance()->setVoiceType(type);
}

class KAccessibleApp::Private
{
    public:
        Adaptor *m_adaptor;
        QMap<QString, KAction*> m_collection;
        explicit Private() : m_adaptor(0) {}
        ~Private() { qDeleteAll(m_collection); }
};

KAccessibleApp::KAccessibleApp()
    : KUniqueApplication()
    , d(new Private)
{
    qDBusRegisterMetaType<KAccessibleInterface>();

    setWindowIcon(KIcon(QLatin1String( "preferences-desktop-accessibility" )));
    setQuitOnLastWindowClosed(false);

    d->m_adaptor = new Adaptor(this);
    if( ! QDBusConnection::sessionBus().registerObject(QLatin1String( "/Adaptor" ), d->m_adaptor, QDBusConnection::ExportAllContents)) {
        kWarning() << "Unable to register KAccessibleApp to dbus";
        QTimer::singleShot(0, this, SLOT(quit()));
    } else {
        KToggleAction* enableScreenreaderAction = new KToggleAction(this);
        enableScreenreaderAction->setText(i18n("Enable Screenreader"));
        enableScreenreaderAction->setIcon(KIcon(QLatin1String( "text-speak" )));
        enableScreenreaderAction->setChecked(d->m_adaptor->speechEnabled());
        connect(enableScreenreaderAction, SIGNAL(triggered(bool)), this, SLOT(enableScreenreader(bool)));
        connect(d->m_adaptor, SIGNAL(speechEnabledChanged(bool)), enableScreenreaderAction, SLOT(setChecked(bool)));
        d->m_collection.insert(QLatin1String( "enableScreenreader" ), enableScreenreaderAction);

        KAction* speakTextAction = new KAction(this);
        speakTextAction->setText(i18n("Speak Text..."));
        speakTextAction->setIcon(KIcon(QLatin1String( "text-plain" )));
        connect(speakTextAction, SIGNAL(triggered(bool)), this, SLOT(speakText()));
        d->m_collection.insert(QLatin1String( "speakText" ), speakTextAction);

        KAction* speakClipboardAction = new KAction(this);
        speakClipboardAction->setText(i18n("Speak Clipboard"));
        speakClipboardAction->setIcon(KIcon(QLatin1String( "klipper" )));
        connect(speakClipboardAction, SIGNAL(triggered(bool)), this, SLOT(speakClipboard()));
        d->m_collection.insert(QLatin1String( "speakClipboard" ), speakClipboardAction);
    }
}

KAccessibleApp::~KAccessibleApp()
{
    delete d;
}

Adaptor* KAccessibleApp::adaptor() const
{
    return d->m_adaptor;
}

KAction* KAccessibleApp::action(const QString &name) const
{
    return d->m_collection.contains(name) ? d->m_collection[name] : 0;
}

void KAccessibleApp::enableScreenreader(bool enabled)
{
    d->m_adaptor->setSpeechEnabled(enabled);
}

void KAccessibleApp::speakClipboard()
{
    const QString text = clipboard()->text();
    if(!text.isEmpty()) {
        //Speaker::instance()->cancel();
        Speaker::instance()->say(text);
    }
}

void KAccessibleApp::speakText()
{
    const QString text = KInputDialog::getText(i18n("Speak Text"), i18n("Type the text and press OK to speak the text."));
    if(!text.isEmpty()) {
        //Speaker::instance()->cancel();
        Speaker::instance()->say(text);
    }
}

/// class for the icon shown in the systemtray.
class SystemTray : public KSystemTrayIcon
{
    public:
        explicit SystemTray(KAccessibleApp *app, QWidget* parent)
            : KSystemTrayIcon(QLatin1String( "preferences-desktop-accessibility" ), parent)
        {
            //QAction *titleAction = contextMenuTitle();
            //titleAction->setText(i18n("Accessibility Bridge"));
            //titleAction->setIcon(KIcon("preferences-desktop-accessibility"));
            //setContextMenuTitle(titleAction);

            //QAction* fileQuitAction = actionCollection()->action("file_quit");
            //if(fileQuitAction) delete actionCollection()->takeAction(fileQuitAction);

            foreach(const QString &name, QStringList() << QLatin1String( "enableScreenreader" ) << QLatin1String( "speakText" ) << QLatin1String( "speakClipboard" ))
                if(KAction* action = app->action(name))
                    contextMenu()->addAction(action);

            //QMenu *popup = dynamic_cast<QMenu*>( KAccessibleApp::App->factory()->container("systemtray_actions", KAccessibleApp::App) );
            //if (popup) contextMenu()->addActions( popup->actions() );
        }
        virtual ~SystemTray()
        {
        }
    protected:
         bool event(QEvent *event)
         {
             /*
             if( event->type() == QEvent::ToolTip ) {
                 QHelpEvent *helpEvent = static_cast<QHelpEvent*>(event);
                 QToolTip::showText(helpEvent->globalPos(), QString("<b>Accessibility Bridge</b><br>%1").arg(tooltip));
                 //QToolTip::hideText();
              }
              */
              return KSystemTrayIcon::event(event);
         }
};

class MainWindow::Private
{
    public:
        KAccessibleApp *m_app;
        Adaptor *m_adaptor;
        SystemTray *m_systemtray;
        KPageWidget *m_pageTab;
        KPageWidgetModel *m_pageModel;
        KComboBox* m_voiceTypeCombo;
        QTreeWidget *m_logs;
        bool m_hideMainWin;
        bool m_logEnabled;

        explicit Private(KAccessibleApp *app) : m_app(app), m_adaptor(app->adaptor()), m_systemtray(0), m_pageTab(0), m_pageModel(0), m_voiceTypeCombo(0), m_logs(0), m_hideMainWin(false), m_logEnabled(false) {}

        void addPage(QWidget* page, const QIcon& iconset, const QString& label)
        {
            QWidget* p = dynamic_cast<QWidget*>(page);
            if (!p) return;
            KPageWidgetItem* item = m_pageModel->addPage(p, QLatin1String( "" )); //p->objectName());
            item->setName(label);
            item->setHeader(QLatin1String( "" )); // hide the header
            item->setIcon(KIcon(iconset));
            QModelIndex index = m_pageModel->index(item);
            Q_ASSERT(index.isValid());
            m_pageModel->setData(index, QVariant(), Qt::DisplayRole);
            m_pageModel->setData(index, QVariant(), KPageModel::HeaderRole);
            //pages.append(page);
            //page->plugGenericActions(this, SLOT(pageActionsChanged(KMLDonkeyPage*)));
        }
};

MainWindow::MainWindow(KAccessibleApp *app)
    : KMainWindow()
    , d(new Private(app))
{
    KConfig config(QLatin1String( "kaccessibleapp" ));
    KConfigGroup group = config.group("Main");
    d->m_logEnabled = group.readEntry("LogEnabled", d->m_logEnabled);

    const int prevVoiceType = Speaker::instance()->voiceType();
    const int newVoiceType = group.readEntry("VoiceType", prevVoiceType);
    if(prevVoiceType != newVoiceType)
        Speaker::instance()->setVoiceType(newVoiceType);

    d->m_systemtray = new SystemTray(app, this);
    d->m_systemtray->show();

    d->m_pageTab = new KPageWidget(this);
    d->m_pageTab->setFaceType( KPageView::Tabbed ); //Auto,Plain,List,Tree,Tabbed
    d->m_pageTab->layout()->setMargin(0);

    d->m_pageModel = new KPageWidgetModel(d->m_pageTab);
    d->m_pageTab->setModel(d->m_pageModel);

    QWidget* readerPage = new QWidget(d->m_pageTab);
    QGridLayout* readerLayout = new QGridLayout(readerPage);
    readerLayout->setMargin(0);
    readerPage->setLayout(readerLayout);
    QCheckBox *enableReader = new QCheckBox(i18n("Enable Screenreader"));
    readerLayout->addWidget(enableReader,0,0,1,2);
#if defined(SPEECHD_FOUND)
    enableReader->setChecked(d->m_adaptor->speechEnabled());
    connect(d->m_adaptor, SIGNAL(speechEnabledChanged(bool)), enableReader, SLOT(setChecked(bool)));
    connect(enableReader, SIGNAL(stateChanged(int)), this, SLOT(enableReaderChanged(int)));

    QLabel *voiceTypeLabel = new QLabel(i18n("Voice Type:"), readerPage);
    readerLayout->addWidget(voiceTypeLabel,1,0);
    d->m_voiceTypeCombo = new KComboBox(this);
    voiceTypeLabel->setBuddy(d->m_voiceTypeCombo);
    d->m_voiceTypeCombo->addItem(i18n("Male 1"), SPD_MALE1);
    d->m_voiceTypeCombo->addItem(i18n("Male 2"), SPD_MALE2);
    d->m_voiceTypeCombo->addItem(i18n("Male 3"), SPD_MALE3);
    d->m_voiceTypeCombo->addItem(i18n("Female 1"), SPD_FEMALE1);
    d->m_voiceTypeCombo->addItem(i18n("Female 2"), SPD_FEMALE2);
    d->m_voiceTypeCombo->addItem(i18n("Female 3"), SPD_FEMALE3);
    d->m_voiceTypeCombo->addItem(i18n("Boy"), SPD_CHILD_MALE);
    d->m_voiceTypeCombo->addItem(i18n("Girl"), SPD_CHILD_FEMALE);
    for(int i = 0; i < d->m_voiceTypeCombo->count(); ++i)
        if(d->m_voiceTypeCombo->itemData(i).toInt() == Speaker::instance()->voiceType())
            d->m_voiceTypeCombo->setCurrentIndex(i);
    connect(d->m_voiceTypeCombo, SIGNAL(activated(int)), this, SLOT(voiceTypeChanged(int)));
    readerLayout->addWidget(d->m_voiceTypeCombo,1,1);

    readerLayout->setRowStretch(2,1);
#else
    enableReader->setEnabled(false);
    readerLayout->setRowStretch(1,1);
#endif
    readerLayout->setColumnStretch(2,1);
    d->addPage(readerPage, KIcon(QLatin1String( "text-speak" )), i18n("Screenreader"));

    QWidget* logsPage = new QWidget(d->m_pageTab);
    QVBoxLayout* logsLayout = new QVBoxLayout(logsPage);
    logsLayout->setMargin(0);
    logsPage->setLayout(logsLayout);
    QCheckBox *enableLogsCheckbox = new QCheckBox(i18n("Enable Logs"));
    logsLayout->addWidget(enableLogsCheckbox);
    d->m_logs = new QTreeWidget(logsPage);
    d->m_logs->setColumnCount(10);
    d->m_logs->setHeaderLabels(QStringList() << i18n("Reason") << i18n("Type") << i18n("Class") << i18n("Name") << i18n("Value") << i18n("Accelerator") << i18n("State") << i18n("Rect") << i18n("Object") <<  i18n("Description"));
    d->m_logs->setRootIsDecorated(false);
    d->m_logs->setEnabled(false);
    if(d->m_logEnabled) {
        enableLogsCheckbox->setChecked(Qt::Checked);
        enableLogs(Qt::Checked);
    }
    connect(enableLogsCheckbox, SIGNAL(stateChanged(int)), this, SLOT(enableLogs(int)));
    logsLayout->addWidget(d->m_logs);
    d->addPage(logsPage, KIcon(QLatin1String( "view-list-text" )), i18n("Logs"));

    setCentralWidget(d->m_pageTab);
    resize(QSize(460, 320).expandedTo(minimumSizeHint()));
    setAutoSaveSettings();
}

MainWindow::~MainWindow()
{
    delete d;
}

void MainWindow::show()
{
    if (!d->m_hideMainWin) {
        d->m_hideMainWin = false;
        KMainWindow::show();
    }
}

void MainWindow::hide()
{
    KMainWindow::hide();
}

bool MainWindow::queryExit()
{
    d->m_hideMainWin = isHidden();
    return true;
}

bool MainWindow::queryClose()
{
    if (!QObject::sender()) {
        hide();
        return false;
    }
    return true;
}

void MainWindow::notified(int reason, const KAccessibleInterface& iface)
{
    QTreeWidgetItem *root = d->m_logs->invisibleRootItem();
    if(root->childCount() > 1000) delete root->takeChild(0);

    QTreeWidgetItem *child = new QTreeWidgetItem(root);
    child->setText(1, iface.className);
    child->setText(2, iface.name);
    child->setText(3, iface.value);
    child->setText(4, iface.accelerator);
    child->setText(5, stateToString(iface.state));
    child->setText(6, QString(QLatin1String("%1,%2,%3,%4")).arg(iface.rect.x()).arg(iface.rect.y()).arg(iface.rect.width()).arg(iface.rect.height()));
    child->setText(7, iface.objectName);
    child->setText(8, iface.description);
    d->m_logs->scrollToItem(child);
}

void MainWindow::enableLogs(int state)
{
    const bool logEnabled = state == Qt::Checked;
    if(logEnabled) {
        connect(d->m_app->adaptor(), SIGNAL(notified(int,KAccessibleInterface)), this, SLOT(notified(int,KAccessibleInterface)));
    } else {
        disconnect(d->m_app->adaptor(), SIGNAL(notified(int,KAccessibleInterface)), this, SLOT(notified(int,KAccessibleInterface)));
        d->m_logs->clear();
    }

    d->m_logs->setEnabled(logEnabled);

    if(d->m_logEnabled != logEnabled) {
        d->m_logEnabled = logEnabled;
        KConfig config(QLatin1String( "kaccessibleapp" ));
        KConfigGroup group = config.group("Main");
        group.writeEntry("LogEnabled", d->m_logEnabled);
    }
}

void MainWindow::enableReaderChanged(int state)
{
    d->m_adaptor->setSpeechEnabled(state == Qt::Checked);
}

void MainWindow::voiceTypeChanged(int index)
{
    d->m_adaptor->setVoiceType(d->m_voiceTypeCombo->itemData(index).toInt());
}

int main(int argc, char *argv[])
{
    KAboutData aboutData("kaccessibleapp", "",
                         ki18n("KDE Accessible"), "0.4",
                         ki18n("KDE Accessible"), KAboutData::License_GPL,
                         ki18n("(c) 2010, 2011 Sebastian Sauer"));
    aboutData.addAuthor(ki18n("Sebastian Sauer"), ki18n("Maintainer"), "sebastian.sauer@kdab.com");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KUniqueApplication::addCmdLineOptions();
    if (!KUniqueApplication::start()) {
       fprintf(stderr, "kaccessibleapp is already running!\n");
       return 0;
    }

    KAccessibleApp app;

    MainWindow window(&app);
    //window.show();

    return app.exec();
}

#include "VoiceCallForm.h"
#include "Utils.h"
#include "WidgetFactory.h"
#include "Commands.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QStackedWidget>
#include <QtCore/QString>
#include <QtGui/QHeaderView>
#include <QtCore/QDebug>
#include <QtGui/QItemDelegate>
#include <QtCore/QSettings>

VoiceCallForm::VoiceCallForm(QWidget *parent)
    : CallHandlerWidget(parent)
{
    QWidget*    widget  = loadForm("qss/forms/voiceCall.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_end,         "voiceCall_end");
    assign(this, m_hold,        "voiceCall_hold");
    assign(this, m_resume,      "voiceCall_resume");
    assign(this, m_video,       "voiceCall_video");
    assign(this, m_chat,        "voiceCall_chat");
    assign(this, m_sms,         "voiceCall_sms");
    assign(this, m_file,        "voiceCall_file");
    assign(this, m_time,        "voiceCall_time");
    assign(this, m_parties,     "voiceCall_parties");
    assign(this, m_holdAll,     "voiceCall_holdAll");
    assign(this, m_confAll,     "voiceCall_confAll");
    assign(this, m_volume,      "voiceCall_volume");
    assign(this, m_minVolume,   "voiceCall_minVolume");
    assign(this, m_maxVolume,   "voiceCall_maxVolume");

    m_confAll->hide();
    m_parties->setSelectionMode(QAbstractItemView::SingleSelection);
    m_parties->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_calltype  = NoCall;

    connect(m_end,      SIGNAL(clicked()),  this,   SLOT(onEndClicked()));
    connect(m_hold,     SIGNAL(clicked()),  this,   SLOT(onHoldClicked()));
    connect(m_resume,   SIGNAL(clicked()),  this,   SLOT(onResumeClicked()));

    connect(m_video,    SIGNAL(clicked()),  this,   SLOT(onVideoClicked()));
    connect(m_chat,     SIGNAL(clicked()),  this,   SLOT(onChatClicked()));
    connect(m_sms,      SIGNAL(clicked()),  this,   SLOT(onSmsClicked()));
    connect(m_file,     SIGNAL(clicked()),  this,   SLOT(onFileClicked()));

    connect(m_confAll,  SIGNAL(clicked()),  this,   SLOT(onConfAllClicked()));
    connect(m_holdAll,  SIGNAL(clicked()),  this,   SLOT(onHoldAllClicked()));

    connect(&m_timer,   SIGNAL(timeout()),  this,   SLOT(onTimerTick()));

    m_volume->setRange(0, 1000);
    connect(m_volume,   SIGNAL(valueChanged(int)), this,   SLOT(onVolumeChanged(int)));
    connect(m_minVolume,SIGNAL(clicked()),  this,   SLOT(onMinVolumeClicked()));
    connect(m_maxVolume,SIGNAL(clicked()),  this,   SLOT(onMaxVolumeClicked()));

    vpMessageSubject()->addObserver(new VpMessageObserverFilterByBearer(
                                            this,
                                            (1 << BC_AUDIOVIDEO) | (1 << BC_VIDEO)));
    m_parties->setModel(&m_model);
    m_parties->horizontalHeader()->hide();
    m_parties->verticalHeader()->hide();
    m_parties->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_parties->horizontalHeader()->setStretchLastSection(true);
    m_parties->setContentsMargins(0, 0, 0, 0);

    connect(m_parties->selectionModel(),
                        SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
                                            this,   SLOT(onItemSelectionChanged()));

    m_ipCallIcon    = new QIcon(getFilename("qss/voiceCallForm/ipCall.png"));
    m_mblCallIcon   = new QIcon(getFilename("qss/voiceCallForm/mobileCall.png"));

    QSize   size;
    size    = m_ipCallIcon->actualSize(QSize(128, 128));
    m_parties->setIconSize(size);
}

//  VoiceCallForm::onCallAccepted
//  -------------------------------------------------------------------------
void    VoiceCallForm::onCallAccepted(
                        VPCALL           call)
{
    char    name[MAXNAMELEN + 1];
    vpstack()->GetCallRemoteName(call, name);

    if(m_call2name.find(call) == m_call2name.end())
    {
        //  it seems that the call was initiated by someone else
        AddCall(call, name);
        m_calltype  = OutgoingCall;
        Q_ASSERT(m_calltype != IncomingCall);
        m_holdAll->hide();
        m_confAll->hide();
    }

    m_resume->hide();
    m_hold->show();

    m_holdAll->show();
    m_confAll->hide();

    showMe(this);

    m_counter   = 0;

    UpdateParties();

    //  make conference
    //  ---------------------------------------------------------------------
    VPCALL  calls[MAX_NUM_OF_PARTIES];
    int     i   = 0;

    foreach(VPCALL call, m_call2name.keys())
    {
        calls[i]    = call;
        ++i;
    }
    vpstack()->ConferenceCalls(calls, m_call2name.size(), false);
}

//  VoiceCallForm::UpdateParties
//  -------------------------------------------------------------------------
void    VoiceCallForm::UpdateParties()
{
    setUpdatesEnabled(false);

    m_row2call.clear();

    m_model.clear();

    m_parties->setContentsMargins(0, 0, 0, 0);

    foreach(QString name, m_name2call.keys())
    {
        char            number[MAXNUMBERLEN + 1];

        vpstack()->GetCallRemoteNumber(m_name2call[name], number);

        ADDRESSTYPE     addressType = AddressType(number);

        int     row = m_model.rowCount();
        m_model.appendRow(new QStandardItem(addressType == ADDRT_PSTN ? *m_mblCallIcon : *m_ipCallIcon, name));
        m_parties->setRowHeight(row, m_parties->iconSize().height());

        m_row2call[row] = m_name2call[name];
    }

    if(m_call2name.empty())
    {
        hideMe(this);
    }

    updateButtons();

    setUpdatesEnabled(true);
}

//  VoiceCallForm::AddCall
//  -------------------------------------------------------------------------
void    VoiceCallForm::AddCall(
                        VPCALL          call,
                        const QString&  name)
{
    if(m_call2name.empty())
    {
        //  set default volume
#ifdef ANDROID_VERSION
    QSettings settings("/sdcard/"PROJECTNAME"/"PROJECTNAME".ini",QSettings::IniFormat);
#else
    QSettings   settings;
#endif

        unsigned    oLevel  = settings.value("audio/oLevel").toUInt();

        m_volume->setValue(oLevel);
#if SIMPLEAUDIO
#else
        VPMIXINGAUDIO_SetLevel(0, oLevel);
#endif
        m_timer.start(1000);
        m_counter   = 0;
        m_time->setText(tr("talking 00:00:00"));
    }

    m_call2name[call]   = name;
    m_name2call[name]   = call;
    m_currname          = name;

    updateButtons();
}

//  VoiceCallForm::DelCall
//  -------------------------------------------------------------------------
void    VoiceCallForm::DelCall(
                        VPCALL          call)
{
    m_name2call.remove(m_call2name[call]);
    m_call2name.remove(call);

    if(m_call2name.empty())
    {
        m_calltype  = NoCall;
        m_timer.start(1000);
    }

    updateButtons();
}

//  VoiceCallForm::updateButtons
//  -------------------------------------------------------------------------
void    VoiceCallForm::updateButtons()
{
    bool    disable =     m_call2name.isEmpty()
                          ||  (m_model.rowCount() > 1)
                          &&  m_parties->selectionModel()->selectedRows().isEmpty();

    m_end->setDisabled(     disable);
    m_hold->setDisabled(    disable);
    m_resume->setDisabled(  disable);
    m_video->setDisabled(   disable);
    m_chat->setDisabled(    disable);
    m_sms->setDisabled(     disable);
    m_file->setDisabled(    disable);
    m_time->setDisabled(    disable);

    m_parties->setDisabled(m_call2name.isEmpty());
    m_holdAll->setDisabled(m_call2name.isEmpty());
    m_confAll->setDisabled(m_call2name.isEmpty());

    m_video->setEnabled(m_call2name.size() <= MAX_NUM_OF_PARTIES);
}

//  VoiceCallForm::onCallRefused
//  -------------------------------------------------------------------------
void    VoiceCallForm::onCallRefused(
                        VPCALL          call,
                        int             reason)
{
    DelCall(call);
}

//  VoiceCallForm::call
//  -------------------------------------------------------------------------
void    VoiceCallForm::call(
                        const QString&      name)
{
    if(m_calltype != IncomingCall)
    {
        if(!m_name2call.contains(name))
        {
            m_calltype      = OutgoingCall;

            m_holdAll->show();
            m_confAll->show();

            emit calling(0, qPrintable(name), BC_VOICE);
        }
    }
}

//  VoiceCallForm::onEndClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onEndClicked()
{
    if(m_calltype != NoCall)
    {
        QItemSelectionModel*    selectionModel = m_parties->selectionModel();

        if(m_model.rowCount() == 1)
        {
            vpstack()->Disconnect(m_row2call[0], REASON_NORMAL);
        }
        else
        {
            foreach(QModelIndex index, selectionModel->selectedRows())
            {
                vpstack()->Disconnect(m_row2call[index.row()], REASON_NORMAL);
            }
        }
    }
}

//  VoiceCallForm::onPartyHoldClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onPartyHoldClicked()
{
    QString         name    = sender()->property("callname").toString();
    QStackedWidget* stack   = static_cast<QStackedWidget*>(sender()->parent());

    vpstack()->Hold(m_name2call[name]);

    stack->setCurrentIndex(1);
}

//  VoiceCallForm::onPartyResumeClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onPartyResumeClicked()
{
    QString         name    = sender()->property("callname").toString();
    QStackedWidget* stack   = static_cast<QStackedWidget*>(sender()->parent());

    vpstack()->Resume(m_name2call[name]);

    stack->setCurrentIndex(0);
}

//  VoiceCallForm::onConfAllClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onConfAllClicked()
{
    foreach(QString name, m_name2call.keys())
    {
        vpstack()->Resume(m_name2call[name]);
    }

    m_holdAll->show();
    m_confAll->hide();
}

//  VoiceCallForm::onHoldAllClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onHoldAllClicked()
{
    foreach(QString name, m_name2call.keys())
    {
        vpstack()->Hold(m_name2call[name]);
    }

    m_holdAll->hide();
    m_confAll->show();
}


//  VoiceCallForm::onHoldClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onHoldClicked()
{
    if(m_calltype !=  NoCall)
    {
        QItemSelectionModel*    selectionModel = m_parties->selectionModel();

        if(m_model.rowCount() == 1)
        {
            vpstack()->Hold(m_row2call[0]);
        }
        else
        {
			foreach(QModelIndex index, selectionModel->selectedRows())
			{
				vpstack()->Hold(m_row2call[index.row()]);
			}
		}

        m_hold->hide();
        m_resume->show();
    }
}

//  VoiceCallForm::onResumeClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onResumeClicked()
{
    if(m_calltype !=  NoCall)
    {
        QItemSelectionModel*    selectionModel = m_parties->selectionModel();

        if(m_model.rowCount() == 1)
        {
            vpstack()->Resume(m_row2call[0]);
        }
        else
        {
			foreach(QModelIndex index, selectionModel->selectedRows())
			{
				vpstack()->Resume(m_row2call[index.row()]);
			}
		}
		
        m_resume->hide();
        m_hold->show();
    }
}

//  VoiceCallForm::onItemSelectionChanged
//  -------------------------------------------------------------------------
void    VoiceCallForm::onItemSelectionChanged()
{
    QItemSelectionModel*    selectionModel = m_parties->selectionModel();

    foreach(QModelIndex index, selectionModel->selectedRows())
    {
        if(vpstack()->IsHeld(m_row2call[index.row()]))
        {
            m_resume->show();
            m_hold->hide();
        }
        else
        {
            m_resume->hide();
            m_hold->show();
        }
    }
}

//  VoiceCallForm::onTimerTick
//  -------------------------------------------------------------------------
void    VoiceCallForm::onTimerTick()
{
    ++m_counter;

    int     h   = m_counter / (60 * 60);
    int     m   = (m_counter / 60) % 60;
    int     s   = m_counter % 60;

    m_time->setText(QString(tr("talking %1:%2:%3"))
                        .arg(h, 2, 10, (QChar)'0')
                        .arg(m, 2, 10, (QChar)'0')
                        .arg(s, 2, 10, (QChar)'0'));
}

//  VoiceCallForm::onSendFileReq
//  -------------------------------------------------------------------------
void    VoiceCallForm::onVpStackMessage(
                                unsigned        msg,
                                VPCALL          call,
                                unsigned        param)
{
    switch(msg)
    {
        case    VPMSG_NEWCALL       :
        {
            char    name[MAXNAMELEN + 1];

            m_calltype  = IncomingCall;

            vpstack()->AnswerCall(call);
            vpstack()->GetCallRemoteName(call, name);
            AddCall(call, name);
            m_currname  = name;

            m_holdAll->hide();
            m_confAll->hide();

            UpdateParties();

            showMe(this);
        } break;

        //  VPMSG_CALLACCEPTED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLACCEPTED  :
        {
            onCallAccepted(call);
        } break;

        //  VPMSG_CALLREFUSED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLREFUSED   :
        {
            onCallRefused(call, param);
        } break;

        //  -----------------------------------------------------------------
        case    VPMSG_CALLENDED     :
        {
            DelCall(call);
            UpdateParties();
        } break;

        case    VPMSG_REMOTELYHELD              :
        {
            m_hold->hide();
            m_resume->show();
        } break;

        case    VPMSG_REMOTELYRESUMED           :
        {
            m_hold->show();
            m_resume->hide();
        } break;
    }
}

//  VoiceCallForm::getUsername
//  -------------------------------------------------------------------------
QString VoiceCallForm::getUsername()
{
    QItemSelectionModel*    selectionModel = m_parties->selectionModel();

    if(m_model.rowCount() == 1)
    {
        return	m_call2name[m_row2call[0]];;
    }
    else
    {
		foreach(QModelIndex index, selectionModel->selectedRows())
		{
			return  m_call2name[m_row2call[index.row()]];
		}
	}
    return  "";
}

//  VoiceCallForm::onVideoClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onVideoClicked()
{
    foreach(QString name, m_call2name.values())
    {
        emit    calling(0, name, BC_AUDIOVIDEO);
    }
}

//  VoiceCallForm::onSmsClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onSmsClicked()
{
    emit    makeCall(getUsername(), BC_SMS);
}

//  VoiceCallForm::onChatClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onChatClicked()
{
    emit    makeCall(getUsername(), BC_CHAT);
}

//  VoiceCallForm::onFileClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onFileClicked()
{
    emit    makeCall(getUsername(), BC_FILE);
}

//  VoiceCallForm::message
//  -------------------------------------------------------------------------
void    VoiceCallForm::message(
                    VpMessageSubject*   subject,
                    unsigned            msg,
                    unsigned            pcall,
                    unsigned            param)
{
    if(msg == VPMSG_CALLACCEPTED)
    {
        char    username[MAXNAMELEN + 1];

        vpstack()->GetCallRemoteName((VPCALL)pcall, username);

        QItemSelectionModel*    selectionModel = m_parties->selectionModel();

        if(m_model.rowCount() == 1)
        {
            vpstack()->Disconnect(m_row2call[0], REASON_NORMAL);
        }
        else
        {
			foreach(QModelIndex index, selectionModel->selectedRows())
			{
				if(username == m_call2name[m_row2call[index.row()]])
				{
					vpstack()->Disconnect(m_row2call[index.row()], REASON_VOICE2VIDEO);
					break;
				}
			}
		}
    }
}

//  VoiceCallForm::onVolumeChanged
//  -------------------------------------------------------------------------
void    VoiceCallForm::onVolumeChanged(int  volume)
{
#if SIMPLEAUDIO
#else
    VPMIXINGAUDIO_SetLevel(0, m_volume->value());
#endif
}

//  VoiceCallForm::onMinVolumeClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onMinVolumeClicked()
{
    m_volume->setValue(m_volume->value() - 0.1 * (m_volume->maximum() - m_volume->minimum()));
}

//  VoiceCallForm::onMaxVolumeClicked
//  -------------------------------------------------------------------------
void    VoiceCallForm::onMaxVolumeClicked()
{
    m_volume->setValue(m_volume->value() + 0.1 * (m_volume->maximum() - m_volume->minimum()));
}

#include <time.h>

#include "CallHistoryForm.h"
#include "Utils.h"
#include "WidgetFactory.h"
#include "LogEntry.h"
#include "Utils.h"

#include <QtCore/QFile>
#include <QtCore/QVariant>
#include <QtCore/QVector>

#include <QtGui/QLabel>
#include <QtGui/QTableWidgetItem>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QItemDelegate>
#include <QtGui/QApplication>
#include <QtGui/QAbstractScrollArea>
#include <QtGui/QAbstractItemView>
#include <QtGui/QHeaderView>
#include <QtCore/QDebug>
#include <QtGui/QPainter>
#include "mainwindow.h"

#include <QtGui/QIcon>

extern
MainWindow*     g_mainWindow;

class   DurationItemDelegate
    : public QItemDelegate
{
    public:
        void    paint(  QPainter*                       painter,
                        const QStyleOptionViewItem&     option,
                        const QModelIndex&              index )const
        {
            int     secs    = index.model()->data(index).toInt();
            QString text    = QString("%1:%2:%3")
                              .arg(secs / 60 / 60,  2, 10, QChar('0'))
                              .arg(secs / 60 % 60,  2, 10, QChar('0'))
                              .arg(secs % 60,       2, 10, QChar('0'));
            QStyleOptionViewItem    opt = option;
            opt.displayAlignment    = Qt::AlignCenter | Qt::AlignVCenter;

            drawDisplay(painter, opt, opt.rect, text);
        }
};

class   FlagItemDelegate
    : public QItemDelegate
{
    public:
        ~FlagItemDelegate()
        {
            foreach(QImage* image, m_image)
            {
                delete image;
            }
        }
        FlagItemDelegate(unsigned int           mask,
                         const QStringList&     files)
        {
            m_mask  = mask;

            foreach(QString file, files)
            {
                QImage* image   = new QImage(file);
                m_image.append(image);
            }
        }

        QSize sizeHint( const QStyleOptionViewItem&     option,
                        const QModelIndex&              index) const
        {
            return  m_image[0]->size();
        }

        void    paint(  QPainter*                       painter,
                        const QStyleOptionViewItem&     option,
                        const QModelIndex&              index )const
        {
            QString         str     = index.model()->data(index, Qt::DisplayRole).toString();
            unsigned int    data    = index.model()->data(index, Qt::DisplayRole).toUInt();
            unsigned int    image   = (data & m_mask) / (((m_mask - 1) & ~m_mask) + 1);

            drawBackground(painter, option, index);

            painter->drawImage( option.rect,
                                *m_image[image % m_image.count()],
                                QRect(QPoint(0, 0), option.rect.size()));
        }

    private:
        QVector<QImage*>    m_image;
        unsigned int        m_mask;
};


CallHistoryForm::CallHistoryForm(QWidget *parent)
    : QWidget(parent)
    , m_click(getFilename("sounds/click.wav"))
{
    QWidget*    widget  = loadForm("qss/forms/log.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_client,      "log_client");
    assign(this, m_all,         "log_all");
    assign(this, m_voice,       "log_voice");
    assign(this, m_video,       "log_video");
    assign(this, m_videom,      "log_videom");
    assign(this, m_sms,         "log_sms");
    assign(this, m_chat,        "log_chat");
    assign(this, m_file,        "log_file");

    assign(this, m_incoming,    "log_incoming");
    assign(this, m_outgoint,    "log_outgoing");
    assign(this, m_answered,    "log_answered");

    assign(this, m_chooseDate,  "log_date");
    assign(this, m_clear,       "log_clear");
    assign(this, m_call,        "log_call");

    assign(this, m_calendar,    "log_calendar");  m_calendar->hide();

    connect(m_chooseDate,   SIGNAL(clicked(bool)),      this,   SLOT(chooseDate()));
    connect(m_clear,        SIGNAL(clicked()),          this,   SLOT(onClearClicked()));
    connect(m_calendar,     SIGNAL(selectionChanged()), this,   SLOT(onDateSelectionChanged()));
    connect(m_call,         SIGNAL(clicked()),          this,   SLOT(onCallClicked()));

    connect(m_all,          SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_voice,        SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_video,        SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_videom,       SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_sms,          SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_chat,         SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_file,         SIGNAL(clicked()),  this, SLOT(updateView()));

    connect(m_incoming,     SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_outgoint,     SIGNAL(clicked()),  this, SLOT(updateView()));
    connect(m_answered,     SIGNAL(clicked()),  this, SLOT(updateView()));

    connect(m_client, SIGNAL(clicked(QModelIndex)), this, SLOT(onTableClicked(QModelIndex)));
}

//  CallHistoryForm::chooseDate
//  -------------------------------------------------------------------------
void    CallHistoryForm::chooseDate()
{
    if(m_calendar->isHidden())
    {
        m_calendar->show();
        m_calendar->raise();
    }
    else
    {
        m_calendar->hide();
    }
}

//  CallHistoryForm::showEvent
//  -------------------------------------------------------------------------
void    CallHistoryForm::showEvent(QShowEvent *)
{
    QSqlQuery   query;

	m_date  = QDate::currentDate().toString("yyyy-MM-dd");
        query.exec(QString("UPDATE log SET type = (type | %1) WHERE (type & %2) = %3 OR (type & %4) = %5")
				.arg(logtRead)
				.arg(logBcMask)
				.arg(logtVoice)
				.arg(logBcMask)
				.arg(logtAudioVideo));
	LOG_SQL_QUERY(query);
    updateView();
}

//  CallHistoryForm::filter
//  -------------------------------------------------------------------------
QString CallHistoryForm::filter()
{
    QString     result;

    if(m_voice->isChecked())
    {
        result.append(QString("AND (type & %1) = %2 ")
                        .arg(logBcMask)
                        .arg(logtVoice));
    }

    if(m_video->isChecked())
    {
        result.append(QString("AND (((type & %1) = %2) OR ((type & %1) = %3)) ")
                        .arg(logBcMask)
                        .arg(logtAudioVideo)
                        .arg(logtVideo));
    }

    if(m_videom->isChecked())
    {
        result.append(QString("AND ((type & %1) = %2) ")
                        .arg(logBcMask)
                        .arg(logtVideoMsg));
    }

    if(m_sms->isChecked())
    {
        result.append(QString("AND ((type & %1) = %2) ")
                        .arg(logBcMask)
                        .arg(logtSms));
    }

    if(m_chat->isChecked())
    {
        result.append(QString("AND ((type & %1) = %2) ")
                        .arg(logBcMask)
                        .arg(logtChat));
    }

    if(m_file->isChecked())
    {
        result.append(QString("AND ((type & %1) = %2) ")
                        .arg(logBcMask)
                        .arg(logtFile));
    }

    result.append("AND (0 ");

    if(m_answered->isChecked())
    {
        result.append(QString("OR (type & %1) ")
                        .arg(logtAnswered));
    }

    if(m_outgoint->isChecked())
    {
        result.append(QString("OR ((type & %1) = %1) ")
                        .arg(logtOutgoing));
    }

    if(m_incoming->isChecked())
    {
        result.append(QString("OR ((type & %1) = 0) ")
                        .arg(logtOutgoing));
    }

    result.append(")");

    return  result;
}

//  CallHistoryForm::updateView
//  -------------------------------------------------------------------------
void    CallHistoryForm::updateView()
{
    setUpdatesEnabled(false);

    if(m_date == "" || m_date == QDate::currentDate().toString("yyyy-MM-dd"))
    {
        model.setQuery(QString("SELECT log.id, log.type, log.type, log.started_on, strftime('%s', log.ended_on) - strftime('%s', log.started_on), log.username, log.vpnumber, type FROM log JOIN user ON log.owner = user.id WHERE user.username = '%1' %2 ORDER BY log.started_on DESC")
                            .arg(getUsername())
                            .arg(filter()));
    }
    else
    {
        model.setQuery(QString("SELECT log.id, log.type, log.type, log.started_on, strftime('%s', log.ended_on) - strftime('%s', log.started_on), log.username, log.vpnumber, type FROM log JOIN user ON log.owner = user.id WHERE user.username = '%1' %2 AND date(started_on) == '%3' ORDER BY log.started_on DESC")
                            .arg(getUsername())
                            .arg(filter())
                            .arg(m_date));
    }

//    model.setHeaderData(1, Qt::Horizontal, QString(tr("Flag")));
    model.setHeaderData(1, Qt::Horizontal, QString(tr("New")));
    model.setHeaderData(2, Qt::Horizontal, QString(tr("Type")));
    model.setHeaderData(3, Qt::Horizontal, QString(tr("Time")));
    model.setHeaderData(4, Qt::Horizontal, QString(tr("Duration")));
    model.setHeaderData(5, Qt::Horizontal, QString(tr("Name")));
    model.setHeaderData(6, Qt::Horizontal, QString(tr("Number")));
    model.setHeaderData(7, Qt::Horizontal, QString(tr("Status")));

    QStringList     flag;
    flag.append(getFilename("qss/logForm/unflagged.png"));
    flag.append(getFilename("qss/logForm/flagged.png"));

    QStringList     oldnew;
    oldnew.append(getFilename("qss/logForm/new.png"));
    oldnew.append(getFilename("qss/logForm/old.png"));

    QStringList     bctype;
    bctype.append(getFilename("qss/logForm/voice.png"));
    bctype.append(getFilename("qss/logForm/video.png"));
    bctype.append(getFilename("qss/logForm/chat.png"));
    bctype.append(getFilename("qss/logForm/sms.png"));
    bctype.append(getFilename("qss/logForm/file.png"));
    bctype.append(getFilename("qss/logForm/video.png"));
    bctype.append(getFilename("qss/logForm/videom.png"));

    QStringList     answered;
    answered.append(getFilename("qss/logForm/unanswered.png"));
    answered.append(getFilename("qss/logForm/answered.png"));

    m_client->setModel(&model);
    m_client->setColumnHidden(0, true);
//    m_client->setItemDelegateForColumn(1, new FlagItemDelegate(logtFlag,        flag));
    m_client->setItemDelegateForColumn(1, new FlagItemDelegate(logtRead,        oldnew));
    m_client->setItemDelegateForColumn(2, new FlagItemDelegate(logBcMask,       bctype));

    m_client->setItemDelegateForColumn(4, new DurationItemDelegate);

    m_client->setItemDelegateForColumn(7, new FlagItemDelegate(logtAnswered,    answered));

    m_client->setShowGrid(false);
    m_client->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_client->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_client->verticalHeader()->hide();
    m_client->setSelectionMode(QAbstractItemView::NoSelection);

    m_client->resizeRowsToContents();
    m_client->resizeColumnsToContents();

    m_client->horizontalHeader()->setHighlightSections(false);
    m_client->horizontalHeader()->setResizeMode(5, QHeaderView::Stretch);

    m_client->setSelectionMode(QAbstractItemView::SingleSelection);
    m_client->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_call->setDisabled(m_client->selectionModel()->selectedRows().isEmpty());

    setUpdatesEnabled(true);
}

//  CallHistoryForm::onTableClicked
//  -------------------------------------------------------------------------
void    CallHistoryForm::onTableClicked(
                        const QModelIndex&    index)
{
    QSqlQueryModel*     model   = static_cast<QSqlQueryModel*>(m_client->model());
    QSqlRecord          record  = model->record(index.row());
    int                 id      = record.value("id").toInt();
    unsigned int        type    = record.value("type").toUInt();
    QSqlQuery           query;
    QModelIndexList     list    = m_client->selectionModel()->selectedRows();
    bool                selected= !list.empty()
                                && list.first().row() == index.row();

    m_click.play();

    switch(index.column())
    {
/*
        case    0:
        {
            query.exec(QString("UPDATE log SET type = ((type & ~%1) | (~type & %1)) WHERE id = '%2'")
                            .arg(logtFlag)
                            .arg(id));
            LOG_SQL_QUERY(query);

            updateView();
        } break;
*/
        case    1:
        {
            query.exec(QString("UPDATE log SET type = (type | %1) WHERE id = '%2'")
                        .arg(logtRead)
                        .arg(id));
            LOG_SQL_QUERY(query);

            updateView();
        } break;

        default:
        {
            if(selected && (type & logBcMask) == logtSms)
            {
                emit showSms(id);
            }

            if(selected && (type & logBcMask) == logtVideoMsg)
            {
                emit showMms(id);
            }
        }
    }

    m_call->setDisabled(m_client->selectionModel()->selectedRows().isEmpty());
}

//  CallHistoryForm::onClearClicked
//  -------------------------------------------------------------------------
void    CallHistoryForm::onClearClicked()
{
    emit clearing();
}

//  CallHistoryForm::onDateSelectionChanged
//  -------------------------------------------------------------------------
void    CallHistoryForm::onDateSelectionChanged()
{
    m_date  = m_calendar->selectedDate().toString("yyyy-MM-dd");
    m_calendar->hide();
    updateView();
}

//  CallHistoryForm::onCallClicked
//  -------------------------------------------------------------------------
void    CallHistoryForm::onCallClicked()
{
    QModelIndexList list    = m_client->selectionModel()->selectedRows();

    if(!list.isEmpty())
    {
        QModelIndex index   = list.first();
        QSqlRecord  record  = model.record(index.row());

        emit call(record.value("username").toString(), BC_VOICE);
    }
}

//  CallHistoryForm::doClear
//  -------------------------------------------------------------------------
void    CallHistoryForm::doClear()
{
    QSqlQuery   query;
    query.exec(QString("DELETE FROM log WHERE owner = (SELECT id FROM user WHERE username = '%1') %2")
                        .arg(getUsername())
                        .arg(filter()));


    updateView();
}


//  CallHistoryForm::onShowSms
//  -------------------------------------------------------------------------
void    CallHistoryForm::onShowSms()
{
    int id  = sender()->property("id").toInt();

    emit    showSms(id);
}

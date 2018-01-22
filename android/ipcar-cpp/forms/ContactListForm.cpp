#include "ContactListForm.h"

#include "WidgetFactory.h"
#include "Commands.h"
#include "Utils.h"
#include "AddressBook.h"

#include <QtGui/QVBoxLayout>
#include <QtGui/QGridLayout>
#include <QtGui/QScrollBar>
#include <QtGui/QPushButton>
#include <QtGui/QStackedWidget>
#include <QtCore/QFile>

#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QtSql/QSqlError>
#include <QtSql/QSqlField>
#include <QtGui/QAbstractItemView>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QItemDelegate>
#include <QtGui/QPainter>
#include <QtGui/QHeaderView>
#include <QtCore/QTextStream>

#include <QtCore/QDebug>

#define CONTACT_COLUMN_INDEX_STATUS     0
#define CONTACT_COLUMN_INDEX_USERNAME   1
#define CONTACT_COLUMN_INDEX_IPCALL     2
#define CONTACT_COLUMN_INDEX_MOBILECALL 3
#define CONTACT_COLUMN_INDEX_VIDEO      4
#define CONTACT_COLUMN_INDEX_VIDEOMSG   5
#define CONTACT_COLUMN_INDEX_SMS        6
#define CONTACT_COLUMN_INDEX_FILE       7
#define CONTACT_COLUMN_INDEX_CHAT       8
#define CONTACT_COLUMN_INDEX_VPNUMBER   9
#define CONTACT_COLUMN_INDEX_FIRSTNAME 10
#define CONTACT_COLUMN_INDEX_LASTNAME  11

QMap<QString, int>  s_vpnumber2status;
QMap<int, QString>  s_id2vpnumber;
QSemaphore          s_semaphore(1);

typedef int (*getUserStatus_t)(int id);

int AAA = 0;

int     getUserStatus(int id)
{
    int result;

    s_semaphore.acquire();
    result  = s_vpnumber2status[s_id2vpnumber[id]];
    s_semaphore.release();

    return  result;
}

class   ContactsItemDelegate
    : public QItemDelegate
{
    public:
        ~ContactsItemDelegate()
        {
            delete m_image;
        }

        ContactsItemDelegate()
        {
            m_image = new QImage(getFilename("qss/contactsForm/separator.png"));
        }

        void    paint(  QPainter*                       painter,
                        const QStyleOptionViewItem&     option,
                        const QModelIndex&              index )const
        {
            QRect   rect    = option.rect;

            //drawBackground(painter, option, index);

            painter->drawImage( QRect(QPoint(   rect.left(),
                                                rect.bottom() - m_image->height()),
                                      QSize(    rect.width(),
                                                m_image->height())),
                                *m_image,
                                QRect(QPoint(   rect.left(),
                                                0),
                                      QSize(    rect.width(),
                                                m_image->height())));
        }
    protected:
        int     getHeight() const
        {
            return  m_image->height();
        }

    private:
        QImage* m_image;
};

class   AolItemDelegate
    : public ContactsItemDelegate
{
    public:
        ~AolItemDelegate()
        {
            delete m_online;
            delete m_offline;
            delete m_unknown;
            delete m_limited;
        }
        AolItemDelegate(    const QString&              online,
                            const QString&              offline,
                            const QString&              unknown,
                            const QString&              limited,
                            getUserStatus_t             getUserStatus)
            : m_getUserStatus(getUserStatus)
        {
            m_online    = new QImage(online);
            m_offline   = new QImage(offline);
            m_unknown   = new QImage(unknown);
            m_limited   = new QImage(limited);
        }

        QSize sizeHint( const QStyleOptionViewItem&     option,
                        const QModelIndex&              index) const
        {
            return  m_online->size() + QSize(0, getHeight());
        }

        void    paint(  QPainter*                       painter,
                        const QStyleOptionViewItem&     option,
                        const QModelIndex&              index )const
        {
            const
            QSqlQueryModel* model   = static_cast<const QSqlQueryModel*>(index.model());
            QSqlRecord      record  = model->record(index.row());
            int             id      = record.value("id").toInt();
            QImage*         image   = m_unknown;

            switch(getUserStatus(id))
            {
                case    AOL_ONLINE:
                {
                    image   = m_online;
                } break;

                case    AOL_LIMITED:
                {
                    image   = m_limited;
                } break;

                case    AOL_OFFLINE:
                {
                    image   = m_offline;
                } break;
            }

            ContactsItemDelegate::paint(painter, option, index);

            painter->drawImage( option.rect,
                                *image,
                                QRect(QPoint(0, 0), option.rect.size()));

        }

    private:
        QImage*                 m_online;
        QImage*                 m_offline;
        QImage*                 m_limited;
        QImage*                 m_unknown;
        getUserStatus_t         m_getUserStatus;
};

class   TextItemDelegate
    : public ContactsItemDelegate
{
    public:
        ~TextItemDelegate()
        {
        }

        TextItemDelegate()
        {
        }

        QSize sizeHint( const QStyleOptionViewItem&     option,
                        const QModelIndex&              index) const
        {
            QString         text    = index.model()->data(index, Qt::DisplayRole).toString();

            return  QSize(option.fontMetrics.width(text) * 1.2, getHeight());
        }

        void    paint(  QPainter*                       painter,
                        const QStyleOptionViewItem&     option,
                        const QModelIndex&              index )const
        {
            QString         text    = index.model()->data(index, Qt::DisplayRole).toString();
            QRect           rect    = option.rect;

            ContactsItemDelegate::paint(painter, option, index);

            painter->drawText(rect, Qt::AlignLeft | Qt::AlignVCenter, text);
        }
};

class   ContactItemDelegate
    : public ContactsItemDelegate
{
    public:
        ~ContactItemDelegate()
        {
            delete m_normal;
            delete m_hover;
            delete m_disabled;
        }
        ContactItemDelegate(const QString&      normal,
                            const QString&      hover,
                            const QString&      disabled,
                            getUserStatus_t     getUserStatus,
                            bool                mobile = false)
        {
            m_normal        = new QImage(normal);
            m_hover         = new QImage(hover);
            m_disabled      = new QImage(disabled);
            m_getUserStatus = getUserStatus;
            m_mobile        = mobile;
        }

        QSize sizeHint( const QStyleOptionViewItem&     option,
                        const QModelIndex&              index) const
        {
            return  m_normal->size() + QSize(7, getHeight());;
        }

        void    paint(  QPainter*                       painter,
                        const QStyleOptionViewItem&     option,
                        const QModelIndex&              index )const
        {
            QImage*         image;
            const
            QSqlQueryModel* model   = static_cast<const QSqlQueryModel*>(index.model());
            QSqlRecord      record  = model->record(index.row());
            int             id      = record.value("id").toInt();
            QString         text    = index.model()->data(index, Qt::DisplayRole).toString();

            if(m_getUserStatus(id) != AOL_ONLINE && !m_mobile || text == "")
            {
                image   = m_disabled;
            }
            else if(option.state & QStyle::State_MouseOver)
            {
                image   = m_hover;
            }
            else
            {
                image   = m_normal;
            }

            ContactsItemDelegate::paint(painter, option, index);

            painter->drawImage( QPoint(option.rect.left() + 7, option.rect.top()),
                                *image,
                                QRect(QPoint(0, 0), option.rect.size()));
        }

    private:
        QImage*             m_normal;
        QImage*             m_hover;
        QImage*             m_disabled;
        getUserStatus_t     m_getUserStatus;
        bool                m_mobile;
};

ContactListForm::ContactListForm(QWidget *parent)
    : QWidget(parent)
    , m_click(getFilename("sounds/click.wav"))
    , m_full(true)
{
    QWidget*    widget  = loadForm("qss/forms/contacts.ui");

    QVBoxLayout*layout  = new QVBoxLayout;

    layout->addWidget(widget);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    this->setLayout(layout);

    assign(this, m_table, "contacts_table");

    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(&m_timer,   SIGNAL(timeout()), this, SLOT(onTimeout()));
    connect(m_table,    SIGNAL(clicked(QModelIndex)), this, SLOT(onTableClicked(QModelIndex)));

    m_lastClickedRow    = -1;
}

#define MAX_AOLs    100
typedef struct tag_AOL_t
{
    AOL             aol;
    AOLNUM          nums[MAX_AOLs];
    int             ids[MAX_AOLs];
} AOL_t;

//  ContactListForm::selectContact
//  -------------------------------------------------------------------------
void    ContactListForm::selectContact()
{
    m_full  = false;
}

//  ContactListForm::updateView
//  -------------------------------------------------------------------------
void    ContactListForm::updateView()
{
    //  update id
    QSqlQuery   query;

    query.exec(QString("SELECT contact.id, contact.vpnumber, contact.username FROM contact JOIN user ON contact.owner = user.id WHERE user.username = '%1'")
                    .arg(getUsername()));
    LOG_SQL_QUERY(query);
    
    s_id2vpnumber.clear();

    while(query.next())
    {
        QSqlRecord      record  = query.record();

        int             id      = record.value("id").toUInt();
        QString         vpnumber= record.value("vpnumber").toString();
        QString         username= record.value("username").toString();

        if(vpnumber.isEmpty())
        {
            VPCALL  call;
            vpstack()->Connect(&call, qPrintable(username), BC_NOTHING);
        }

        else if(username.isEmpty() || username ==  vpnumber)
        {
            VPCALL  call;
            vpstack()->Connect(&call, qPrintable(vpnumber), BC_NOTHING);
        }
        else
        {
            s_id2vpnumber[id]   = vpnumber;
        }
    }

    setUpdatesEnabled(false);

    //  get data
    //  ---------------------------------------------------------------------
    m_model.setQuery(QString("SELECT "
                                "contact.id, "
                                "contact.username, "
                                "contact.id, "
                                "contact.mobile, "
                                "contact.id, "
                                "contact.id, "
                                "contact.id, "
                                "contact.id, "
                                "contact.id, "
                                "contact.vpnumber, "
                                "contact.firstName, "
                                "contact.lastName "
                             "FROM contact JOIN user ON contact.owner = user.id WHERE user.username = '%1'")
                    .arg(getUsername()));

    m_table->setModel(&m_model);

    //  CONTACT_COLUMN_INDEX_STATUS
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_STATUS,
                    new AolItemDelegate(
                            getFilename("qss/contactsForm/online.png"),
                            getFilename("qss/contactsForm/offline.png"),
                            getFilename("qss/contactsForm/nostatus.png"),
                            getFilename("qss/contactsForm/limited.png"),
                            getUserStatus));

    //  CONTACT_COLUMN_INDEX_USERNAME
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_USERNAME,
                    new TextItemDelegate());
    if(m_full)
        m_table->horizontalHeader()->setResizeMode(CONTACT_COLUMN_INDEX_USERNAME, QHeaderView::Stretch);

    //  CONTACT_COLUMN_INDEX_IPCALL
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_IPCALL,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/call.png"),
                            getFilename("qss/contactsForm/call-hover.png"),
                            getFilename("qss/contactsForm/call-disabled.png"),
                            getUserStatus));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_IPCALL);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_IPCALL);

    //  CONTACT_COLUMN_INDEX_MOBILECALL
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_MOBILECALL,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/mobile.png"),
                            getFilename("qss/contactsForm/mobile-hover.png"),
                            getFilename("qss/contactsForm/mobile-disabled.png"),
                            getUserStatus,
                            true));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_MOBILECALL);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_MOBILECALL);

    //  CONTACT_COLUMN_INDEX_VIDEO
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_VIDEO,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/video.png"),
                            getFilename("qss/contactsForm/video-hover.png"),
                            getFilename("qss/contactsForm/video-disabled.png"),
                            getUserStatus));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_VIDEO);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_VIDEO);

    //  CONTACT_COLUMN_INDEX_VIDEOMSG
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_VIDEOMSG,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/videom.png"),
                            getFilename("qss/contactsForm/videom-hover.png"),
                            getFilename("qss/contactsForm/videom-disabled.png"),
                            getUserStatus));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_VIDEOMSG);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_VIDEOMSG);

    //  CONTACT_COLUMN_INDEX_SMS
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_SMS,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/sms.png"),
                            getFilename("qss/contactsForm/sms-hover.png"),
                            getFilename("qss/contactsForm/sms.png"),
                            getUserStatus));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_SMS);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_SMS);

    //  CONTACT_COLUMN_INDEX_FILE
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_FILE,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/file.png"),
                            getFilename("qss/contactsForm/file-hover.png"),
                            getFilename("qss/contactsForm/file-disabled.png"),
                            getUserStatus));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_FILE);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_FILE);

    //  CONTACT_COLUMN_INDEX_CHAT
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_CHAT,
                    new ContactItemDelegate(
                            getFilename("qss/contactsForm/chat.png"),
                            getFilename("qss/contactsForm/chat-hover.png"),
                            getFilename("qss/contactsForm/chat-disabled.png"),
                            getUserStatus));
    if(!m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_CHAT);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_CHAT);

    //  CONTACT_COLUMN_INDEX_VPNUMBER
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_VPNUMBER,
                    new TextItemDelegate());
    if(m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_VPNUMBER); // vpnumber
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_VPNUMBER);

    //  CONTACT_COLUMN_INDEX_FIRSTNAME
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_FIRSTNAME,
                    new TextItemDelegate());
    if(m_full)
        m_table->hideColumn(CONTACT_COLUMN_INDEX_FIRSTNAME);
    else
        m_table->showColumn(CONTACT_COLUMN_INDEX_FIRSTNAME);

    //  CONTACT_COLUMN_INDEX_LASTNAME
    //  -------------------------------------------------------------
    m_table->setItemDelegateForColumn(
                    CONTACT_COLUMN_INDEX_LASTNAME,
                    new TextItemDelegate());
    if(m_full)
    {
        m_table->hideColumn(CONTACT_COLUMN_INDEX_LASTNAME);
    }
    else
    {
        m_table->showColumn(CONTACT_COLUMN_INDEX_LASTNAME);
        m_table->horizontalHeader()->setResizeMode(CONTACT_COLUMN_INDEX_LASTNAME, QHeaderView::Stretch);
    }

    m_table->setShowGrid(false);
    m_table->verticalHeader()->hide();
    m_table->horizontalHeader()->hide();
    m_table->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_table->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
    m_table->horizontalHeader()->setHighlightSections(false);
    m_table->setContentsMargins(0, 0, 0, 0);
    m_table->resizeRowsToContents();
    m_table->resizeColumnsToContents();
    m_table->viewport()->setAttribute(Qt::WA_Hover);
    m_table->viewport()->setMouseTracking(true);

    m_timer.setSingleShot(true);

    setUpdatesEnabled(true);

    if(m_lastClickedRow != -1)
    {
        m_table->selectRow(m_lastClickedRow);
    }
}

//  ContactListForm::onLogon
//  -------------------------------------------------------------------------
void    ContactListForm::onLogon()
{
    m_timer.start(1);
}

//  ContactListForm::onSelectContact
//  -------------------------------------------------------------------------
void    ContactListForm::onSelectContact()
{
    m_full  = false;
    showMe(this);
}

//  ContactListForm::onAbUpdate
//  -------------------------------------------------------------------------
void    ContactListForm::onAbUpdate(
                    const QString&      username,
                    const QString&      vpnumber)
{
    QSqlQuery   query;
    int         id;

    query.exec(QString("SELECT contact.id FROM contact JOIN user ON contact.owner = user.id WHERE user.username = '%1' AND contact.username = '%2'")
                    .arg(getUsername())
                    .arg(username));
    LOG_SQL_QUERY(query);


    if(query.first())
    {
        QSqlRecord      record  = query.record();
        int             id      = record.value("id").toUInt();

        if(s_id2vpnumber[id] != vpnumber)
        {
            s_id2vpnumber[id] = vpnumber;
            m_timer.start(1);
        }
    }
}

//  ContactListForm::hideEvent
//  -------------------------------------------------------------------------
void    ContactListForm::hideEvent(QHideEvent *event)
{
    m_timer.stop();
    if(m_full == false)
    {
        m_full  = true;
        updateView();
    }
}

//  ContactListForm::showEvent
//  -------------------------------------------------------------------------
void    ContactListForm::showEvent( QShowEvent * event )
{
    updateView();

    m_timer.start(1);
}

//  ContactListForm::onTimeout
//  -------------------------------------------------------------------------
void    ContactListForm::onTimeout()
{
    s_semaphore.acquire();
    AOL_t*  aol = (AOL_t*)malloc(sizeof(AOL_t));

    aol->aol.notificationreqop  = NOTIFICATIONREQ_ENABLE;
    aol->aol.N                  = 0;
    aol->aol.nums               = aol->nums;

    foreach(int id, s_id2vpnumber.keys())
    {
        QString         vpnumber= s_id2vpnumber[id];

        if(vpnumber.isEmpty())
        {
            continue;
        }

        int             srvid   = vpnumber.left(3).toInt();
        int             num     = vpnumber.right(vpnumber.length() - 3).toInt();

        aol->nums[aol->aol.N].srvid = srvid;
        aol->nums[aol->aol.N].num   = num;
        aol->nums[aol->aol.N].online= 0;
        aol->ids[aol->aol.N]        = id;

        ++aol->aol.N;

        if(aol->aol.N == MAX_AOLs)
        {
            vpstack()->AskOnline(&aol->aol);

            aol = (AOL_t*)malloc(sizeof(AOL_t));

            aol->aol.notificationreqop  = NOTIFICATIONREQ_ENABLE;
            aol->aol.N                  = 0;
            aol->aol.nums               = aol->nums;
        }
    }

    if(aol->aol.N)
    {
        vpstack()->AskOnline(&aol->aol);
    }
    else
    {
        free(aol);
    }

    s_semaphore.release();
}

//  ContactListForm::onSmsForward
//  -------------------------------------------------------------------------
//void    ContactListForm::onSmsForward()
//{
//}

//  ContactListForm::onTableClicked
//  -------------------------------------------------------------------------
void    ContactListForm::onTableClicked(
                    const QModelIndex&    index)
{
    QSqlQueryModel*     model   = static_cast<QSqlQueryModel*>(m_table->model());
    QSqlRecord          record  = model->record(index.row());
    int                 id      = record.value("id").toInt();
    QString             vpnumber= record.value("username").toString();
    QString             mobile  = record.value("mobile").toString();
    QString             text    = index.model()->data(index, Qt::DisplayRole).toString();
    bool                offline = getUserStatus(id) != AOL_ONLINE;

    if(     (offline && (index.column() != CONTACT_COLUMN_INDEX_MOBILECALL)|| text == "")
        &&  (index.column() != CONTACT_COLUMN_INDEX_USERNAME)
        &&  (index.column() != CONTACT_COLUMN_INDEX_SMS))
    {
        //  this item is disabled
        return;
    }

    if(m_lastClickedRow != index.row())
    {
        m_lastClickedRow    = index.row();
        return;
    }

    m_click.play();

    if(m_full == false)
    {
        emit    contactSelected(vpnumber);
        m_full  = true;
        hideMe(this);
        return;
    }

    switch(index.column())
    {
        case    CONTACT_COLUMN_INDEX_USERNAME   :
        {
            emit editContact(id);
        } break;

        case    CONTACT_COLUMN_INDEX_IPCALL     :
        {
            emit    makeCall(vpnumber, BC_VOICE);
        } break;

        case    CONTACT_COLUMN_INDEX_MOBILECALL :
        {
            if(!mobile.isEmpty())
            {
                emit    makeCall(mobile, BC_VOICE);
            }
        } break;

        case    CONTACT_COLUMN_INDEX_VIDEO      :
        {
            emit    makeCall(vpnumber, BC_AUDIOVIDEO);
        } break;
        case    CONTACT_COLUMN_INDEX_VIDEOMSG   :
        {
            emit    makeCall(vpnumber, BC_VIDEOMSG);
        } break;

        case    CONTACT_COLUMN_INDEX_SMS        :
        {
            if(!vpnumber.isEmpty())
                emit    makeCall(vpnumber, BC_SMS);
            else if(!mobile.isEmpty())
                emit    makeCall(mobile, BC_SMS);
        } break;
        case    CONTACT_COLUMN_INDEX_FILE       :
        {
            emit    makeCall(vpnumber, BC_FILE);
        } break;

        case    CONTACT_COLUMN_INDEX_CHAT       :
        {
            emit    makeCall(vpnumber, BC_CHAT);
        } break;
    }
}

//  ContactListForm::aolAck
//  -------------------------------------------------------------------------
void    ContactListForm::aolAck(    bool                nlast,
                                    AOL*                aol)
{
    AOL_t*          ack = (AOL_t*)aol;

    if(aol == NULL)
    {
        return;
    }

    for(int i = 0; i < ack->aol.N; ++i)
    {
        int     id          = ack->ids[i];
        QString vpnumber    = s_id2vpnumber[id];

        s_vpnumber2status[vpnumber] = ack->nums[i].online;
    }

    if(!nlast)
    {
        free(ack);
    }
    else
    {
        m_timer.start(1000);
    }

    repaint();
}

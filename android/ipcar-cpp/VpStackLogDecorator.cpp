#include "VpStackLogDecorator.h"

#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

#include "LogEntry.h"
#include "Utils.h"

VpStackLogDecorator::VpStackLogDecorator(
                    IVPSTACK*           base,
                    QObject*            parent)
    : VpStackDecorator(base, parent)
{
}

int         VpStackLogDecorator::Disconnect(
                    VPCALL              vpcall,
                    int                 reason)
{
    switch(reason)
    {
        case    REASON_BUSY :
        {
            updateCallType(vpcall, logtBusy | logtRead);
        } break;
        
        default:
            updateCallType(vpcall, logtRead);
    };

    return  m_base->Disconnect(
                        vpcall,
                        reason);
}

int         VpStackLogDecorator::AnswerCall(
                    VPCALL              vpcall)
{
    updateCallType(vpcall, logtAnswered | logtRead);

    return  m_base->AnswerCall(
                        vpcall);
}

int         VpStackLogDecorator::SetCallFilePath(
                    VPCALL              vpcall,
                    const char*         path)
{
    if(m_callid.find(vpcall) != m_callid.end())
    {
        QSqlQuery   query;
        int         id  = m_callid[vpcall];

        query.exec(QString("INSERT INTO file VALUES(NULL, '%1', '%2')")
                        .arg(id)
                        .arg(path));
        LOG_SQL_QUERY(query);
    }

    return  m_base->SetCallFilePath(vpcall, path);
}


//  VpStackLogDecorator::addCall
//  -------------------------------------------------------------------------
void        VpStackLogDecorator::addCall(
                            VPCALL              call)
{
    if(m_base->GetCallBearer(call) == BC_NOTHING)
        return;

    char    username[MAXNAMELEN + 1];
    char    vpnumber[MAXNUMBERLEN + 1];

    m_base->GetCallRemoteName(  call, username);
    m_base->GetCallRemoteNumber(call, vpnumber);
    m_base->GetCallBearer(      call);

    QSqlQuery   query;
    char        message[MAXMESSAGELEN + 1];

    m_base->GetCallText(call, message);

    query.exec(QString("INSERT INTO log VALUES(NULL, '%1', '%2', '%3', '%4', datetime(), datetime(), '%5')")
                    .arg(m_userid)
                    .arg(username)
                    .arg(vpnumber)
                    .arg(message)
                    .arg(m_base->GetCallBearer(call) << 8));
    LOG_SQL_QUERY(query);

    query.exec(QString("SELECT max(id) FROM log"));
    LOG_SQL_QUERY(query);

    query.next();
    m_callid[call] = query.value(0).toInt();
}

//  VpStackLogDecorator::updateCallType
//  -------------------------------------------------------------------------
void        VpStackLogDecorator::updateCallType(
                            VPCALL              call,
                            int                 type)
{
    if(m_base->GetCallBearer(call) == BC_NOTHING)
        return;

    int         id      = m_callid[call];

    QSqlQuery   query;

    query.exec(QString("UPDATE log SET type = (type | %1) WHERE id = '%2'")
                .arg(type)
                .arg(id));
    LOG_SQL_QUERY(query);
}

//  VpStackLogDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackLogDecorator::vpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    VPCALL  call    = reinterpret_cast<VPCALL>(pcall);

    switch(msg)
    {
        case	VPMSG_SERVERSTATUS  :
        {
            switch(pcall)
            {
                case    REASON_LOGGEDON     :
                {
                    OpenDatabase(m_base->LogonName(), m_base->LogonNumber(), m_base->LogonNumber());

                    QSqlQuery   query;
                    query.exec(QString("SELECT id FROM user WHERE username == '%1'").arg(m_base->LogonName()));
                    LOG_SQL_QUERY(query);

                    if(query.next())
                    {
                        m_userid    = query.value(0).toInt();
                    }
                } break;

                case    REASON_AUTHERROR    :
                {
                    // ???
                } break;
            }
        } break;

        //  VPMSG_NEWCALL
        //  -----------------------------------------------------------------
        case    VPMSG_NEWCALL       :
        {
            int bc  = m_base->GetCallBearer(call);

            addCall(call);

            if(bc == BC_SMS)
            {
                updateCallType(call, logtIncomming | logtAnswered);
            }
            else
            {
                updateCallType(call, logtIncomming);
            }
        } break;

        //  VPMSG_CALLACCEPTED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLACCEPTED  :
        {
            addCall(call);
            updateCallType(call, logtOutgoing | logtAnswered);
        } break;

        case    VPMSG_CALLREFUSED   :
        {
            addCall(call);
            updateCallType(call, logtOutgoing);
        }

        //  VPMSG_CALLENDED
        //  -----------------------------------------------------------------
        case    VPMSG_CALLENDED         :
        {
            if(m_base->GetCallBearer(call) == BC_NOTHING)
                break;

            QSqlQuery   query;
            query.exec(QString("UPDATE log SET ended_on = datetime() WHERE id = '%1'").arg(m_callid[call]));
            LOG_SQL_QUERY(query);

            m_callid.remove(call);
        } break;


    }

    DefNotificationRoutine(msg, pcall, param);
}

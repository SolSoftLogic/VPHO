#include "VpStackContactDecorator.h"
#include "Utils.h"

#include <QtCore/QTemporaryFile>
#include <QtCore/QTextStream>
#include <QtCore/QVariant>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

//  VpStackContactDecorator::VpStackContactDecorator
//  -------------------------------------------------------------------------
VpStackContactDecorator::VpStackContactDecorator(
                            IVPSTACK*           base,
                            QObject*            parent)
    : VpStackDecorator(base, parent)
{

}

//  VpStackContactDecorator::ServerTransfer
//  -------------------------------------------------------------------------
int         VpStackContactDecorator::ServerTransfer(
                int                 op,
                const char*         path,
                bool                amactive)
{
    if(op == TCPFT_RXAB || op == TCPFT_TXAB)
    {
        QList<serverTransfer_t>&list    = op == TCPFT_TXAB
                                        ? m_rxlist
                                        : m_txlist;
        serverTransfer_t    entry;

        entry.op        = op;
        entry.path      = path;
        entry.amactive  = amactive;

        list.push_back(entry);

        if(list.size() == 1)
        {
            int result;

            result  = m_base->ServerTransfer(op, path, amactive);

            if(result != 0)
            {
                list.pop_back();
            }
        }

        return  0;
    }
    else
    {
        return  m_base->ServerTransfer(op, path, amactive);
    }

}

//  VpStackContactDecorator::rxAbook
//  -------------------------------------------------------------------------
void        VpStackContactDecorator::rxAbook(const QString& filename)
{
    QFile       file(filename);

    if(file.open(QIODevice::ReadOnly))
    {
        QSqlQuery   query;

        query.exec(QString("SELECT id FROM user WHERE username = '%1'")
                   .arg(m_base->LogonName()));
        LOG_SQL_QUERY(query);

        query.first();

        int             owner   = query.value(0).toInt();

        query.exec("BEGIN");
        LOG_SQL_QUERY(query);

        QTextStream     stream(&file);

        while(!stream.atEnd())
        {
            QString     line;

            line    = stream.readLine();

            QSqlQuery   query;
            query.exec(line.arg(owner));
            LOG_SQL_QUERY(query);

            if(query.lastError().type() != QSqlError::NoError)
            {
                query.exec("ROLLBACK");
                LOG_SQL_QUERY(query);
                return;
            }

        }

        query.exec("COMMIT");
        LOG_SQL_QUERY(query);
    }
}

//  VpStackContactDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void        VpStackContactDecorator::vpstackNotificationRoutine(
                unsigned            msg,
                unsigned            pcall,
                unsigned            param)
{
    DefNotificationRoutine(msg, pcall, param);

    if(     msg == VPMSG_SERVERSTATUS && pcall == REASON_LOGGEDON)
    {
        QTemporaryFile  file;

        file.open();
        ServerTransfer(TCPFT_RXAB, qPrintable(file.fileName()), true);
    }

    if(     msg == VPMSG_SERVERTRANSFERFINISHED
       && (     pcall == TCPFT_TXAB
            ||  pcall == TCPFT_RXAB))
    {
        QList<serverTransfer_t>&list    = pcall == TCPFT_TXAB
                                        ? m_rxlist
                                        : m_txlist;
        serverTransfer_t    entry;

        entry   = list.front();

        if(pcall == TCPFT_RXAB)
        {
            rxAbook(entry.path);
        }

        list.pop_front();

        if(!list.empty())
        {
            entry   = list.front();

            m_base->ServerTransfer(entry.op, qPrintable(entry.path), entry.amactive);
        }
    }
}


#include "VpStackAolDecorator.h"

//  VpStackAolDecorator::VpStackAolDecorator
//  -------------------------------------------------------------------------
VpStackAolDecorator::VpStackAolDecorator(
                    IVPSTACK*           base,
                    QObject*            parent)
    : VpStackDecorator(base, parent)
{
}

//  VpStackAolDecorator::AskOnline
//  -------------------------------------------------------------------------
int     VpStackAolDecorator::AskOnline(
                    AOL*                aol)
{
    m_fifo.append(aol);

    if(m_fifo.size() == 1)
    {
        return  m_base->AskOnline(aol);
    }

    return  0;
}

//  VpStackAolDecorator::vpstackNotificationRoutine
//  -------------------------------------------------------------------------
void    VpStackAolDecorator::vpstackNotificationRoutine(
                            unsigned            msg,
                            unsigned            pcall,
                            unsigned            param)
{
    switch(msg)
    {
        case    VPMSG_QUERYONLINEACK    :
        {
            if(!m_fifo.empty())
            {
                AOL*    aol = m_fifo.first();

                DefNotificationRoutine(VPMSG_QUERYONLINEACK, pcall, (unsigned int)aol);

                //  call == 1 -> more data will follow
                //  call == 0 -> this is the last one
                if(pcall == 0)
                {
                    m_fifo.pop_front();

                    if(!m_fifo.empty())
                    {
                        m_base->AskOnline(m_fifo.first());
                    }
                }
            }
            return;
        } break;

        default:
            DefNotificationRoutine(msg, pcall, param);
    }

}

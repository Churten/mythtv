/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt3Support module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License versions 2.0 or 3.0 as published by the Free Software
** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file.  Alternatively you may (at
** your option) use any later version of the GNU General Public
** License if such license has been publicly approved by Trolltech ASA
** (or its successors, if any) and the KDE Free Qt Foundation. In
** addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.2, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** Please review the following information to ensure GNU General
** Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/. If
** you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech, as the sole
** copyright holder for Qt Designer, grants users of the Qt/Eclipse
** Integration plug-in the right for the Qt/Eclipse Integration to
** link to functionality provided by Qt Designer and its related
** libraries.
**
** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE. Trolltech reserves all rights not expressly
** granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "mmembuf.h"

// *******************************************************************
// QMembuf declaration and implementation
// *******************************************************************

/*  \internal
    This class implements an efficient buffering of data that is often used by
    asynchronous IO classes like QSocket, QHttp and QProcess.
*/

MMembuf::~MMembuf()
{
    while (!m_buf.isEmpty())
        delete m_buf.takeFirst();
}

/*! \internal
    This function consumes \a nbytes bytes of data from the
    buffer and copies it into \a sink. If \a sink is a 0 pointer
    the data goes into the nirvana.
*/
bool MMembuf::consumeBytes(quint64 nbytes, char *sink)
{
    if (nbytes == 0 || (qint64)nbytes > m_size)
        return false;
    m_size -= nbytes;
    while (!m_buf.isEmpty()) {
        QByteArray *a = m_buf.first();
        if ((int)(m_index + nbytes) >= a->size()) {
            // Here we skip the whole byte array and get the next later
            int len = a->size() - m_index;
            if (sink) {
                memcpy(sink, a->constData()+m_index, len);
                sink += len;
            }
            nbytes -= len;
            m_buf.removeFirst();
	    delete a;
            m_index = 0;
            if (nbytes == 0)
                break;
        } else {
            // Here we skip only a part of the first byte array
            if (sink)
                memcpy(sink, a->constData()+m_index, nbytes);
            m_index += nbytes;
            break;
        }
    }
    return true;
}

/*! \internal
    Scans for any occurrence of '\n' in the buffer. If \a store
    is not 0 the text up to the first '\n' (or terminating 0) is
    written to \a store, and a terminating 0 is appended to \a store
    if necessary. Returns true if a '\n' was found; otherwise returns
    false.
*/
bool MMembuf::scanNewline(QByteArray *store)
{
    if (m_size == 0)
        return false;
    int i = 0; // index into 'store'
    bool retval = false;
    for (int j = 0; j < m_buf.size(); ++j) {
        QByteArray *a = m_buf.at(j);
        char *p = a->data();
        int n = a->size();
        if (!j) {
            // first buffer
            p += m_index;
            n -= m_index;
        }
        if (store) {
            while (n-- > 0) {
                *(store->data()+i) = *p;
                if (++i == store->size())
                    store->resize(store->size() < 256
                                   ? 1024 : store->size()*4);
                if (*p == '\n') {
                    retval = true;
                    goto end;
                }
                p++;
            }
        } else {
            while (n-- > 0) {
                if(*p == '\n')
                    return true;
                p++;
            }
        }
    }
 end:
    if (store)
        store->resize(i);
    return retval;
}

int MMembuf::ungetch(int ch)
{
    if (m_buf.isEmpty() || m_index==0) {
        // we need a new QByteArray
        auto *ba = new QByteArray;
        ba->resize(1);
        m_buf.prepend(ba);
        m_size++;
        (*ba)[0] = ch;
    } else {
        // we can reuse a place in the buffer
        QByteArray *ba = m_buf.first();
        m_index--;
        m_size++;
        (*ba)[(int)m_index] = ch;
    }
    return ch;
}

// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * (c) 2017, Deepin Technology Co., Ltd. <support@deepin.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#ifndef _DMR_DVD_UTILS_H
#define _DMR_DVD_UTILS_H

#define _DMR_DVD_UTILS_H

#include <QtCore>
#include <QThread>

namespace dmr {
//add by xxj
#ifdef heyi
namespace dvd {
// device could be a dev node or a iso file
QString RetrieveDVDTitle(const QString &device);
/*
   class RetrieveDvdThread
   the class function DVD thread, Retrieve DVD and get DVD message
   todo Handle dvdnav_open blocking of the dvdnav library function
*/
class RetrieveDvdThread: public QThread
{
    Q_OBJECT

public:
    explicit RetrieveDvdThread();
    ~RetrieveDvdThread();

    static RetrieveDvdThread *get();
    void startDvd(const QString &dev);

    // device could be a dev node or a iso file
    QString getDvdMsg(const QString &device);

protected:
    void run();

signals:
    void sigData(const QString &title);

private:
    QAtomicInt _quit{0};
    QString m_dev {QString()};

};

}
#endif
}

#endif /* ifndef _DMR_DVD_UTILS_H */


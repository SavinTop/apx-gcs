﻿/*
 * Copyright (C) 2011 Aliaksei Stratsilatau <sa@uavos.com>
 *
 * This file is part of the UAV Open System Project
 *  http://www.uavos.com/
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth
 * Floor, Boston, MA 02110-1301, USA.
 *
 */
#pragma once

#include "VehiclesDB.h"
#include <QtCore>

class DBReqVehiclesLoadNconf : public DBReqVehicles
{
    Q_OBJECT
public:
    explicit DBReqVehiclesLoadNconf(quint64 nconfID)
        : DBReqVehicles()
        , nconfID(nconfID)
    {}
    explicit DBReqVehiclesLoadNconf(QString sn)
        : DBReqVehicles(sn)
        , nconfID(0)
    {}
    virtual bool run(QSqlQuery &query);

    //result
    QVariantMap info;
    QVariantMap values;

protected:
    quint64 nconfID;
signals:
    void nconfFound(quint64 nconfID);
    void configLoaded(QVariantMap info, QVariantMap values);
};

class DBReqVehiclesLoadNconfLatest : public DBReqVehiclesLoadNconf
{
    Q_OBJECT
public:
    explicit DBReqVehiclesLoadNconfLatest(QString sn)
        : DBReqVehiclesLoadNconf(sn)
    {}

protected:
    bool run(QSqlQuery &query);
};

class DBReqVehiclesSaveNconf : public DBReqVehicles
{
    Q_OBJECT
public:
    explicit DBReqVehiclesSaveNconf(QVariantMap dictInfo, QVariantMap values, quint64 time)
        : DBReqVehicles(dictInfo.value("sn").toString())
        , nconfID(0)
        , dictInfo(dictInfo)
        , values(values)
        , time(time ? time : QDateTime::currentDateTime().toMSecsSinceEpoch())
    {}
    bool run(QSqlQuery &query);
    //result
    quint64 nconfID;

private:
    QVariantMap dictInfo;
    QVariantMap values;
    quint64 time;
signals:
    void nconfFound(quint64 nconfID);
};
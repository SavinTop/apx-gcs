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

#include <ApxMisc/DelayedEvent.h>
#include <Database/DatabaseRequest.h>
#include <Fact/Fact.h>
#include <Protocols/ProtocolMission.h>
#include <QGeoCoordinate>
#include <QtCore>
class VehicleMission;
class Vehicle;
class MissionShare;
class MissionGroup;
class LookupMissions;

class MissionStorage : public QObject
{
    Q_OBJECT
    Q_ENUMS(MissionItemType)

public:
    explicit MissionStorage(VehicleMission *mission);

    VehicleMission *mission;
    QString dbHash;

    ProtocolMission::Mission saveToDict() const;
    void loadFromDict(ProtocolMission::Mission d);

private:
    void saveItemsToDict(QList<ProtocolMission::Item> &items, const MissionGroup *g) const;
    void loadItemsFromDict(const QList<ProtocolMission::Item> &items, MissionGroup *g) const;

    DelayedEvent evtUpdateSite;

private slots:
    //database
    void dbSaved(QString hash);
    void dbLoaded(QVariantMap info, QVariantMap details, ProtocolMission::Mission data);
    void dbSiteFound(quint64 siteID, QString site);
    void dbFindSite();

public slots:
    void saveMission();
    void loadMission(QString hash);

signals:
    void loaded();
    void saved();
};

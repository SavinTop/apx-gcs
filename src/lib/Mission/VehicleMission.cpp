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
#include "VehicleMission.h"
#include "Vehicle.h"
#include "MissionItems.h"
#include "Waypoint.h"

//pack
#include <Mandala.h>
#include <Mission.h>
#include <node.h>
//=============================================================================
VehicleMission::VehicleMission(Vehicle *parent)
  : Fact(parent,"mission","Mission",tr("Vehicle mission"),GroupItem,NoData),
    vehicle(parent)
{
  f_request=new Fact(this,"request",tr("Request"),tr("Download from vehicle"),FactItem,ActionData);
  connect(f_request,&Fact::triggered,vehicle,&Vehicle::requestMission);

  f_upload=new Fact(this,"upload",tr("Upload"),tr("Upload to vehicle"),FactItem,ActionData);
  connect(f_upload,&Fact::triggered,this,&VehicleMission::upload);
  connect(f_upload,&Fact::enabledChanged,this,&VehicleMission::actionsUpdated);

  f_stop=new Fact(this,"stop",tr("Stop"),tr("Stop data requests"),FactItem,ActionData);
  connect(f_stop,&Fact::triggered,this,&VehicleMission::stop);
  connect(f_stop,&Fact::enabledChanged,this,&VehicleMission::actionsUpdated);

  f_waypoints=new MissionItems(this,"waypoints",tr("Waypoints"),"");
  f_runways=new MissionItems(this,"runways",tr("Runways"),"");

  connect(this,&Fact::modifiedChanged,this,&VehicleMission::updateActions);

  updateActions();

  if(!vehicle->isLocal()){
    //f_request->trigger();
    //QTimer::singleShot(1000,f_request,&Fact::trigger);
  }

  //qmlRegisterUncreatableType<Vehicles>("GCS.Vehicles", 1, 0, "Vehicles", "Reference only");

  FactSystem::instance()->jsSync(this);
}
//=============================================================================
void VehicleMission::updateActions()
{
  /*bool busy=false;//model->requestManager.busy();
  bool upgrading=false;//model->isUpgrading();
  bool bModAll=modified();
  bool bEmpty=nodesCount()<=0;
  f_upload->setEnabled(bModAll && (!(busy)));
  f_stop->setEnabled(busy||upgrading);
  f_reload->setEnabled(!(upgrading||bEmpty));*/
}
//=============================================================================
//=============================================================================
void VehicleMission::clear()
{
  /*if(snMap.isEmpty())return;
  snMap.clear();
  nGroups.clear();
  f_list->removeAll();
  setModified(false);
  f_list->setModified(false);*/
  FactSystem::instance()->jsSync(this);
}
//=============================================================================
void VehicleMission::upload()
{
  /*foreach(NodeItem *node,snMap.values()){
    node->upload();
  }*/
}
//=============================================================================
void VehicleMission::stop()
{
}
//=============================================================================
bool VehicleMission::unpackMission(const QByteArray &ba)
{
  _bus_packet &packet=*(_bus_packet*)ba.data();
  uint data_cnt=ba.size();
  if(data_cnt<bus_packet_size_hdr)return false;
  data_cnt-=bus_packet_size_hdr;
  if(data_cnt<4)return false;
  if(packet.id!=idx_mission) return false;
  int wpcnt=0,rwcnt=0;
  const uint8_t *ptr=packet.data;
  for(uint cnt=0;data_cnt>=sizeof(Mission::_item_hdr);data_cnt-=cnt){
    ptr+=cnt;
    const Mission::_item_hdr *hdr=(Mission::_item_hdr*)ptr;
    switch(hdr->type){
      case Mission::mi_stop:
        data_cnt-=sizeof(Mission::_item_hdr);
      break;
      case Mission::mi_wp:
      {
        cnt=sizeof(Mission::_item_wp);
        if(data_cnt<cnt)break;
        wpcnt++;
        const Mission::_item_wp *e=(Mission::_item_wp*)ptr;
        Waypoint *f=new Waypoint(f_waypoints);
        f->f_altitude->setValue(e->alt);
        f->f_type->setValue(e->hdr.type);
        f->f_latitude->setValue(e->lat);
        f->f_longitude->setValue(e->lon);
      }
      continue;
      case Mission::mi_rw:
      {
        const Mission::_item_rw *e=(Mission::_item_rw*)ptr;
        cnt=sizeof(Mission::_item_rw);
        if(data_cnt<cnt)break;
        rwcnt++;
      }
      continue;
      case Mission::mi_tw:
      {
        const Mission::_item_tw *e=(Mission::_item_tw*)ptr;
        cnt=sizeof(Mission::_item_tw);
        if(data_cnt<cnt)break;
      }
      continue;
      case Mission::mi_pi:
      {
        const Mission::_item_pi *e=(Mission::_item_pi*)ptr;
        cnt=sizeof(Mission::_item_pi);
        if(data_cnt<cnt)break;
      }
      continue;
      case Mission::mi_action:
      {
        const Mission::_item_action *e=(Mission::_item_action*)ptr;
        cnt=Mission::action_size(e->hdr.option);
        if(data_cnt<cnt)break;
      }
      continue;
      case Mission::mi_restricted:
      {
        const Mission::_item_area *e=(Mission::_item_area*)ptr;
        cnt=Mission::area_size(e->pointsCnt);
        if(data_cnt<cnt)break;
      }
      continue;
      case Mission::mi_emergency:
      {
        const Mission::_item_area *e=(Mission::_item_area*)ptr;
        cnt=Mission::area_size(e->pointsCnt);
        if(data_cnt<cnt)break;
      }
      continue;
    }
    break;
  }
  if(data_cnt){
    qWarning()<<"error in mission";
  }else{
    qDebug()<<"Mission received"<<wpcnt<<rwcnt;
  }

  return true;
}
//=============================================================================

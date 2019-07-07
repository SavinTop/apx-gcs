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
#include "AppEngine.h"
#include <ApxDirs.h>
#include <ApxLog.h>
//=============================================================================
AppEngine::AppEngine(QObject *parent)
    : QQmlApplicationEngine(parent)
{
    installExtensions(QJSEngine::AllExtensions);
    //installExtensions(QJSEngine::TranslationExtension|QJSEngine::ConsoleExtension);
    addImportPath(ApxDirs::userPlugins().absolutePath());
    addImportPath("qrc:/");

    // QML types register
    qmlRegisterUncreatableType<Fact>("APX.Facts", 1, 0, "Fact", "Reference only");
    qmlRegisterUncreatableType<FactAction>("APX.Facts", 1, 0, "FactAction", "Reference only");

    jsRegisterFunctions();

    // script include file (default)
    QFile jsFile(ApxDirs::res().filePath("scripts/gcs.js"));
    if (jsFile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&jsFile);
        QString contents = stream.readAll();
        jsFile.close();
        jsexec(contents);
    }
    // script include file (user commands)
    QFile jsFile2(ApxDirs::scripts().filePath("gcs.js"));
    if (jsFile2.open(QIODevice::ReadOnly)) {
        QTextStream stream(&jsFile2);
        QString contents = stream.readAll();
        jsFile2.close();
        jsexec(contents);
    }

    //add is queued to wait inherited constructors
    //connect(this,&Fact::itemAdded,this,&AppEngine::jsAddItem);//,Qt::QueuedConnection);
    //connect(this,&Fact::itemRemoved,this,&AppEngine::jsRemoveItem);//,Qt::QueuedConnection);
}
//=============================================================================
void AppEngine::jsSyncObject(QObject *obj)
{
    QQmlEngine::setObjectOwnership(obj, QQmlEngine::CppOwnership);
    globalObject().setProperty(obj->objectName(), newQObject(obj));
}
//=============================================================================
void AppEngine::jsSync(Fact *item)
{
    //qDebug()<<item;
    QList<FactBase *> list = item->pathList();
    QJSValue v = globalObject();
    for (int i = list.size() - 1; i > 0; --i) {
        Fact *fact = static_cast<Fact *>(list.at(i));
        QJSValue vp = v.property(fact->name());
        if (vp.isUndefined() || (!vp.isQObject()) || vp.toQObject() != fact) {
            vp = newQObject(fact);
            v.setProperty(fact->name(), vp);
        }
        v = vp;
    }
    jsSync(item, v);
    //collectGarbage();
}
//=============================================================================
QJSValue AppEngine::jsSync(Fact *factItem, QJSValue parent) //recursive
{
    //qDebug()<<factItem->path();
    QQmlEngine::setObjectOwnership(factItem, QQmlEngine::CppOwnership);
    QJSValue js_factItem = newQObject(factItem); //toScriptValue<Fact*>(factItem);//
    parent.setProperty(factItem->name(), js_factItem);
    for (int i = 0; i < factItem->size(); ++i)
        jsSync(factItem->child(i), js_factItem);
    if (!factItem->actions.isEmpty()) {
        QJSValue js_actions = newObject();
        foreach (FactAction *i, factItem->actions) {
            QQmlEngine::setObjectOwnership(i, QQmlEngine::CppOwnership);
            QJSValue js_action = newQObject(i);
            js_actions.setProperty(i->name(), js_action);
            if (i->fact()) {
                jsSync(i->fact(), js_factItem);
            }
        }
        js_factItem.setProperty("action", js_actions);
    }
    return js_factItem;
}
//=============================================================================
QJSValue AppEngine::jsexec(const QString &s)
{
    QJSValue result;
    result = evaluate(s);
    if (result.isError()) {
        apxMsgW() << result.toString();
    }
    return result;
}
//=============================================================================
void AppEngine::jsRegister(QString fname, QString description, QString body)
{
    jsexec(QString("function %1 { %2;};").arg(fname).arg(body));
    jsexec(QString("%1.info=\"%2\";").arg(fname.left(fname.indexOf('('))).arg(description));
    js_descr[fname] = description;
}
//=============================================================================
//=============================================================================
void AppEngine::jsAddItem(FactBase *item)
{
    //qDebug()<<static_cast<Fact*>(item)->path();
    QQmlEngine::setObjectOwnership(item, QQmlEngine::CppOwnership);
    //find the parents tree, last item in list = this
    QList<FactBase *> list = item->pathList();
    QJSValue v = globalObject();
    for (int i = list.size() - 1; i >= 0; --i) {
        Fact *fact = static_cast<Fact *>(list.at(i));
        QJSValue vp = v.property(fact->name());
        if (vp.isUndefined() || (!vp.isQObject()) || vp.toQObject() != fact) {
            vp = newQObject(fact);
            v.setProperty(fact->name(), vp);
            //qDebug()<<fact->path();
        }
        v = vp;
    }
    //collectGarbage();
}
void AppEngine::jsRemoveItem(FactBase *item)
{
    //qDebug()<<"jsRemoveItem"<<static_cast<Fact*>(item)->path();
    //find the parents tree, last item in list = this
    QList<FactBase *> list = item->pathList();
    QJSValue v = globalObject();
    for (int i = list.size() - 1; i > 0; --i) {
        Fact *fact = static_cast<Fact *>(list.at(i));
        QJSValue vp = v.property(fact->name());
        if (vp.isUndefined())
            return; //no parents?
        v = vp;
    }
    v.deleteProperty(item->name());
}
//=============================================================================
//=============================================================================
void AppEngine::jsRegisterFunctions()
{
    //system functions and objects
    jsRegister("help()", QObject::tr("print commands help"), "application.engine.help();");
    jsRegister("req(n)",
               QObject::tr("request var n from UAV"),
               "apx.vehicles.current.mandala[n].request();");
    jsRegister("send(n)",
               QObject::tr("send var n to UAV"),
               "apx.vehicles.current.mandala[n].send();");
    jsRegister("nodes()", QObject::tr("rescan bus nodes"), "apx.vehicles.current.nodes.request();");
    jsRegister("nstat()",
               QObject::tr("print nodes status"),
               "print('nodes statistics:');apx.vehicles.current.nodes.nstat();");
    jsRegister("serial(p,v)",
               QObject::tr("send data v to serial port ID p"),
               "apx.vehicles.current.sendSerial(p,v);");
    jsRegister("vmexec(f)",
               QObject::tr("execute function on VMs"),
               "apx.vehicles.current.vmexec(f);");
    jsRegister("sleep(n)", QObject::tr("sleep n milliseconds"), "application.engine.sleep(n);");
    jsRegister("next()", QObject::tr("switch to next vehicle"), "apx.vehicles.selectNext();");
    jsRegister("prev()", QObject::tr("switch to previous vehicle"), "apx.vehicles.selectPrev();");

    //some helper functions
    jsRegister("trigger(v,a,b)",
               QObject::tr("trigger value of v to a or b"),
               "if(v==a)return b; else return a;");
    jsRegister("bound(v)",
               QObject::tr("wrap angle -180..+180"),
               "while(v>=180)v-=360;while(v<-180)v+=360;return v;");
    jsRegister("ls(a,b)",
               QObject::tr("print members of type b for scope a"),
               "for(var i in a)if(typeof(a[i])==b || !b)print(i+\" - \"+typeof(a[i]));");
    jsRegister("vars(a)",
               QObject::tr("print variables for scope a"),
               "if(arguments.length==0)a=this;for(var i in "
               "a)if(typeof(a[i])=='number')print(i+\"=\"+a[i]);");
    jsRegister(
        "func(a)",
        QObject::tr("print functions for scope a"),
        "if(arguments.length==0)a=this;for(var i in a)if(typeof(a[i])=='function')print(i);");
    //predefined commands
    jsRegister("ahrs()", QObject::tr("reset AHRS"), "req('roll');");
    jsRegister("zrc()",
               QObject::tr("reset pilot controls"),
               "rc_roll=0;rc_pitch=0;rc_throttle=0;rc_yaw=0;");
    jsRegister("zps()", QObject::tr("reset barometric altitude on ground"), "altps_gnd=0;");
    jsRegister("ned()",
               QObject::tr("reset local GPS coordinates"),
               "home_lat=gps_lat;home_lon=gps_lon;home_hmsl=gps_hmsl;");
    jsRegister("hmsl()", QObject::tr("reset local GPS altitude"), "home_hmsl=gps_hmsl;");
}
//=============================================================================
void AppEngine::help()
{
    QString s;
    s += "<html><table>";
    foreach (const QString &cmd, js_descr.keys()) {
        s += "<tr><td valign='middle'>";
        s += "<nobr>&nbsp;<font color=cyan>";
        s += cmd;
        s += "</font></nobr></td><td>"; // width='100%'>";
        s += "<font color=gray> &nbsp;" + js_descr.value(cmd) + "</font>";
        s += "</td></tr>";
    }
    s += "</table>";
    apxMsg() << s;
}
//=============================================================================
void AppEngine::sleep(quint16 ms)
{
    QEventLoop loop;
    QTimer::singleShot(ms, &loop, SLOT(quit()));
    loop.exec();
}
//=============================================================================
//=============================================================================
QByteArray AppEngine::jsToArray(QJSValue data)
{
    //qDebug()<<portID<<data.toString()<<data.isArray()<<data.toVariant().toByteArray().toHex();
    QByteArray ba;
    if (data.isString() || data.toString().contains(',')) {
        bool ok = false;
        foreach (QString sv,
                 data.toString().trimmed().toLower().split(',', QString::SkipEmptyParts)) {
            uint v;
            sv = sv.trimmed();
            if (sv.startsWith("0x"))
                v = sv.mid(2).toUInt(&ok, 16);
            else
                v = sv.toUInt(&ok, 10);
            if (!ok)
                break;
            ba.append(static_cast<char>(v));
        }
        if (!ok)
            return QByteArray();
    } else if (data.isNumber()) {
        ba.append(static_cast<char>(data.toInt()));
    } else
        return QByteArray();
    return ba;
}
//=============================================================================
QJSValue AppEngine::jsGetProperty(const QString &path)
{
    QStringList list = path.split('.');
    QJSValue v = globalObject();
    for (int i = 0; i < list.size(); ++i) {
        QJSValue vp = v.property(list.at(i));
        if (vp.isUndefined())
            return QJSValue();
        v = vp;
    }
    return v;
}
//=============================================================================
//=============================================================================
QObject *AppEngine::loadQml(const QString &qmlFile)
{
    //qDebug()<<qmlFile;
    QQmlComponent c(this, qmlFile, QQmlComponent::PreferSynchronous);
    //setObjectOwnership(&c,CppOwnership);
    QObject *obj = c.create();
    c.completeCreate();
    if (c.isError()) {
        apxMsgW() << c.errorString();
        return nullptr;
    }
    //setObjectOwnership(obj,CppOwnership);
    return obj;
}
//=============================================================================
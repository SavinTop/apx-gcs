/*
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
#include "ProtocolServiceFile.h"
#include "ProtocolServiceNode.h"
#include "ProtocolServiceRequest.h"

#include <ApxLink/node.h>
//=============================================================================
ProtocolServiceFile::ProtocolServiceFile(ProtocolServiceNode *node, quint16 cmdBase)
    : ProtocolBase(node)
    , node(node)
    , reqFileRead(nullptr)
    , reqFileWrite(nullptr)
    , reqRead(nullptr)
    , reqWrite(nullptr)
    , dataAddr(0)
    , dataSize(0)
{
    cmd_file = cmdBase + _cmd_file;
    cmd_read = cmdBase + _cmd_read;
    cmd_write = cmdBase + _cmd_write;
    connect(node, &ProtocolServiceNode::unknownServiceData, this, &ProtocolServiceFile::serviceData);
}
//=============================================================================
void ProtocolServiceFile::download()
{
    node->setProgress(0);
    qDebug() << node->name();
    reqFileRead = node->request(cmd_file, QByteArray(), 1000, false);
    //req->rxCntFilter=sizeof(_flash_file);
    //connect(req,&ProtocolServiceRequest::requestReplyData,this,&ProtocolServiceFile::fileReadReply);
}
//=============================================================================
void ProtocolServiceFile::upload(QByteArray data)
{
    qDebug() << node->name();
    node->setProgress(0);
    wdata = data;
    dataSize = static_cast<uint>(data.size());
    _flash_file hdr;
    memset(&hdr, 0, sizeof(_flash_file));
    hdr.start_address = 0;
    hdr.size = dataSize;
    quint8 xor_crc = 0;
    for (int i = 0; i < data.size(); i++)
        xor_crc ^= static_cast<quint8>(wdata.at(i));
    hdr.xor_crc = xor_crc;
    QByteArray ba(reinterpret_cast<const char *>(&hdr), sizeof(_flash_file));
    reqFileWrite = node->request(cmd_file, ba, 3000, false);
    //req->rxCntFilter=0;
    //connect(req,&ProtocolServiceRequest::requestReplyData,this,&ProtocolServiceFile::fileWriteReply);
}
//=============================================================================
//=============================================================================
void ProtocolServiceFile::fileReadReply(QByteArray data)
{
    //qDebug()<<node->info.name<<data.toHex().toUpper();
    _flash_file hdr;
    memcpy(&hdr, data.data(), static_cast<size_t>(data.size()));
    if (hdr.size == 0) {
        qDebug() << "empty file";
        emit fileReceived(QByteArray());
        return;
    }
    //start reading flash blocks
    dataAddr = 0;
    dataSize = hdr.size;
    dataCRC = hdr.xor_crc;
    if (request_download())
        return;
    qDebug() << "empty file (hdr)";
    emit fileReceived(QByteArray());
}
//=============================================================================
void ProtocolServiceFile::fileWriteReply()
{
    //qDebug()<<node->info.name<<data.size();
    dataAddr = 0;
    if (request_upload())
        return;
    emit fileUploaded();
}
//=============================================================================
bool ProtocolServiceFile::request_download(void)
{
    if (dataAddr >= dataSize)
        return false;
    quint16 cnt = sizeof(_flash_data::data);
    uint rcnt = dataSize - dataAddr;
    _flash_data_hdr hdr;
    hdr.start_address = dataAddr;
    hdr.data_size = cnt < rcnt ? cnt : static_cast<quint16>(rcnt);
    reqRead = node->request(cmd_read,
                            QByteArray(reinterpret_cast<const char *>(&hdr), sizeof(hdr)),
                            1000,
                            false);
    //connect(req,&ProtocolServiceRequest::requestReplyData,this,&ProtocolServiceFile::readReply);
    //qDebug()<<hdr.start_address<<hdr.data_size;
    node->setProgress(dataAddr * 100 / dataSize);
    return true;
}
bool ProtocolServiceFile::request_upload(void)
{
    if (dataAddr >= dataSize)
        return false;
    quint16 cnt = sizeof(_flash_data::data);
    uint rcnt = dataSize - dataAddr;
    _flash_data_hdr hdr;
    hdr.start_address = dataAddr;
    hdr.data_size = cnt < rcnt ? cnt : static_cast<quint16>(rcnt);
    QByteArray blockData(wdata.mid(static_cast<int>(dataAddr), hdr.data_size));
    reqWrite = node->request(cmd_write,
                             QByteArray(reinterpret_cast<const char *>(&hdr), sizeof(hdr))
                                 .append(blockData),
                             1000,
                             false);
    //req->rxCntFilter=sizeof(_flash_data_hdr);
    //connect(req,&ProtocolServiceRequest::requestReplyData,this,&ProtocolServiceFile::writeReply);
    //qDebug()<<hdr.start_address<<hdr.data_size;
    node->setProgress(dataAddr * 100 / dataSize);
    return true;
} //=============================================================================
void ProtocolServiceFile::readReply(QByteArray data)
{
    //qDebug()<<node->info.name<<data.size();
    if (data.size() <= static_cast<int>(sizeof(_flash_data_hdr))) {
        qDebug() << "Wrong reply size" << data.size();
        return;
    }
    _flash_data_hdr hdr;
    memcpy(&hdr, data.data(), sizeof(_flash_data_hdr));
    if (hdr.start_address != dataAddr) {
        qDebug() << "Wrong addr" << hdr.start_address << dataAddr;
        return;
    }
    uint sz = static_cast<uint>(data.size()) - sizeof(_flash_data_hdr);
    if (sz != hdr.data_size) {
        qDebug() << "Wrong data size" << hdr.data_size << sz;
        return;
    }
    rdata.append(data.mid(sizeof(_flash_data_hdr)));
    dataAddr += hdr.data_size;
    if (request_download())
        return;
    //downloaded, check consistency
    quint8 xor_crc = 0;
    for (int i = 0; i < rdata.size(); i++)
        xor_crc ^= static_cast<quint8>(rdata.at(i));
    if (xor_crc != dataCRC) {
        qDebug() << "Wrong script_block CRC";
        rdata.clear();
        return;
    }
    //qDebug()<<"done";
    emit fileReceived(rdata);
}
//=============================================================================
void ProtocolServiceFile::writeReply(QByteArray data)
{
    //qDebug()<<node->info.name<<data.size();
    _flash_data_hdr hdr;
    memcpy(&hdr, data.data(), sizeof(_flash_data_hdr));
    if (hdr.start_address != dataAddr) {
        qDebug() << "Wrong addr" << hdr.start_address << dataAddr;
        return;
    }
    dataAddr += hdr.data_size;
    if (request_upload())
        return;
    //qDebug()<<"done";
    emit fileUploaded();
}
//=============================================================================
//=============================================================================
void ProtocolServiceFile::serviceData(quint16 cmd, QByteArray data)
{
    if (cmd == cmd_file) {
        if (reqFileRead) {
            if (data.size() != sizeof(_flash_file))
                return;
            ProtocolServiceRequest *r = reqFileRead;
            reqFileRead = nullptr;
            fileReadReply(data);
            r->finish();
        } else if (reqFileWrite) {
            if (data.size() != 0)
                return;
            ProtocolServiceRequest *r = reqFileWrite;
            reqFileWrite = nullptr;
            fileWriteReply();
            r->finish();
        }
    } else if (cmd == cmd_read) {
        if (reqRead) {
            if (data.size() <= static_cast<int>(sizeof(_flash_data_hdr)))
                return;
            if (reqRead->data != data.left(reqRead->data.size()))
                return;
            ProtocolServiceRequest *r = reqRead;
            reqRead = nullptr;
            readReply(data);
            r->finish();
        }
    } else if (cmd == cmd_write) {
        if (reqWrite) {
            if (data.size() != sizeof(_flash_data_hdr))
                return;
            if (data != reqWrite->data.left(data.size()))
                return;
            ProtocolServiceRequest *r = reqWrite;
            reqWrite = nullptr;
            writeReply(data);
            r->finish();
        }
    }
}
//=============================================================================
//=============================================================================

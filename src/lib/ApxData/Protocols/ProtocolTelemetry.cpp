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
#include "ProtocolTelemetry.h"
#include "ProtocolVehicle.h"

#include <App/App.h>
#include <Mandala/Mandala.h>

#include <Xbus/telemetry/TelemetryValuePack.h>

ProtocolTelemetry::ProtocolTelemetry(ProtocolVehicle *vehicle)
    : ProtocolBase(vehicle, "telemetry")
    , vehicle(vehicle)
{
    setIcon("sitemap");
    setTitle(tr("Telemetry"));
    setDescr(tr("Downlink stream decoder"));

    connect(this, &Fact::enabledChanged, this, &ProtocolTelemetry::updateStatus);
    connect(this, &Fact::triggered, this, [this]() { decoder.reset(); });
}

void ProtocolTelemetry::updateStatus()
{
    if (!enabled()) {
        setValue(QString("RESYNC %1").arg(decoder.fmt_cnt()));
        return;
    }
    setValue(QString("%1 slots, %2 Hz").arg(decoder.slots_cnt()).arg(decoder.rate(), 0, 'f', 1));
}

void ProtocolTelemetry::downlink(const xbus::pid_s &pid, ProtocolStreamReader &stream)
{
    mandala::uid_t uid = pid.uid;
    if (!mandala::cmd::env::telemetry::match(uid)) {
        trace_downlink(stream.payload());
        // regular mandala value
        if (uid >= mandala::cmd::env::uid)
            return;
        if (stream.available() <= mandala::spec_s::psize())
            return;

        mandala::spec_s spec;
        spec.read(&stream);

        TelemetryValues values = unpack(pid, spec, stream);
        if (values.isEmpty()) {
            qWarning() << "unpack data values error";
            return;
        }
        vehicle->updateStreamType(ProtocolVehicle::DATA);
        emit valuesData(values);
        return;
    }

    // telemetry section uid
    switch (uid) {
    case mandala::cmd::env::telemetry::format::uid:
        trace_downlink(stream.payload());
        if (pid.pri == xbus::pri_response) {
            if (stream.available() < (sizeof(uint8_t) * 2))
                return;
            uint8_t part, parts;
            stream >> part;
            stream >> parts;
            _request_format_part = 0;
            qDebug() << "format:" << part << parts << stream.available();
            if (!decoder.decode_format(part, parts, stream)) {
                qWarning() << decoder.valid() << decoder.slots_cnt();
                return;
            }
            if (decoder.valid())
                return;
            if (parts <= 1)
                return;
            if (++part >= parts) {
                qWarning() << "format done";
                _request_format_part = 0;
                return;
            }
            _request_format_part = part;
            request_format(part);
        }
        return;
    }

    // telemetry data stream
    if (uid != mandala::cmd::env::telemetry::data::uid)
        return;

    if (stream.available() < xbus::telemetry::stream_s::psize()) {
        qWarning() << stream.available();
        return;
    }

    vehicle->updateStreamType(ProtocolVehicle::TELEMETRY);

    trace_downlink(stream.toByteArray(stream.pos(), 2));     // ts
    trace_downlink(stream.toByteArray(stream.pos() + 2, 1)); // hash
    trace_downlink(stream.toByteArray(stream.pos() + 3, 1)); // fmt
    trace_downlink(stream.toByteArray(stream.pos() + 4, stream.available() - 4));

    bool upd = decoder.decode(pid, stream);
    bool valid = decoder.valid();

    // manage timestamp wraps
    uint16_t ts = decoder.timestamp_dms();
    uint16_t dts = ts - _ts_s;
    _ts_s = ts;
    qint64 dts_ms = dts * 100;
    qint64 elapsed = _ts_time.elapsed();
    _ts_time.start();

    if (!_ts_time.isValid())
        _timestamp_ms = 0;
    else if (abs(elapsed - dts_ms) > 1000) {
        _timestamp_ms = 0;
    } else {
        _timestamp_ms += dts_ms;
    }

    //qDebug() << decoder.seq();

    if (enabled() && !valid) {
        qWarning() << "stream error";
    }

    setEnabled(valid);

    if (!valid || _rate_s != decoder.rate()) {
        _rate_s = decoder.rate();
        updateStatus();
    }

    //qDebug() << valid << upd;

    if (!valid) {
        if (!_request_format_time.isValid() || _request_format_time.elapsed() > 500)
            request_format(_request_format_part);
        return;
    }

    if (!upd)
        return;

    // collect updated values
    TelemetryValues values;

    for (size_t i = 0; i < decoder.slots_cnt(); ++i) {
        auto &flags = decoder.dec_slots().flags[i];
        if (!flags.upd)
            continue;
        flags.upd = false;
        auto const &f = decoder.dec_slots().fields[i];
        TelemetryValue t;
        t.pid = f.pid;
        auto const &value = decoder.dec_slots().value[i];
        t.value = raw_value(&value, flags.type);
        values.append(t);
    }

    emit telemetryData(values, _timestamp_ms);
}

void ProtocolTelemetry::request_format(uint8_t part)
{
    qDebug() << part;
    _request_format_time.start();
    ostream.req(mandala::cmd::env::telemetry::format::uid);
    ostream << part;
    vehicle->send(ostream.toByteArray());
}

QVariant ProtocolTelemetry::raw_value(const void *src, mandala::type_id_e type)
{
    switch (type) {
    default:
        return QVariant();
    case mandala::type_real:
        return QVariant::fromValue(xbus::telemetry::raw_value<mandala::real_t>(src, type));
    case mandala::type_dword:
        return QVariant::fromValue(xbus::telemetry::raw_value<mandala::dword_t>(src, type));
    case mandala::type_word:
        return QVariant::fromValue(xbus::telemetry::raw_value<mandala::word_t>(src, type));
    case mandala::type_byte:
        return QVariant::fromValue(xbus::telemetry::raw_value<mandala::byte_t>(src, type));
    case mandala::type_option:
        return QVariant::fromValue(xbus::telemetry::raw_value<mandala::option_t>(src, type));
    }
}

ProtocolTelemetry::TelemetryValues ProtocolTelemetry::unpack(const xbus::pid_s &pid,
                                                             const mandala::spec_s &spec,
                                                             ProtocolStreamReader &stream)
{
    TelemetryValues values;

    if (pid.pri > 0) {
        qWarning() << "pri:" << pid.pri << Mandala::meta(pid.uid).path;
    }

    if (spec.type >= mandala::type_bundle) {
        int vcnt = 0;
        switch (spec.type) {
        default:
            break;
        case mandala::type_vec2:
            vcnt = 2;
            break;
        case mandala::type_vec3:
            vcnt = 3;
            break;
        }
        mandala::spec_s vspec{};
        xbus::pid_s vpid{pid};
        for (int i = 0; i < vcnt; ++i) {
            vspec.type = Mandala::meta(vpid.uid).type_id;
            TelemetryValues vlist = unpack(vpid, vspec, stream);
            if (vlist.isEmpty())
                break;
            values.append(vlist);
            vpid.uid++;
        }
        if (values.size() != vcnt)
            values.clear();
        return values;
    }

    TelemetryValue value;
    value.pid = pid;
    //value.value=mandala::read<QVariant>(spec.type,stream);

    //qDebug() << Mandala::meta(pid.uid).name << stream->dump_payload();
    switch (spec.type) {
    default:
        break;
    case mandala::type_real:
        value.value = unpack<mandala::real_t>(stream);
        values.append(value);
        break;
    case mandala::type_dword:
        value.value = unpack<mandala::dword_t>(stream);
        values.append(value);
        break;
    case mandala::type_word:
        value.value = unpack<mandala::word_t>(stream);
        values.append(value);
        break;
    case mandala::type_byte:
        value.value = unpack<mandala::byte_t>(stream);
        values.append(value);
        break;
    case mandala::type_option:
        value.value = unpack<mandala::option_t>(stream);
        values.append(value);
        break;
    }
    if (value.value.isNull()) {
        values.clear();
        // error
        qDebug() << "error: " << Mandala::meta(pid.uid).path << stream.available()
                 << stream.dump_payload();
    }

    return values;
}

void ProtocolTelemetry::pack(const QVariant &v,
                             mandala::type_id_e type,
                             ProtocolStreamWriter &stream)
{
    switch (type) {
    default:
        return;
    case mandala::type_real:
        stream.write<mandala::real_t>(v.value<mandala::real_t>());
        return;
    case mandala::type_dword:
        stream.write<mandala::dword_t>(v.value<mandala::dword_t>());
        return;
    case mandala::type_word:
        stream.write<mandala::word_t>(v.value<mandala::word_t>());
        return;
    case mandala::type_byte:
        stream.write<mandala::byte_t>(v.value<mandala::byte_t>());
        return;
    case mandala::type_option:
        stream.write<mandala::option_t>(v.value<mandala::option_t>());
        return;
    }
}

void ProtocolTelemetry::sendValue(ProtocolTelemetry::TelemetryValue value)
{
    ostream.req(value.pid.uid, value.pid.pri);
    mandala::spec_s spec;
    spec.type = Mandala::meta(value.pid.uid).type_id;
    spec.write(&ostream);
    if (!value.value.isNull()) {
        pack(value.value, spec.type, ostream);
    }
    vehicle->send(ostream.toByteArray());
}
void ProtocolTelemetry::sendBundle(ProtocolTelemetry::TelemetryValues values)
{
    if (values.size() <= 1 && values.size() > 3) {
        qWarning() << "wrong size" << values.size();
        return;
    }

    ostream.req(values.first().pid.uid, values.first().pid.pri);
    mandala::spec_s spec;
    if (values.size() == 2)
        spec.type = mandala::type_vec2;
    else if (values.size() == 3)
        spec.type = mandala::type_vec3;

    spec.write(&ostream);

    for (auto const &v : values) {
        pack(v.value, Mandala::meta(v.pid.uid).type_id, ostream);
    }

    vehicle->send(ostream.toByteArray());
}

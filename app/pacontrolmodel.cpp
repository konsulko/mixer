/*
 * Copyright (C) 2016 Konsulko Group
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "pacontrolmodel.h"
#include "pac.h"

PaControl::PaControl(const quint32 &cindex, const QString &desc, const quint32 &type, const quint32 &channel, const QString &cdesc, const quint32 &volume)
	: m_cindex(cindex), m_desc(desc), m_type(type), m_channel(channel), m_cdesc(cdesc), m_volume(volume)
{
}

quint32 PaControl::cindex() const
{
	return m_cindex;
}

QString PaControl::desc() const
{
	return m_desc;
}

quint32 PaControl::type() const
{
	return m_type;
}

quint32 PaControl::channel() const
{
	return m_channel;
}

QString PaControl::cdesc() const
{
	return m_cdesc;
}


quint32 PaControl::volume() const
{
	return m_volume;
}

// FIXME: Not all of these should be editable roles
void PaControl::setCIndex(const QVariant &cindex)
{
	m_cindex = cindex.toUInt();
}

void PaControl::setDesc(const QVariant &desc)
{
	m_desc = desc.toString();
}

void PaControl::setType(const QVariant &type)
{
	m_type = type.toUInt();
}

void PaControl::setChannel(const QVariant &channel)
{
	m_channel = channel.toUInt();
}

void PaControl::setCDesc(const QVariant &cdesc)
{
	m_cdesc = cdesc.toString();
}

void PaControl::setVolume(pa_context *pa_ctx, const QVariant &volume)
{
	if (volume != m_volume) {
		m_volume = volume.toUInt();
		pac_set_volume(pa_ctx, type(), cindex(), channel(), m_volume);
	}
}

PaControlModel::PaControlModel(QObject *parent)
	: QAbstractListModel(parent)
{
	pa_ctx = pac_init(this, "Mixer");
}

void PaControlModel::addControl(const PaControl &control)
{
	beginInsertRows(QModelIndex(), rowCount(), rowCount());
	m_controls << control;
	endInsertRows();
}

void add_one_control(void *ctx, int cindex, const char *desc, int type, int channel, const char *cdesc, int volume)
{
	// Get the PaControlModel object from the opaque pointer context
	PaControlModel *pacm = static_cast<PaControlModel*>(ctx);
	pacm->addControl(PaControl(cindex, desc, type, channel, cdesc, volume));
}

int PaControlModel::rowCount(const QModelIndex & parent) const {
	Q_UNUSED(parent);
	return m_controls.count();
}

bool PaControlModel::setData(const QModelIndex &index, const QVariant &value, int role) {
	if (index.row() < 0 || index.row() >= m_controls.count())
		return false;
	PaControl &control = m_controls[index.row()];
	if (role == CIndexRole)
		control.setCIndex(value);
	else if (role == DescRole)
		control.setDesc(value);
	else if (role == TypeRole)
		control.setType(value);
	else if (role == ChannelRole)
		control.setChannel(value);
	else if (role == CDescRole)
		control.setCDesc(value);
	else if (role == VolumeRole)
		control.setVolume(pa_ctx, value);
	emit dataChanged(index, index);
	return true;
}

QVariant PaControlModel::data(const QModelIndex & index, int role) const {
	if (index.row() < 0 || index.row() >= m_controls.count())
		return QVariant();

	const PaControl &control = m_controls[index.row()];
	if (role == CIndexRole)
		return control.cindex();
	else if (role == DescRole)
		return control.desc();
	else if (role == TypeRole)
		return control.type();
	else if (role == ChannelRole)
		return control.channel();
	else if (role == CDescRole)
		return control.cdesc();
	else if (role == VolumeRole)
		return control.volume();
	return QVariant();
}

Qt::ItemFlags PaControlModel::flags(const QModelIndex &index) const
{
	if (!index.isValid())
		return Qt::ItemIsEnabled;

	return QAbstractListModel::flags(index) | Qt::ItemIsEditable;
}

QHash<int, QByteArray> PaControlModel::roleNames() const {
	QHash<int, QByteArray> roles;
	roles[CIndexRole] = "cindex";
	roles[DescRole] = "desc";
	roles[TypeRole] = "type";
	roles[ChannelRole] = "channel";
	roles[CDescRole] = "cdesc";
	roles[VolumeRole] = "volume";
	return roles;
}

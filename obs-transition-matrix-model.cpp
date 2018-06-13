/*
Copyright (C) 2018 by Fabio Madia <admshao@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "obs-transition-matrix-model.hpp"

TransitionMatrixModel::TransitionMatrixModel(QObject *parent)
		:QStandardItemModel(parent)
{
	obs_frontend_get_scenes(&scenes);

	for (size_t row = 0; row < scenes.sources.num + 1; row++) {
		for (size_t col = 0; col < scenes.sources.num; col++) {
			QStandardItem *item = new QStandardItem();
			item->setEditable(false);
			setItem((int)row, (int)col, item);
		}
	}

	tableView = dynamic_cast<QTableView *>(parent);

	QHeaderView *verticalHeader = tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(50);

	QHeaderView *horizontalHeader = tableView->horizontalHeader();
	horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader->setDefaultSectionSize(150);
}

int TransitionMatrixModel::rowCount(const QModelIndex &) const
{
	return static_cast<int>(scenes.sources.num + 1);
}

int TransitionMatrixModel::columnCount(const QModelIndex &) const
{
	return static_cast<int>(scenes.sources.num);
}

QVariant TransitionMatrixModel::headerData(int section,
		Qt::Orientation orientation, int role) const
{
	if (role == Qt::DisplayRole) {
		QString name;
		if (orientation == Qt::Horizontal) {
			string scene = obs_source_get_name(scenes.sources.array
					[section]);
			if (scene.size() > 10) {
				scene = scene.substr(0, 10);
				name = scene.append("..").c_str();
			} else {
				name = scene.c_str();
			}
			return name;
		} else {
			name = obs_module_text("OBSTransitionMatrix.Any");
			if (section > 0) {
				string scene = obs_source_get_name(scenes
						.sources.array[section - 1]);
				if (scene.size() > 15) {
					scene = scene.substr(0, 15);
					name = scene.append("..").c_str();
				} else {
					name = scene.c_str();
				}
			}
			return name;
		}
	}

	if (role == Qt::TextAlignmentRole)
		return Qt::AlignCenter;

	return QVariant();
}

QVariant createDisplayString(struct transition_matrix &tm)
{
	if (tm.transition == NONE)
		return obs_module_text("OBSTransitionMatrix.None");

	QString ret;
	if (tm.transition.length() > 15) {
		ret = tm.transition.substr(0, 15).c_str();
		ret.append(".. ");
	} else {
		ret = tm.transition.c_str();
	}
	ret.append(", ");
	ret.append(QString::number(tm.duration));
	ret.append("ms");
	return ret;
}

QVariant TransitionMatrixModel::data(const QModelIndex &index, int role) const
{
	if (role == Qt::TextAlignmentRole)
		return Qt::AlignCenter;

	if (!index.isValid() || role != Qt::DisplayRole
			|| index.row() == index.column() + 1)
		return QVariant();

	if (index.row() > 0) {
		obs_source_t *fromSrc = scenes.sources.array[index.row() - 1];
		string fromName = obs_source_get_name(fromSrc);

		auto sm_it = scene_matrix.find(fromName);
		if (sm_it != scene_matrix.end()) {
			obs_source_t *toSrc = scenes.sources.array[index
					.column()];
			string toName = obs_source_get_name(toSrc);

			auto data = sm_it->second.data;
			auto it = data.find(toName);
			if (it != data.end())
				return createDisplayString(it->second);
		}

		return obs_module_text("OBSTransitionMatrix.None");
	}

	obs_source_t *source = scenes.sources.array[index.column()];
	string name = obs_source_get_name(source);
	return createDisplayString(scene_matrix[ANY].data[name]);
}

TransitionMatrixModel::~TransitionMatrixModel()
{
	obs_frontend_source_list_free(&scenes);
}

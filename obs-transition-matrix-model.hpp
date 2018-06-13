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

#pragma once

#include <QStandardItemModel>
#include <QTableView>
#include <QSpinBox>
#include <QMenu>
#include <QWidgetAction>
#include <QPushButton>
#include <QHeaderView>

#include "obs-transition-matrix.hpp"

class TransitionMatrixModel : public QStandardItemModel
{
	Q_OBJECT
public:
	TransitionMatrixModel(QObject *parent = nullptr);
	~TransitionMatrixModel();
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	int columnCount(const QModelIndex &parent = QModelIndex()) const
			override;
	QVariant headerData(int section, Qt::Orientation orientation, int role)
			const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)
			const override;

public slots:
	void customMenuRequested(const QPoint &pos);

private:
	QTableView *tableView = nullptr;
	struct obs_frontend_source_list scenes = {};
	struct obs_frontend_source_list transitions = {};
};

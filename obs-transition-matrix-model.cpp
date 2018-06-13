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
	obs_frontend_get_transitions(&transitions);

	for (size_t row = 0; row < scenes.sources.num + 1; row++) {
		for (size_t col = 0; col < scenes.sources.num; col++) {
			QStandardItem *item = new QStandardItem();
			item->setEditable(false);
			setItem((int)row, (int)col, item);
		}
	}

	tableView = dynamic_cast<QTableView *>(parent);
	tableView->setContextMenuPolicy(Qt::CustomContextMenu);

	connect(tableView, SIGNAL(customContextMenuRequested(QPoint)),
			SLOT(customMenuRequested(QPoint)));

	QHeaderView *verticalHeader = tableView->verticalHeader();
	verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
	verticalHeader->setDefaultSectionSize(50);

	QHeaderView *horizontalHeader = tableView->horizontalHeader();
	horizontalHeader->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader->setDefaultSectionSize(150);
}

void TransitionMatrixModel::customMenuRequested(const QPoint &pos)
{
	QModelIndex index = tableView->indexAt(pos);
	if (index.row() == -1 || index.column() == -1
			|| index.row() == index.column() + 1)
		return;

	obs_source_t *from = nullptr;
	if (index.row() > 0)
		from = scenes.sources.array[index.row() - 1];

	obs_source_t *to = scenes.sources.array[index.column()];

	string fromName = ANY;
	if (from)
		fromName = obs_source_get_name(from);

	string toName = obs_source_get_name(to);

	QMenu *menu = new QMenu(tableView);

	QSpinBox *duration = new QSpinBox(menu);
	duration->setMinimum(50);
	duration->setSuffix("ms");
	duration->setMaximum(20000);
	duration->setSingleStep(50);
	duration->setValue(DEFAULT);

	auto sm_it = scene_matrix.find(fromName);
	if (sm_it != scene_matrix.end()) {
		auto data = sm_it->second.data;
		auto it = data.find(toName);
		if (it != data.end())
			duration->setValue(it->second.duration);
	}

	auto setTransition = [=] (QAction *action, QSpinBox *duration,
			QPushButton *pushBtn)
	{
		string transition = action->property("transition").toString()
				.toStdString();

		pushBtn->setText(transition.c_str());

		if (transition == NONE) {
			if (fromName == ANY) {
				scene_matrix[fromName].data[toName].to = toName;
				scene_matrix[fromName].data[toName].transition =
						transition;
				scene_matrix[fromName].data[toName].duration =
						DEFAULT;
				duration->setValue(DEFAULT);
			} else {
				scene_matrix[fromName].data.erase(toName);
				if (scene_matrix[fromName].data.empty())
					scene_matrix.erase(fromName);
			}
		} else {
			scene_matrix[fromName].data[toName].to = toName;
			scene_matrix[fromName].data[toName].transition =
					transition;
			scene_matrix[fromName].data[toName].duration =
					duration->value();
		}

		blog(LOG_INFO, "Transition from '%s' to '%s' selected: %s %d",
				fromName.c_str(), toName.c_str(), transition
				.c_str(), duration->value());

		tableView->update();
	};

	auto setDuration = [=] (int duration)
	{
		auto smit = scene_matrix.find(fromName);
		if (smit != scene_matrix.end()) {
			auto data = smit->second.data;
			auto it = data.find(toName);
			if (it != data.end()) {
				scene_matrix[fromName].data[toName].duration =
						duration;

				blog(LOG_INFO, "Updated duration from '%s' to "
						"'%s': %d", fromName.c_str(),
						toName.c_str(), duration);

				tableView->update();
			}
		}
	};

	connect(duration, QOverload<int>::of(&QSpinBox::valueChanged),
			setDuration);

	QPushButton *pushBtn = new QPushButton(menu);
	QWidgetAction *pushBtnAction = new QWidgetAction(menu);
	pushBtnAction->setDefaultWidget(pushBtn);
	menu->addAction(pushBtnAction);

	QMenu *transitionMenu = new QMenu;
	pushBtn->setMenu(transitionMenu);
	string currentName;
	string largestName;

	for (size_t i = 0; i < transitions.sources.num + 1; i++) {
		string trName;

		if (i > 0)
			trName = obs_source_get_name(transitions.sources
					.array[i - 1]);
		else
			trName = NONE;

		if (trName.size() > largestName.size())
			largestName = trName;

		QAction *action = transitionMenu->addAction(trName.c_str());
		action->setProperty("transition", trName.c_str());

		bool match;
		auto smit = scene_matrix.find(fromName);
		if (smit != scene_matrix.end()) {
			auto data = smit->second.data;
			auto it = data.find(toName);
			if (it != data.end()) {
				match = it->second.transition == trName;
				goto end;
			}
		}

		match = trName == NONE;

end:
		if (match)
			currentName = trName;

		connect(action, &QAction::triggered, std::bind(setTransition,
				action, duration, pushBtn));
	}

	QWidgetAction *durationAction = new QWidgetAction(menu);
	durationAction->setDefaultWidget(duration);

	menu->addSeparator();
	menu->addAction(durationAction);

	pushBtn->setText(largestName.c_str());

	menu->popup(tableView->viewport()->mapToGlobal(pos));

	pushBtn->setText(currentName.c_str());
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
	obs_frontend_source_list_free(&transitions);

	update_scenes_transition_override();
}

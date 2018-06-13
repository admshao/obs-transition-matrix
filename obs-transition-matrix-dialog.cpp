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

#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableView>

#include "obs-transition-matrix-model.hpp"
#include "obs-transition-matrix-dialog.hpp"

TransitionMatrixDialog::TransitionMatrixDialog(QMainWindow *parent)
		: QDialog(parent)
{
	QVBoxLayout *mainLayout = new QVBoxLayout();

	/* --------------------------------------------- */

	QTableView *tableView = new QTableView(this);
	tableView->setModel(new TransitionMatrixModel(tableView));

	tableView->setFocusPolicy(Qt::NoFocus);
	tableView->setSelectionMode(QAbstractItemView::NoSelection);
	tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

	/* --------------------------------------------- */

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidget(tableView);
	scrollArea->setWidgetResizable(true);

	/* --------------------------------------------- */

	QPushButton *closeButton = new QPushButton(
			obs_module_text("OBSTransitionMatrix.Close"));
	QLabel *descriptionLabel = new QLabel(
			obs_module_text("OBSTransitionMatrix.Description"));

	QHBoxLayout *bottomLayout = new QHBoxLayout;
	bottomLayout->addWidget(descriptionLabel);
	bottomLayout->addWidget(closeButton);
	bottomLayout->setAlignment(descriptionLabel, Qt::AlignLeft);
	bottomLayout->setAlignment(closeButton, Qt::AlignRight);

	connect(closeButton, &QPushButton::clicked, [this] () {close();});

	/* --------------------------------------------- */

	mainLayout->addWidget(scrollArea);
	mainLayout->addLayout(bottomLayout);

	/* --------------------------------------------- */

	setLayout(mainLayout);

	setWindowTitle(obs_module_text("OBSTransitionMatrix"));

	setMinimumSize(800, 600);
}

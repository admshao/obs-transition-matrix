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

#include "obs-transition-matrix.hpp"
#include "obs-transition-matrix-dialog.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(MODULE_NAME, "en-US")

static void handle_obs_frontend_save_load(obs_data_t *, bool, void *)
{
}

static void handle_obs_frontend_event(enum obs_frontend_event, void *)
{
}

bool obs_module_load(void)
{
	QAction *action = (QAction*)obs_frontend_add_tools_menu_qaction(
			obs_module_text("OBSTransitionMatrix"));

	auto cb = [] {
		obs_frontend_push_ui_translation(obs_module_get_string);
		TransitionMatrixDialog tmd((QMainWindow *)
				obs_frontend_get_main_window());
		tmd.exec();

		obs_frontend_pop_ui_translation();
	};

	QAction::connect(action, &QAction::triggered, cb);

	obs_frontend_add_save_callback(handle_obs_frontend_save_load, nullptr);
	obs_frontend_add_event_callback(handle_obs_frontend_event, nullptr);

	return true;
}

void obs_module_unload(void)
{
}

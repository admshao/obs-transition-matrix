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

map<string, scene_data> scene_matrix;

static void clear_matrix_data()
{
	scene_matrix.clear();
}

static void dump_saved_matrix()
{
	blog(LOG_INFO, "Scene count: %lu", scene_matrix.size());

	for (auto sm_it : scene_matrix) {
		blog(LOG_INFO, "\t'From' Scene: %s", sm_it.first.c_str());
		blog(LOG_INFO, "\tOverride count: %lu", sm_it.second.data
				.size());

		for (auto tm_it : sm_it.second.data) {
			blog(LOG_INFO, "\t\t'To' Scene: '%s'", tm_it.second.to
					.c_str());
			blog(LOG_INFO, "\t\t\t\tTransition: %s", tm_it.second
					.transition.c_str());
			blog(LOG_INFO, "\t\t\t\tDuration: %d ms", tm_it.second
					.duration);
		}
	}
}

static void create_transition_matrix(map<string, transition_matrix> &tm,
		obs_data_t *data)
{
	string to = obs_data_get_string(data, "to");
	tm[to].to = to;
	tm[to].transition = obs_data_get_string(data, "transition");
	tm[to].duration = (int)obs_data_get_int(data, "duration");
}

static void create_scene_data(obs_data_t *scene)
{
	obs_data_array_t *data = obs_data_get_array(scene, "data");
	string sceneName = obs_data_get_string(scene, "scene");

	scene_matrix[sceneName].scene = sceneName;

	size_t transition_count = obs_data_array_count(data);
	size_t i;
	for (i = 0; i < transition_count; i++) {
		obs_data_t *transition = obs_data_array_item(data, i);
		create_transition_matrix(scene_matrix[sceneName].data,
				transition);

		obs_data_release(transition);
	}

	obs_data_array_release(data);
}

static void load_scenes(obs_data_t *matrix)
{
	obs_data_array_t *scenes = obs_data_get_array(matrix, "matrix");

	size_t scene_count = obs_data_array_count(scenes);
	size_t i;
	for (i = 0; i < scene_count; i++) {
		obs_data_t *scene = obs_data_array_item(scenes, i);
		create_scene_data(scene);
		obs_data_release(scene);
	}

	obs_data_array_release(scenes);
}

static void load_saved_matrix(obs_data_t *save_data)
{
	obs_data_t *obj = obs_data_get_obj(save_data, MODULE_NAME);
	if (!obj)
		return;

	load_scenes(obj);

	obs_data_release(obj);
}

static void save_transition_data(map<string, transition_matrix> &sd,
		obs_data_array_t *scene)
{
	for (auto td_it : sd) {
		obs_data_t *transition = obs_data_create();

		obs_data_set_string(transition, "to", td_it.second.to.c_str());
		obs_data_set_string(transition, "transition", td_it.second
				.transition.c_str());
		obs_data_set_int(transition, "duration", td_it.second.duration);

		obs_data_array_push_back(scene, transition);

		obs_data_release(transition);
	}
}

static void save_scenes_data(obs_data_array_t *scenes)
{
	for (auto sm_it : scene_matrix) {
		if (sm_it.first == ANY)
			continue;

		obs_data_t *scene = obs_data_create();
		obs_data_array_t *data = obs_data_array_create();

		save_transition_data(sm_it.second.data, data);
		obs_data_set_string(scene, "scene", sm_it.first.c_str());
		obs_data_set_array(scene, "data", data);

		obs_data_array_push_back(scenes, scene);

		obs_data_array_release(data);
		obs_data_release(scene);
	}
}

static void save_scenes(obs_data_t *matrix)
{
	obs_data_array_t *scenes = obs_data_array_create();

	save_scenes_data(scenes);
	obs_data_set_array(matrix, "matrix", scenes);

	obs_data_array_release(scenes);
}

static void save_matrix_data(obs_data_t *save_data)
{
	if (scene_matrix.size() < 2)
		return;

	obs_data_t *obj = obs_data_create();

	save_scenes(obj);
	obs_data_set_obj(save_data, MODULE_NAME, obj);

	obs_data_release(obj);
}

static void handle_obs_frontend_save_load(obs_data_t *save_data, bool saving,
		void *)
{
	if (saving) {
		save_matrix_data(save_data);
	} else {
		clear_matrix_data();
		load_saved_matrix(save_data);
		dump_saved_matrix();
	}
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

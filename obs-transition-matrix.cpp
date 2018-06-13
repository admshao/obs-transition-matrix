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

#include <set>

#include "obs-transition-matrix.hpp"
#include "obs-transition-matrix-dialog.hpp"

OBS_DECLARE_MODULE()
OBS_MODULE_USE_DEFAULT_LOCALE(MODULE_NAME, "en-US")

map<string, scene_data> scene_matrix;

static set<string> sceneNames;

static void clear_matrix_data()
{
	scene_matrix.clear();
	sceneNames.clear();
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

static void load_default_transition_override()
{
	struct obs_frontend_source_list scenes = {};
	obs_frontend_get_scenes(&scenes);

	scene_matrix[ANY].scene = ANY;

	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *src = scenes.sources.array[i];
		obs_data_t *data = obs_source_get_private_settings(src);

		string transition = obs_data_get_string(data, "transition");
		int duration = (int)obs_data_get_int(data,
				"transition_duration");

		obs_data_release(data);

		string to = obs_source_get_name(src);

		sceneNames.emplace(to);

		scene_matrix[ANY].data[to].to = to;
		if (transition.empty())
			scene_matrix[ANY].data[to].transition = NONE;
		else
			scene_matrix[ANY].data[to].transition = transition;

		scene_matrix[ANY].data[to].duration = duration;
		if (duration == 0)
			scene_matrix[ANY].data[to].duration = DEFAULT;
	}

	obs_frontend_source_list_free(&scenes);
}

static void set_source_transition_override(struct transition_matrix &tm)
{
	obs_source_t *dest = obs_get_source_by_name(tm.to.c_str());
	obs_data_t *data = obs_source_get_private_settings(dest);

	if (tm.transition == NONE) {
		obs_data_erase(data, "transition");
	} else {
		obs_data_set_string(data, "transition", tm.transition.c_str());
		obs_data_set_int(data, "transition_duration", tm.duration);
	}

	obs_data_release(data);
	obs_source_release(dest);
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
	load_default_transition_override();

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

static void save_default_transition_override()
{
	for (auto tm_it : scene_matrix[ANY].data)
		set_source_transition_override(tm_it.second);
}

static void save_matrix_data(obs_data_t *save_data)
{
	save_default_transition_override();

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

void update_scenes_transition_override()
{
	for (auto tm_it : scene_matrix[ANY].data)
		set_source_transition_override(tm_it.second);

	obs_source_t *scene = obs_frontend_get_current_scene();
	string name = obs_source_get_name(scene);

	auto sm_it = scene_matrix.find(name);
	if (sm_it != scene_matrix.end())
		for (auto tm_it : sm_it->second.data)
			set_source_transition_override(tm_it.second);

	obs_source_release(scene);
}

static void handle_scene_list_changed()
{
	struct obs_frontend_source_list scenes = {};
	obs_frontend_get_scenes(&scenes);

	set<string> newSceneNames;
	for (size_t i = 0; i < scenes.sources.num; i++) {
		obs_source_t *src = scenes.sources.array[i];
		newSceneNames.emplace(obs_source_get_name(src));
	}

	obs_frontend_source_list_free(&scenes);

	if (newSceneNames.size() > sceneNames.size()) {
		set<string> tempSet = newSceneNames;
		for (const string &s : sceneNames)
			tempSet.erase(s);

		sceneNames = newSceneNames;
		string newName = *tempSet.begin();

		scene_matrix[ANY].data[newName].to = newName;
		scene_matrix[ANY].data[newName].transition = NONE;
		scene_matrix[ANY].data[newName].duration = DEFAULT;

		return;
	} else if (newSceneNames.size() == sceneNames.size()) {
		string name;
		string removed;
		for (const string &s : newSceneNames)
			if (!sceneNames.erase(s))
				name = s;
		removed = *sceneNames.begin();

		for (map<string, scene_data>::iterator sm = scene_matrix
				.begin(); sm != scene_matrix.end();) {
			if (sm->first == removed) {
				scene_matrix[name].scene = name;

				for (auto tm_it : sm->second.data) {
					scene_matrix[name].data[tm_it.second.to]
							.to = tm_it.second.to;
					scene_matrix[name].data[tm_it.second.to]
							.duration = tm_it.second
							.duration;
					scene_matrix[name].data[tm_it.second.to]
							.transition = tm_it
							.second.transition;
				}

				scene_matrix.erase(sm++);
				continue;
			}

			for (map<string, transition_matrix>::iterator tm = sm
					->second.data.begin(); tm != sm->second
					.data.end();) {
				if (tm->first == removed) {
					sm->second.data[name].to = name;
					sm->second.data[name].transition = tm
							->second.transition;
					sm->second.data[name].duration = tm
							->second.duration;

					sm->second.data.erase(tm++);
					continue;
				}
				tm++;
			}

			if (sm->second.data.empty())
				scene_matrix.erase(sm++);
			else
				sm++;
		}

		sceneNames = newSceneNames;

		update_scenes_transition_override();
		return;
	}

	set<string> tempSet = sceneNames;
	for (const string &s : newSceneNames)
		tempSet.erase(s);

	string removedScene = *tempSet.begin();
	sceneNames = newSceneNames;

	for (map<string, scene_data>::iterator sm = scene_matrix.begin();
			sm != scene_matrix.end();) {
		if (sm->first == removedScene) {
			scene_matrix.erase(sm++);
			continue;
		}

		for (map<string, transition_matrix>::iterator tm = sm->second
				.data.begin(); tm != sm->second.data.end();) {
			if (tm->first == removedScene)
				sm->second.data.erase(tm++);
			else
				tm++;
		}

		if (sm->second.data.empty())
			scene_matrix.erase(sm++);
		else
			sm++;
	}
}

static void handle_obs_frontend_event(enum obs_frontend_event event, void *)
{
	switch (event) {
	case OBS_FRONTEND_EVENT_SCENE_LIST_CHANGED:
		handle_scene_list_changed();
		break;
	case OBS_FRONTEND_EVENT_SCENE_CHANGED:
		update_scenes_transition_override();
		break;
	default:;
	}
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

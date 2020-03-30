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

#include <cstring>
#include <map>

#include <string>
#include <obs-module.h>
#include <obs-frontend-api.h>
#include <util/platform.h>

#define MODULE_NAME "obs-transition-matrix"

#define NONE "None"
#define ANY "Any"
#define DEFAULT 300

#define blog(level, msg, ...) \
	blog(level, "transition-matrix: " msg, ##__VA_ARGS__)

using namespace std;

struct transition_matrix {
	string to;
	string transition;
	int duration;
};

struct scene_data {
	string scene;
	map<string, transition_matrix> data;
};

extern map<string, scene_data> scene_matrix;

extern void update_scenes_transition_override();

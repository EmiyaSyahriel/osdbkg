////////////////////////
/// CRYSTAL.CPP - Crystals Object Renderer
/// Created at 15/05/2022 05:53 AM by EmiyaSyahriel
////////////////////////
#include "OSDBKG.HPP"
#include <time.h>
#include <glm/trigonometric.hpp>

struct _crystal_transforms_t {
	glm::mat4 matrix;
	bool shine;
};

struct _crystal_data_t {
	float* vdata;
	int* idata;
	size_t idata_sz;
	size_t fdata_sz;
	int model_i, draw_i;
	time_t time;
	tm* local_time;
	glm::vec4 v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
	_crystal_transforms_t transforms[12];
	glm::mat4 center = glm::mat4(1.0f);
} _crystal_data;

void osd_crystal_init() {
	_crystal_data.model_i = osd_oglx_model_index(&states.model, "clock_crystal");
	mdl_object_t* mdl = &states.model.objects[_crystal_data.model_i]; // Do not copy
	size_t face_count = mdl->faces.size();
	_crystal_data.vdata = new float[face_count * 8];
	_crystal_data.idata = new int[face_count * 3];
	_crystal_data.idata_sz = face_count * 3;

	int d = 0;
	std::vector<float> float_data;
	for (int i = 0; i < mdl->faces.size(); i++) {
		mdl_face_t face = mdl->faces[i];
		for (int j = 0; j < 3; j++) {
			mdl_face_idx_t tri = face[j];
			glm::vec3 pos = states.model.positions[tri.v];
			glm::vec3 nrm = states.model.normal[tri.vn];
			glm::vec2 uvs = states.model.texcoord[tri.vt];
			float_data.push_back(pos.x);
			float_data.push_back(pos.y);
			float_data.push_back(pos.z);
			float_data.push_back(nrm.x);
			float_data.push_back(nrm.y);
			float_data.push_back(nrm.z);
			//float_data.push_back(uvs.x);
			//float_data.push_back(uvs.y);
			float_data.push_back(0.0f);
			float_data.push_back(0.0f);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_crystal);
	glBufferData(GL_ARRAY_BUFFER, float_data.size() * sizeof(float), float_data.data(), GL_STATIC_DRAW);
	_crystal_data.fdata_sz = float_data.size();
	osd_vertattrib_main();
}

// returns isFore
bool osd_crystal_predraw() {
	glUniform1f(states.shader_data.main._IOR, 1.0f);
	glUniform4f(states.shader_data.main._ColorFog, 0.0f, 0.6f, 0.6f, 0.125f);
	glUniform1f(states.shader_data.main._Refractive, 1.0f);
	glUniform4f(states.shader_data.main._Color, 0.0f, 0.1f, 0.5f, 0.5f);
	glUniform1f(states.shader_data.main._AlphaBlend, 0.0f);
	return true;
}

void osd_crystal_draw_shapes(glm::mat4 matrix, bool shine) {
	osd_oglx_assign_transform(states.shader_data.main._Transform, matrix);
	if (shine) {
		glUniform4f(states.shader_data.main._ColorAdd, 0.0f, 0.75f, 1.0f, glm::mix(1.25f, 2.0f, sinf((float)states.currentTime + _crystal_data.draw_i)));
	}
	else {
		glUniform4f(states.shader_data.main._ColorAdd, 0.6f, 0.75f, 1.0f, 0.5f);
	}
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)_crystal_data.fdata_sz);
	osd_oglx_swap_refract_buffer();
}

int osd_crystal_depth_sort(const void* a, const void* b) {
	glm::mat4 ma = (*(const _crystal_transforms_t*)a).matrix, mb = (*(const _crystal_transforms_t*)b).matrix;
	glm::vec4 va = ma * _crystal_data.v, vb = mb * _crystal_data.v;
	return (int)((vb.z * 100.0f) - (va.z * 100.0f));
}

void osd_crystal_draw(bool fore) {
	int i;
	if (!fore) { // Do Item Calculations
		time(&_crystal_data.time);
		_crystal_data.local_time = localtime(&_crystal_data.time);
		_crystal_data.draw_i = 0;

		float rad = glm::radians(fmod((float)_crystal_data.local_time->tm_hour, 12.0f) * 30.0f);
		glm::vec2 clock_axis = glm::vec2(
			glm::sin(-rad),
			(float)glm::cos((float)rad)
		);

		osdbkg_transform_action_t transforms[6] = {
			{ TRANSFORM::SCALE_UNIFORM, { 2.0f } },
			{ TRANSFORM::TRANSLATE, {0.0f, 0.0f, 0.0f}},
			{ TRANSFORM::ROTATE, {clock_axis.x, clock_axis.y, 0.0f, (float)states.currentTime * -10.0f}},
			{ TRANSFORM::ROTATE, {0.0f, 0.0f, 1.0f, 0.0f} },
			{ TRANSFORM::TRANSLATE, {0.0f, 2.0f, 0.0f}},
			{ TRANSFORM::ROTATE, {0.0f, 1.0f, 0.0f, 0.0f} }
		};
		for (i = 0; i < 12; i++) {
			transforms[3].data[3] = i * 30.0f;
			transforms[5].data[3] = (float)(states.currentTime + (i * 90.0f));
			glm::mat4 matrix = osd_oglx_set_object_transform(-1,
				6, transforms);
			bool isHourNow = (_crystal_data.local_time->tm_hour % 12) == i;
			int min = _crystal_data.local_time->tm_min;
			int sec = _crystal_data.local_time->tm_sec;
			bool isMinuteNow = min >= i * 5 && min < ((i + 1) * 5);
			bool isSecNow = sec >= i * 5 && sec < ((i + 1) * 5);
			bool isNow = isHourNow || isMinuteNow || isSecNow;

			_crystal_data.transforms[i].matrix = matrix;
			_crystal_data.transforms[i].shine = isNow;
		}
		_crystal_data.center = osd_oglx_set_object_transform(-1, 3, transforms);
		// sort by depth
		std::qsort(
			_crystal_data.transforms, 
			12, 
			sizeof(_crystal_transforms_t), 
			osd_crystal_depth_sort
		);
	}

	glUseProgram(states.shader.main);
	glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_crystal);
	osd_vertattrib_main();
	osd_crystal_predraw();

	glm::vec4 ctr = _crystal_data.center * _crystal_data.v;
	for (i = 0; i < 12; i++) {
		_crystal_transforms_t t = _crystal_data.transforms[i];
		glm::vec4 pos = t.matrix * _crystal_data.v;
		bool atfore = (pos.z - ctr.z) < 0.0f;
		if (atfore == fore) {
			osd_crystal_draw_shapes(t.matrix, t.shine);
			_crystal_data.draw_i++;
		}
	}
}
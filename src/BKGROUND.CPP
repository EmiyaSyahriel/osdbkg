////////////////////////
/// BKGROUND.CPP - Background Cylinder Renderer
/// Created at 15/05/2022 05:53 AM by EmiyaSyahriel
////////////////////////

#include "OSDBKG.HPP"
#include <glm/trigonometric.hpp>

struct _bkgnd_data_t {
	float* vdata;
	int* idata;
	size_t idata_sz;
	int model_i;
} _bkgnd_data ;


void osd_bg_init() {
	_bkgnd_data.model_i = osd_oglx_model_index(&states.model, "bg_cylinder");
	mdl_object_t *mdl = &states.model.objects[_bkgnd_data.model_i]; // Do not copy
	size_t face_count = mdl->faces.size();
	_bkgnd_data.vdata = new float[face_count * 8];
	_bkgnd_data.idata = new int[face_count * 3];
	_bkgnd_data.idata_sz = face_count * 3;
}

void osd_bg_predraw() {
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, states.textures.data.main_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, states.textures.data.active_refract_texture);
	glUniform4f(states.shader_data.main._Color, 0.3f, 0.15f, 0.5f, 1.0f);
	glUniform4f(states.shader_data.main._ColorFog, 0.0f, 0.0f, 0.0f, 1.0f);
	glUniform1i(states.shader_data.main._MainTexture, 0);
	glUniform1i(states.shader_data.main._FramebufferTexture, 1);
	glUniform1f(states.shader_data.main._Refractive, 0.0f);
	glUniform1f(states.shader_data.main._AlphaBlend, 1.0f);
	glUniform4f(states.shader_data.main._ColorAdd, 0.0f, 0.0f, 0.0f, 0.0f);

	osdbkg_transform_action_t transforms[4] = { 
		{ TRANSFORM::ROTATE, {1.0f, 0.0f, 0.0f, 180.0f}},
		{ TRANSFORM::TRANSLATE, {0.0f, 0.0f, 0.0f}},
		{ TRANSFORM::ROTATE, {0.0f, 0.0f, 1.0f, (float)states.currentTime * 10.0f} },
		{ TRANSFORM::SCALE, {5.0f, 5.0f, 10.0f } },
	};

	osd_oglx_set_object_transform(states.shader_data.main._Transform,
		4, transforms);
	//osd_oglx_assign_transform(states.shaders.main._Transform, glm::mat4(1.0f));
	osd_vertattrib_main();
}

void osd_bg_draw() {
	glUseProgram(states.shader.main);
	int d = 0;
	mdl_object_t* mdl = &states.model.objects[_bkgnd_data.model_i]; // Do not copy
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
			float_data.push_back(uvs.x + ((float)states.currentTime * 0.025f));
			float_data.push_back(uvs.y);
		}
	}
	glBindBuffer(GL_ARRAY_BUFFER, states.buffers.data.v_backcyl);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // IDK for now
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * float_data.size(), float_data.data(), GL_DYNAMIC_DRAW);
	osd_bg_predraw();
	glDrawArrays(GL_TRIANGLES, 0, (GLsizei)float_data.size());
	osd_oglx_swap_refract_buffer();
	float_data.clear();
}
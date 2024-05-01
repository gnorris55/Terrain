#ifndef RIGID_MAIN_H
#define RIGID_MAIN_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <RigidBodies/RigidMain.h>
#include <learnopengl/model.h>

class MainRigidBody {

public:
	
	Shader* program;
	glm::vec4 linear_velocity;
	glm::vec4 position;
	glm::vec3 scale;
	glm::vec4 center_of_mass;
	
	glm::mat3 rotation_matrix;
	glm::mat3 inertia_matrix;

	RawModel* model = NULL;

	MainRigidBody(Shader *shader, glm::vec4 starting_pos, glm::vec4 starting_rotation, glm::vec3 scale) {
		this->program = shader;
		this->scale = scale;
		this->position = starting_pos;
		//this->rotation = starting_rotation;

	}

	void load_model() {
		//for loading model
	}

	void integrate_body(float curr_time) {
		//what integration is best for rigid bodies, let not do verlet this time
	}

private:


};


#endif


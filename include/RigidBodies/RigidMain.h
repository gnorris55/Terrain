#ifndef RIGID_MAIN_H
#define RIGID_MAIN_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#include <learnopengl/model.h>

class MainRigidBody {

public:
	
	Shader* program;

	float time_step = 0.01;
	float mass;

	glm::vec4 collision_force = glm::vec4(0, 0, 0, 0);
	glm::vec4 force;
	glm::vec4 torque;

	glm::vec4 position;
	glm::vec3 scale;
	glm::vec4 center_of_mass;



	glm::vec4 linear_velocity = glm::vec4(0,0,0,0);
	glm::vec4 angular_velocity = glm::vec4(0,0,0,0);
	

	glm::mat4 local_coords_matrix;
	glm::mat3 rotation_matrix;
	//glm::quat rotation_quat = glm::quat(1.0f, 0.7f, 0.5f, 0.0f);
	glm::quat rotation_quat = glm::quat(0.0f, 0.0f, 0.0f, 0.0f);
	glm::mat3 inertia_matrix;

	RawModel* model = NULL;

	MainRigidBody(Shader *shader, glm::vec4 starting_pos, glm::vec4 starting_rotation, glm::vec3 scale, float mass) {
		this->program = shader;
		this->scale = scale;
		this->position = starting_pos;
		this->mass = mass;
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


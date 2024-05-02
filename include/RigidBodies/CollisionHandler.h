#ifndef COLLISION_HANDLER
#define COLLISION_HANDLER

#include <RigidBodies/Cube.h>
#include <RigidBodies/Plane.h>

class CollisionHandler {

public:
	
	void add_box(Cube* box) {
		boxes.push_back(box);
	}
	void add_plane(Plane* plane) {
		planes.push_back(plane);
	}


	void handle_collisions() {
		plane_on_cube();
	}



private:

	std::vector<Cube*> boxes;
	std::vector<Plane*> planes;

	void plane_on_cube() {

		for (int i = 0; i < boxes.size(); i++) {
			for (int j = 0; j < planes.size(); j++) {
				//IMPORTANT: a object can collide with two planes at the same time
				handle_plane_on_cube(boxes[i], planes[j]);
			}
		}

		//solve collision: penalty forces
	}

	void handle_plane_on_cube(Cube* cube, Plane* plane) {
	
		for (int i = 0; i < cube->box_mesh.size(); i++) {
			std::cout << glm::to_string(cube->box_mesh[i] * cube->local_coords_matrix) << "\n";
			glm::vec4 difference = cube->box_mesh[i] * cube->local_coords_matrix - plane->position;
			float height_from_plane = glm::dot(difference.xyz(), plane->normal.xyz());
			if (height_from_plane < 0) {
				plane_cube_collision_response(cube->box_mesh[i] * cube->local_coords_matrix, cube, plane);
				//return;
			}
		}
	}

	void plane_cube_collision_response(glm::vec4 point, Cube* cube, Plane* plane) {
		glm::vec4 point_velocity = cube->linear_velocity + glm::vec4(glm::cross(cube->angular_velocity.xyz(), (point - cube->center_of_mass).xyz()), 0.0);
		float init_relative_velocity = glm::dot(point_velocity.xyz(), plane->normal.xyz());
		std::cout << "v-: " << init_relative_velocity << "\n";

		if (init_relative_velocity < 0) {
			glm::mat3 inertia_matrix = cube->get_world_inertia_matrix();
			
			glm::vec4 r = point - cube->center_of_mass;
			float cr = 0.25f;

			float j = (-(1 + cr) * init_relative_velocity) / ((1/cube->mass) + glm::dot(plane->normal.xyz(), (inertia_matrix * glm::cross(glm::cross(r.xyz(), plane->normal.xyz()), r.xyz()))));

			glm::vec4 delta_v = (1/cube->mass)*j * plane->normal;
			glm::vec4 delta_omega = glm::vec4(j * inertia_matrix * glm::cross(r.xyz(), plane->normal.xyz()), 0.0);

			//std::cout << "curr velocity: " << glm::to_string(cube->linear_velocity) << "\n";
			//std::cout << "delta v: " << glm::to_string(delta_v) << "\n";
			//std::cout << "delta omega: " << glm::to_string(delta_omega) << "\n";

			cube->linear_velocity = cube->linear_velocity + delta_v;
			cube->angular_velocity = cube->angular_velocity + delta_omega;

			// need to handle resting tolerance LCP (Linear complementary problem)
		}
		float relative_acc  = glm::dot(plane->normal.xyz(), cube->force.xyz() / cube->mass);
		std::cout << "relative accelerations: " << glm::dot(plane->normal.xyz(), cube->force.xyz()/cube->mass) << "\n";
		if (relative_acc <= 0 && init_relative_velocity > -0.2 &&init_relative_velocity < 0.2) {
			glm::vec4 resting_force = plane->normal * -cube->mass * relative_acc;
			cube->collision_force = cube->collision_force + resting_force;
		}
	}

		

};

#endif 
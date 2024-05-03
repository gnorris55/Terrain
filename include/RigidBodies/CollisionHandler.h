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
		cube_on_cube();
	}



private:

	std::vector<Cube*> boxes;
	std::vector<Plane*> planes;

	void cube_on_cube() {
		for (int i = 0; i < boxes.size() - 1; i++) {
			for (int j = i + 1; j < boxes.size(); j++) {
				handle_cube_on_cube(boxes[i], boxes[j]);
			}

		}
	}

	void handle_cube_on_cube(Cube* boxA, Cube* boxB) {
		//two types of collisions:
		// vertex on face edge on edge
		glm::vec3 minA = boxA->get_mins();
		glm::vec3 minB = boxB->get_mins();
		glm::vec3 maxA = boxA->get_maxes();
		glm::vec3 maxB = boxB->get_maxes();

		//std::cout << "minA: " << glm::to_string(minA) << "\n";
		//std::cout << "minB: " << glm::to_string(minB) << "\n";
		//std::cout << "maxA: " << glm::to_string(maxA) << "\n";
		//std::cout << "maxB: " << glm::to_string(maxB) << "\n";
		

		//TODO:
		if ((minB.x > maxA.x) || (minA.x > maxB.x)
			|| (minB.y > maxA.y) || (minA.y > maxB.y)
			|| (minB.z > maxA.z) || (minA.z > maxB.z))
			return;
		else {
			std::cout << "box collision!\n";
			for (int i = 0; i < boxA->box_mesh.size(); i++) {
				glm::vec4 world_point = boxA->box_mesh[i] * boxA->local_coords_matrix;
				for (int j = 0; j < boxB->faces.size(); j++) {
					FacePlane curr_face = boxB->faces[j];
					glm::vec4 difference = world_point - boxB->box_mesh[curr_face.points[0]] * boxB->local_coords_matrix;
					float plane_dot = glm::dot(difference.xyz(), curr_face.normal.xyz()*boxB->rotation_matrix);
					if (plane_dot < 0)
						in_polygon(world_point, curr_face, boxA, boxB, plane_dot);
				}
			}
		}
	}

	bool in_polygon(glm::vec4 point, FacePlane face, Cube *boxA, Cube *boxB, float plane_dot) {
		//std::cout << "dot of plane: " << plane_dot << "\n";
		//std::cout << "box A linear velocity: " << glm::to_string(boxA->linear_velocity) << "\n";
		
		float hit = plane_dot / (glm::dot(boxA->linear_velocity.xyz(), face.normal.xyz()*boxB->rotation_matrix));
		//std::cout << "hit scalar: " << hit << "\n";
		glm::vec4 hit_point = point - hit * boxA->linear_velocity;
		//std::cout << "our hit point: " << glm::to_string(hit_point) << "\n";
		//std::cout << "p1: " << glm::to_string(boxB->box_mesh[face.points[0]]*boxB->local_coords_matrix) << "\n";
		//std::cout << "p2: " << glm::to_string(boxB->box_mesh[face.points[1]]*boxB->local_coords_matrix) << "\n";
		//std::cout << "p3: " << glm::to_string(boxB->box_mesh[face.points[2]]*boxB->local_coords_matrix) << "\n";
		//std::cout << "p4: " << glm::to_string(boxB->box_mesh[face.points[3]]*boxB->local_coords_matrix) << "\n";
		return false;
	}


	void cube_collision_response(Cube* boxA, Cube* boxB) {
	}

	void plane_on_cube() {

		for (int i = 0; i < boxes.size(); i++) {
			for (int j = 0; j < planes.size(); j++) {
				//IMPORTANT: a object can collide with two planes at the same time
				handle_plane_on_cube(boxes[i], planes[j]);
			}
		}

	}

	void handle_plane_on_cube(Cube* cube, Plane* plane) {
	
		for (int i = 0; i < cube->box_mesh.size(); i++) {
			glm::vec4 difference = cube->box_mesh[i] * cube->local_coords_matrix - plane->position;
			float height_from_plane = glm::dot(difference.xyz(), plane->normal.xyz());
			if (height_from_plane < 0) {
				plane_cube_collision_response(cube->box_mesh[i] * cube->local_coords_matrix, cube, plane);
			}
		}
	}

	void plane_cube_collision_response(glm::vec4 point, Cube* cube, Plane* plane) {
		glm::vec4 point_velocity = cube->linear_velocity + glm::vec4(glm::cross(cube->angular_velocity.xyz(), (point - cube->center_of_mass).xyz()), 0.0);
		float init_relative_velocity = glm::dot(point_velocity.xyz(), plane->normal.xyz());
		glm::vec4 r = point - cube->center_of_mass;

		if (init_relative_velocity < 0.0) {
			
			glm::mat3 inertia_matrix = cube->get_world_inertia_matrix();
			float cr = 0.1f;

			float j = (-(1 + cr) * init_relative_velocity) / ((1/cube->mass) + glm::dot(plane->normal.xyz(), (inertia_matrix * glm::cross(glm::cross(r.xyz(), plane->normal.xyz()), r.xyz()))));

			glm::vec4 delta_v = (1/cube->mass)*j * plane->normal;
			//std::cout << "delta_v" << glm::to_string(delta_v) << "\n";
			glm::vec4 delta_omega = glm::vec4(j * inertia_matrix * glm::cross(r.xyz(), plane->normal.xyz()), 0.0);

			cube->linear_velocity = cube->linear_velocity + delta_v;
			cube->angular_velocity = cube->angular_velocity + delta_omega;

		}

		float drag_c = 1.0;
		float relative_acc  = glm::dot(plane->normal.xyz(), cube->force.xyz() / cube->mass);
		if (relative_acc <= 0 && init_relative_velocity > -0.2 && init_relative_velocity < 0.2) {
			glm::vec4 resting_force = plane->normal * -cube->mass * relative_acc;
			glm::vec4 friction = cube->mass*(-cube->linear_velocity * drag_c);
			glm::vec4 torque_friction = glm::vec4(cube->inertia_matrix*-(cube->angular_velocity.xyz * drag_c), 0.0);
			float tangential_velocity = glm::dot(r.xyz(), cube->angular_velocity.xyz());

			//std::cout << "tangential vel: " << tangential_velocity << "\n";
			cube->collision_force = cube->collision_force + resting_force;
			cube->friction_force = friction;

			cube->friction_torque = torque_friction;

		}
	}

		

};

#endif 
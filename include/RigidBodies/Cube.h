#ifndef CUBE_H
#define CUBE_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW

#include <RigidBodies/RigidMain.h>
#include <vector>
struct FacePlane {
	glm::vec4 normal;
	std::vector<int> points = std::vector<int>(4, 0);
};

class Cube : public MainRigidBody  {

public:
	Loader loader;
	bool once = true;
	Renderer renderer;
	std::vector<glm::vec4> box_mesh = std::vector<glm::vec4>(8);
	std::vector<RawModel> box_meshes;
	std::vector<FacePlane> faces;
	glm::vec4 friction_force;
	glm::vec4 friction_torque;

	Cube(Shader* shader, glm::vec4 starting_pos, glm::vec4 starting_velocity, glm::vec4 starting_angular, glm::vec3 scale, float mass) : MainRigidBody(shader, starting_pos, starting_velocity, starting_angular, scale, mass) {
		load_model();
		create_properties();
	}

	void draw_box() {
		RawModel box_mesh = box_meshes[0];
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0, 0, -15));
		
		model = model*local_coords_matrix;
		glUniformMatrix4fv(glGetUniformLocation(program->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
		renderer.render(box_mesh, GL_TRIANGLES);
	}

	void update_motion(/*CollisionHandler* collision_handler*/) {

		get_forces();
		symplectic_integration();
		update_local_coords();
	}

	glm::vec3 get_maxes() {
		
		float max_x = -1000000;
		float max_y = -1000000;
		float max_z = -1000000;
		for (int i = 0; i < box_mesh.size(); i++) {
			glm::vec4 world_point = box_mesh[i] * local_coords_matrix;
			if (world_point.x > max_x) {
				max_x = world_point.x;
			}
			if (world_point.y > max_y) {
				max_y = world_point.y;
			}
			if (world_point.z > max_z) {
				max_z = world_point.z;
			}
		}
		return glm::vec3(max_x, max_y, max_z);
	}

	glm::vec3 get_mins() {
		float min_x = 10000000;
		float min_y = 10000000;
		float min_z = 10000000;
		for (int i = 0; i < box_mesh.size(); i++) {
			glm::vec4 world_point = box_mesh[i] * local_coords_matrix;
			if (world_point.x < min_x) {
				min_x = world_point.x;
			}
			if (world_point.y < min_y) {
				min_y = world_point.y;
			}
			if (world_point.z < min_z) {
				min_z = world_point.z;
			}

		}
		return glm::vec3(min_x, min_y, min_z);
	}

	glm::mat3 get_world_inertia_matrix() {
		return rotation_matrix * glm::inverse(inertia_matrix) * glm::transpose(rotation_matrix);
	}

private:

	void get_forces(/*CollisionHandler* collision_handler*/) {
		torque = glm::vec4(0, 0, 0, 0);
		force = glm::vec4(0, 0, 0, 0);

		force = force + glm::vec4(0, -9.8f, 0.0, 0.0)*mass + collision_force + friction_force;
		torque = torque +  collision_torque + friction_torque;

		collision_force = glm::vec4(0, 0, 0, 0);
		collision_torque = glm::vec4(0, 0, 0, 0);

	}

	void symplectic_integration() {
		
		// linear movement
		linear_velocity = linear_velocity + (time_step * force) / mass;
		//std::cout << "linear vel: " << glm::to_string(linear_velocity) << "\n";
		center_of_mass = center_of_mass + time_step * linear_velocity;

		// angular movement
		glm::mat3 rotation_matrix = glm::mat3_cast(rotation_quat);
		glm::mat3 world_inertia = rotation_matrix * glm::inverse(inertia_matrix) * glm::transpose(rotation_matrix);
		
		glm::vec3 angular_momentum = inertia_matrix * angular_velocity;
		
		//angular velocity
		glm::vec3 omega = world_inertia * angular_momentum;
		glm::quat omega_quat = glm::quat(0, omega);
		glm::quat omega_quat_prime = (0.5f) * omega_quat * rotation_quat;

		angular_velocity = angular_velocity + time_step*glm::vec4(world_inertia*torque.xyz(), 0.0);
		rotation_quat = rotation_quat + time_step * omega_quat_prime;
		
		rotation_quat = glm::normalize(rotation_quat);
	}

	void update_local_coords() {
		rotation_matrix = glm::mat3_cast(rotation_quat);
		local_coords_matrix = glm::transpose(	glm::mat4(glm::vec4(rotation_matrix[0], 0.0), 
												glm::vec4(rotation_matrix[1], 0.0), 
												glm::vec4(rotation_matrix[2], 0.0), 
												center_of_mass));
	}

	void create_properties() {
		create_local_coords();
		create_inertia_matrix();
	}

	void create_local_coords() {
		glm::vec3 sum_of_position = glm::vec4(0.0, 0.0, 0.0, 0.0);
		//calculate center of mass
		float total_mass = 0;
		for (auto point : box_mesh) {
			sum_of_position = sum_of_position + point.xyz();
			total_mass += 1.0f;
		}
		center_of_mass = position + glm::vec4(sum_of_position * (1.0f/total_mass), 1.0);
		rotation_matrix = glm::mat3_cast(rotation_quat);

		local_coords_matrix = glm::transpose(glm::mat4(glm::vec4(rotation_matrix[0],0.0), glm::vec4(rotation_matrix[1],0.0), glm::vec4(rotation_matrix[2], 0.0), center_of_mass));

	}

	void create_inertia_matrix() {
		//since our object is a box, the moment of inertia is quite simple to compute
		float length = glm::length(box_mesh[0] - box_mesh[3]);
		float width = glm::length(box_mesh[0] - box_mesh[4]);
		float height = glm::length(box_mesh[0] - box_mesh[1]);
		
		glm::vec3 row1 = glm::vec3((mass/12.0f)*(pow(width, 2) + pow(height, 2)), 0.0, 0.0);
		glm::vec3 row2 = glm::vec3(0.0, (mass / 12.0f) * ((length, 2) + pow(width, 2)), 0.0);
		glm::vec3 row3 = glm::vec3(0.0, 0.0, (mass / 12.0f) * (pow(length, 2) + pow(height, 2)));
	
		inertia_matrix = glm::mat3(row1, row2, row3);

	}

	void form_triangle(float arr[], float normals[], int triangle_index, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3, glm::vec4 normal) {
		int arr_index = triangle_index * 9;
		normals[arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;
		normals[++arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;
		normals[++arr_index] = normal.x;
		normals[++arr_index] = normal.y;
		normals[++arr_index] = normal.z;


		arr_index = triangle_index * 9;
		arr[arr_index] = p1.x;
		arr[++arr_index] = p1.y;
		arr[++arr_index] = p1.z;
		arr[++arr_index] = p2.x;
		arr[++arr_index] = p2.y;
		arr[++arr_index] = p2.z;
		arr[++arr_index] = p3.x;
		arr[++arr_index] = p3.y;
		arr[++arr_index] = p3.z;
	}


	void add_face_plane(glm::vec4 n, int p1, int p2, int p3, int p4) {
		FacePlane curr_plane;
		curr_plane.normal = n;
		curr_plane.points[0] = p1;
		curr_plane.points[1] = p2;
		curr_plane.points[2] = p3;
		curr_plane.points[3] = p4;
		faces.push_back(curr_plane);
	}

	void load_model() {

		const int num_vertices = 12 * 3 * 3;
		float mesh_vertices[num_vertices];
		float normals[num_vertices];

		glm::mat4 scale_matrix = glm::mat4(1.0f);
		scale_matrix = glm::scale(scale_matrix, this->scale.xyz());

		glm::vec4 p1 = glm::vec4(-0.5, -0.5, 0.5, 1)*scale_matrix;
		glm::vec4 p2 = glm::vec4(-0.5, 0.5, 0.5, 1)*scale_matrix;
		glm::vec4 p3 = glm::vec4(0.5, 0.5, 0.5, 1)*scale_matrix;
		glm::vec4 p4 = glm::vec4(0.5, -0.5, 0.5, 1)*scale_matrix;
		
		glm::vec4 p5 = glm::vec4(-0.5, -0.5, -0.5, 1)*scale_matrix;
		glm::vec4 p6 = glm::vec4(-0.5, 0.5, -0.5, 1)*scale_matrix;
		glm::vec4 p7 = glm::vec4(0.5, 0.5, -0.5, 1)*scale_matrix;
		glm::vec4 p8 = glm::vec4(0.5, -0.5, -0.5, 1)*scale_matrix;

		box_mesh[0] = p1;
		box_mesh[1] = p2;
		box_mesh[2] = p3;
		box_mesh[3] = p4;
		box_mesh[4] = p5;
		box_mesh[5] = p6;
		box_mesh[6] = p7;
		box_mesh[7] = p8;

		//front side
		form_triangle(mesh_vertices, normals, 0, p1, p2, p3, glm::vec4(0, 0, 1, 0));
		form_triangle(mesh_vertices, normals, 1, p1, p3, p4, glm::vec4(0, 0, 1, 0));
		add_face_plane(glm::vec4(0, 0, 2, 0), 0, 1, 2, 3);

		
		//left side  
		form_triangle(mesh_vertices, normals, 2, p1, p2, p6, glm::vec4(-1, 0, 0, 0));
		form_triangle(mesh_vertices, normals, 3, p1, p6, p5, glm::vec4(-1, 0, 0, 0));
		add_face_plane(glm::vec4(-1, 0, 0, 0), 0, 1, 4, 5);

		//right side
		form_triangle(mesh_vertices, normals, 4, p4, p3, p7, glm::vec4(1, 0, 0, 0));
		form_triangle(mesh_vertices, normals, 5, p4, p7, p8, glm::vec4(1, 0, 0, 0));
		add_face_plane(glm::vec4(1, 0, 0, 0), 3, 2, 6, 7);

		//back side
		form_triangle(mesh_vertices, normals, 6, p5, p6, p7, glm::vec4(0, 0, -1, 0));
		form_triangle(mesh_vertices, normals, 7, p5, p7, p8, glm::vec4(0, 0, -1, 0));
		add_face_plane(glm::vec4(0, 0, -1, 0), 4, 5, 6, 7);

		//top
		form_triangle(mesh_vertices, normals, 8, p2, p6, p7, glm::vec4(0, 1, 0, 0));
		form_triangle(mesh_vertices, normals, 9, p2, p7, p3, glm::vec4(0, 1, 0, 0));
		add_face_plane(glm::vec4(0, 1, 0, 0), 1, 5, 2, 6);

		//bottom
		form_triangle(mesh_vertices, normals, 10, p1, p5, p8, glm::vec4(0, -1, 0, 0));
		form_triangle(mesh_vertices, normals, 11, p1, p8, p4, glm::vec4(0, -1, 0, 0));
		add_face_plane(glm::vec4(0, -1, 0, 0), 0, 4, 3, 7);
	
		RawModel new_rawModel = loader.loadToVAO(mesh_vertices, normals, num_vertices * sizeof(float));
		box_meshes.push_back(new_rawModel);
	}
	//loads the raw model for a cube


};


#endif

#ifndef CUBE_H
#define CUBE_H
#define GLM_FORCE_SWIZZLE
#define GLM_SWIZZLE_XYZW

#include <RigidBodies/RigidMain.h>
#include <vector>


class Cube : public MainRigidBody  {

public:
	Loader loader;
	Renderer renderer;
	std::vector<glm::vec4> box_mesh = std::vector<glm::vec4>(8);
	std::vector<RawModel> box_meshes;
	Cube(Shader* shader, glm::vec4 starting_pos, glm::vec4 starting_rotation, glm::vec3 scale) : MainRigidBody(shader, starting_pos, starting_rotation, scale) {
		load_model();
		center_of_mass = get_center_mass();
		std::cout << glm::to_string(center_of_mass) << "\n";
	}

	void draw_box() {
		RawModel box_mesh = box_meshes[0];
		glm::mat4 model = glm::mat4(1.0f);
		//model = glm::translate(model, glm::vec3(0, 0, -15));
		glUniformMatrix4fv(glGetUniformLocation(program->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINES);
		renderer.render(box_mesh, GL_TRIANGLES);
	}

private:

	void create_properties() {
		create_local_coords();
		create_inertia_matrix();
	}

	void create_local_coords() {
		glm::vec4 sum_of_position = glm::vec4(0.0, 0.0, 0.0, 0.0);
		//calculate center of mass
		float total_mass = 0;
		for (auto point : box_mesh) {
			sum_of_position = sum_of_position + point;
			total_mass += 1.0f;
		}

		//calculate rotation matrix
		glm::vec3 ux = glm::normalize(box_mesh[0].xyz() - center_of_mass.xyz());
		glm::vec3 uy = glm::normalize(glm::cross(ux, (box_mesh[1] - center_of_mass).xyz()));
		glm::vec3 uz = glm::normalize(glm::cross(ux, uy));
		rotation_matrix = glm::mat3(ux, uy, uz);

		center_of_mass = sum_of_position * (1.0f/total_mass);
	}

	void create_inertia_matrix() {
		//since our object is a box, the moment of inertia is quite simple to compute
		//TODO

	}

	void form_triangle(float arr[], float normals[], int triangle_index, glm::vec4 p1, glm::vec4 p2, glm::vec4 p3) {
		int arr_index = triangle_index * 9;
		glm::vec4 edge1 = p2 - p1;
		glm::vec4 edge2 = p3 - p1;
		glm::vec3 normal = glm::cross(edge1.xyz(), edge2.xyz());

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

	void load_model() {

		const int num_vertices = 12 * 3 * 3;
		float mesh_vertices[num_vertices];
		float normals[num_vertices];

		glm::vec4 p1 = glm::vec4(0.0, 0.0, 0.0, 1);
		glm::vec4 p2 = glm::vec4(0.0, 1.0, 0.0, 1);
		glm::vec4 p3 = glm::vec4(1.0, 1.0, 0.0, 1);
		glm::vec4 p4 = glm::vec4(1.0, 0.0, 0.0, 1);
		
		glm::vec4 p5 = glm::vec4(0.0, 0.0, -1.0, 1);
		glm::vec4 p6 = glm::vec4(0.0, 1.0, -1.0, 1);
		glm::vec4 p7 = glm::vec4(1.0, 1.0, -1.0, 1);
		glm::vec4 p8 = glm::vec4(1.0, 0.0, -1.0, 1);

		box_mesh[0] = p1;
		box_mesh[1] = p2;
		box_mesh[2] = p3;
		box_mesh[3] = p4;
		box_mesh[4] = p5;
		box_mesh[5] = p6;
		box_mesh[6] = p7;
		box_mesh[7] = p8;

		//front side
		form_triangle(mesh_vertices, normals, 0, p1, p2, p3);
		form_triangle(mesh_vertices, normals, 1, p1, p3, p4);

		//left side  
		form_triangle(mesh_vertices, normals, 2, p1, p2, p6);
		form_triangle(mesh_vertices, normals, 3, p1, p6, p5);

		//right side
		form_triangle(mesh_vertices, normals, 4, p4, p3, p7);
		form_triangle(mesh_vertices, normals, 5, p4, p7, p8);

		//back side
		form_triangle(mesh_vertices, normals, 6, p5, p6, p7);
		form_triangle(mesh_vertices, normals, 7, p5, p7, p8);

		//top
		form_triangle(mesh_vertices, normals, 8, p2, p6, p7);
		form_triangle(mesh_vertices, normals, 9, p2, p7, p3);

		//bottom
		form_triangle(mesh_vertices, normals, 10, p1, p5, p8);
		form_triangle(mesh_vertices, normals, 11, p1, p8, p4);
		
		RawModel new_rawModel = loader.loadToVAO(mesh_vertices, normals, num_vertices * sizeof(float));
		box_meshes.push_back(new_rawModel);
	}
	//loads the raw model for a cube


};


#endif

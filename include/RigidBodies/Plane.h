#ifndef PLANE_H
#define PLANE_H

#include <RigidBodies/RigidMain.h>
#include <vector>

class Plane {

public:
	Shader* program;

	glm::vec4 position;
	glm::vec4 normal;
	std::vector<RawModel> plane_mesh;
	int num_bones;

	Plane(glm::vec4 position, glm::vec4 normal, Shader* program) {
		this->position = position;
		this->normal = normal;
		this->program = program;
		create_mesh();
	}

	void draw_plane() {
		glUniform3fv(glGetUniformLocation(program->ID, "objectColor"), 1, glm::value_ptr(glm::vec3(1.0, 0.0, 1.0)));
		glUniformMatrix4fv(glGetUniformLocation(program->ID, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

		renderer.render(plane_mesh[0], GL_TRIANGLES);
	}

private:
	Loader loader;
	Renderer renderer;

	void create_mesh() {
		float sideLength = 10000;
		//glm::vec3 p1 = glm::cross(normal.xyz(), position.xyz());	
		//glm::vec3 p2 = p1*-1.0f;

		glm::vec3 v1 = normalize(cross(normal.xyz(), glm::vec3(1.0f, 0.0f, 0.0f)));
		glm::vec3 v2 = normalize(cross(normal.xyz(), v1));

		// Calculate the coordinates of the other three points
		glm::vec3 p1 = position.xyz() + sideLength / 2.0f * v1 + sideLength / 2.0f * v2;
		glm::vec3 p2 = position.xyz() + sideLength / 2.0f * v1 - sideLength / 2.0f * v2;
		glm::vec3 p3 = position.xyz() - sideLength / 2.0f * v1 + sideLength / 2.0f * v2;

		std::cout << "p0" << glm::to_string(position) << "p1: " << glm::to_string(p1) << ",p2: " << glm::to_string(p2) << ", p3: " << glm::to_string(p3) << "\n";
		float plane_vertices[6 * 3];
		float normals[6 * 3];
		for (int j = 0; j < 6; j++) {
			normals[(j * 3)] = normal.x;
			normals[(j * 3) + 1] = normal.y;
			normals[(j * 3) + 2] = normal.z;
		}

		int i = 0;

		// first triangle
		plane_vertices[i++] = p3.x;
		plane_vertices[i++] = p3.y;
		plane_vertices[i++] = p3.z;

		plane_vertices[i++] = p1.x;
		plane_vertices[i++] = p1.y;
		plane_vertices[i++] = p1.z;

		plane_vertices[i++] = p2.x;
		plane_vertices[i++] = p2.y;
		plane_vertices[i++] = p2.z;

		//second triangle
		plane_vertices[i++] = p3.x;
		plane_vertices[i++] = p3.y;
		plane_vertices[i++] = p3.z;

		plane_vertices[i++] = p2.x;
		plane_vertices[i++] = p2.y;
		plane_vertices[i++] = p2.z;

		plane_vertices[i++] = position.x;
		plane_vertices[i++] = position.y;
		plane_vertices[i++] = position.z;



		plane_mesh.push_back(loader.loadToVAO(plane_vertices, normals, (18) * sizeof(float)));
	}

};



#endif
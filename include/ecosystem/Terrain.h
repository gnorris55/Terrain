#ifndef TERRAIN_H
#define TERRAIN_H

class Terrain {

public:

	
	Shader* shader;
	Shader *grass_shader;
	Renderer renderer;
	glm::vec4 position;
	std::vector<RawModel> terrain_mesh;
	std::vector<RenderTile*> render_tiles;
	unsigned int texture;
	unsigned int clumping_tex;
	unsigned int heights_tex;
	unsigned int wind_texture;

	Terrain(Shader* shader, Shader *grass_shader, glm::vec4 position, const char *grass_heights) {
		this->shader = shader;
		this->grass_shader = grass_shader;
		this->position = position;

		createTerrainMesh();
		load_textures(grass_heights);
		//load_grass_tex();
		//loadTerrainTexture();
		defineRenderTiles();
		std::cout << grass_shader->ID << "\n";
	}

	~Terrain() {
		for (RenderTile* renderTile : render_tiles) {
			delete renderTile;
		}
	}

	void draw(float time) {

		shader->use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		RawModel temp_mesh = terrain_mesh[0];
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::transpose(glm::translate(model, position.xyz()));

		glUniform3fv(glGetUniformLocation(shader->ID, "objectColor"), 1, glm::value_ptr(glm::vec3(0.3, 0.3, 0.3)));

		glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, glm::value_ptr(model));
		renderer.render(temp_mesh, GL_TRIANGLES);
		
		for (int i = 0; i < 64*64; i++) {
			render_tiles[i]->drawGrass(time);

		}

	}


	
private:
	const int TERRAIN_SIZE = 128;
	const int SQUARE_SIZE = 18;
	const int NUM_DIVISION = 64;
	glm::vec3 points[64][64];
	glm::vec2 globalTexCoord[64][64];
	Loader loader;


	void createTerrainMesh() {
	
		const int num_vertices = 73728;
		const int num_tex = 49152;
		float vertices[num_vertices];
		float normals[num_vertices];
		float tex_coord[num_tex];

		setPoints();
		mapVertices(vertices, normals, tex_coord);

		RawModel temp_rawModel = loader.loadToVAOTexture(vertices, normals, tex_coord, sizeof(vertices) - ((NUM_DIVISION * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE), sizeof(tex_coord) - ((NUM_DIVISION * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE));
		//RawModel temp_rawModel = loader.loadToVAOTexture(vertices, normals, tex_coord, sizeof(vertices) - ((NUM_DIVISION * sizeof(float) * SQUARE_SIZE) * 2) + (sizeof(float) * SQUARE_SIZE), num_tex * sizeof(float));
		terrain_mesh.push_back(temp_rawModel);

	}
	unsigned int load_texture(const char *texture_name) {

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// load image, create texture and generate mipmaps
		int width, height, nrChannels;
		// The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
		unsigned char* data = stbi_load(texture_name, &width, &height, &nrChannels, 0);
		std::cout << "height: " << height << "\n";
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
		return texture;
	}

	void load_textures(const char *height_texture) {
		//const char* texture_name = "Textures / dirt.jpg";
		texture = load_texture("Textures/dirt.jpg");
		heights_tex = load_texture(height_texture);
		wind_texture = load_texture("Textures/wind.jpg");
	}

	void defineRenderTiles() {
		float tile_step = (float)TERRAIN_SIZE / (float)NUM_DIVISION;
		ComputeShader compute_shader("Shaders/computeShader.cs");
		float heights[64*64];
		float winds[64 * 64*3];

		glBindTexture(GL_TEXTURE_2D, heights_tex);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, heights);
		
		glBindTexture(GL_TEXTURE_2D, wind_texture);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, winds);

		
		for (int i = 0; i < 10; i++) {
			std::cout << i << ": " << heights[i] << "\n";
		}

		for (int i = 0; i < 64; i++) {
			for (int j = 0; j < 64; j++) {
				//std::cout << i << ", " << j << ": " << heights[i * 64 + j] << "\n";
				glm::vec3 wind_normal;
				wind_normal.x = winds[(i * 64 + j) * 3];
				wind_normal.y = winds[(i * 64 + j) * 3 + 1];
				wind_normal.z = winds[(i * 64 + j) * 3 + 1];
				
				glm::vec4 tile_position = position;
				tile_position.x += j * tile_step;
				tile_position.z += i * tile_step;
				RenderTile* temp_tile = new RenderTile(glm::vec4(points[0][0], 0.0) + tile_position, tile_step, heights[i * 64 + j], glm::normalize(wind_normal), &compute_shader, grass_shader);
				render_tiles.push_back(temp_tile);

			}
		}

	}

	void mapVertices(float vertices[], float normals[], float tex_coord[]) {
		int vertex = 0;
		int normalsVertex = 0;
		int texVertex = 0;
		for (int i = 0; i < NUM_DIVISION; i++) {
			for (int j = 0; j < NUM_DIVISION; j++) {
				if ((j != NUM_DIVISION - 1 && i != NUM_DIVISION - 1) || (j == NUM_DIVISION - 1 && i == NUM_DIVISION - 1)) {
					glm::vec2 inputTex[] = {globalTexCoord[i][j] , globalTexCoord[i][j + 1], globalTexCoord[i + 1][j], globalTexCoord[i + 1][j + 1]};
					glm::vec3 inputPoints[] = { points[i][j], points[i][j + 1], points[i + 1][j], points[i + 1][j + 1] };
					mapSquare(vertices, normals, tex_coord, &vertex, &normalsVertex, &texVertex, inputPoints, inputTex, glm::vec3(1.0f, 1.0f, 1.0f));

				}

			}

		}
	}

	void setPoints() {
		float posX, posY, posZ;
		for (int i = 0; i < NUM_DIVISION; i++) {
			for (int j = 0; j < NUM_DIVISION; j++) {
				posX = (float)j / ((float)NUM_DIVISION - 1) * TERRAIN_SIZE;
				posY = 0;
				posZ = (float)i / ((float)NUM_DIVISION - 1) * TERRAIN_SIZE;
				points[j][i] = glm::vec3(posX, posY, posZ);
				//heightMap[j][i] = posY;

				glm::vec2 tempTexCoord;
				tempTexCoord.x = (float)j / ((float)NUM_DIVISION - 1);
				tempTexCoord.y = (float)i / ((float)NUM_DIVISION - 1);
				globalTexCoord[j][i] = tempTexCoord;
			}

		}

	}

	//todo create math class
	void mapSquare(float vertices[], float normals[], float tex[], int* vertex, int* normalsVertex, int* texVertex, glm::vec3 points[], glm::vec2 textureCoord[], glm::vec3 normalDir) {
		//first triangle
		vertexToElement(vertices, vertex, points[0]);
		vertexToElement(vertices, vertex, points[1]);
		vertexToElement(vertices, vertex, points[2]);

		glm::vec3 normalVec = calculateNormals(points[0], points[1], points[2]);
		normalVec *= normalDir;
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);

		texToElement(tex, texVertex, textureCoord[0]);
		texToElement(tex, texVertex, textureCoord[1]);
		texToElement(tex, texVertex, textureCoord[2]);


		//second triangle
		vertexToElement(vertices, vertex, points[1]);
		vertexToElement(vertices, vertex, points[3]);
		vertexToElement(vertices, vertex, points[2]);

		normalVec = calculateNormals(points[1], points[3], points[2]);
		normalVec *= normalDir;
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		vertexToElement(normals, normalsVertex, normalVec);
		
		texToElement(tex, texVertex, textureCoord[1]);
		texToElement(tex, texVertex, textureCoord[3]);
		texToElement(tex, texVertex, textureCoord[2]);
	}

	void vertexToElement(float vertices[], int* vertex, glm::vec3 vector) {
		vertices[*vertex] = vector.x;
		vertices[*vertex + 1] = vector.y;
		vertices[*vertex + 2] = vector.z;
		*vertex += 3;
	}
	
	void texToElement(float tex[], int* vertex, glm::vec2 vector) {
		tex[*vertex] = vector.x;
		tex[*vertex + 1] = vector.y;
		*vertex += 2;
	}

	glm::vec3 calculateNormals(glm::vec3 vectorA, glm::vec3 vectorB, glm::vec3 vectorC) {
		//(B-A)x(C-A)
		//
		return glm::cross(vectorB - vectorA, vectorC - vectorA);
	}

};

#endif
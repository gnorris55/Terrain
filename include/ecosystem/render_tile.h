#ifndef RENDER_TILE_H
#define RENDER_TILE_H
class RenderTile {

public:

	ComputeShader *compute_shader;
	Shader* grass_shader;
	static constexpr int TEXTURE_WIDTH = 32, TEXTURE_HEIGHT = 32;
	glm::vec4 position;
	
	float tile_step;
	float tile_height;
	glm::vec3 wind_normal;
	
	unsigned int grass_VAO;
	unsigned int grass_texture1;
	unsigned int grass_texture2;

	RenderTile(glm::vec4 position, float tile_step, float tile_height, glm::vec3 wind_normal,  ComputeShader *compute_shader, Shader *grass_shader) {
		this->position = position;
		this->wind_normal = wind_normal;
		this->compute_shader = compute_shader;
		this->grass_shader = grass_shader;
		this->tile_step = tile_step;
		this->tile_height = tile_height;
		createTextures();
		runGrassDisplacementAlg();
		//std::cout << "wind: " << glm::to_string(wind_normal) << "\n";
		//std::cout << "ID: " << grass_shader->ID << "\n";
	}

	float get_sin_value(float time, float displacement) {
		float freq = 1.5;
		//0.5 * (np.sin(2 * np.pi * f * t + phi) + 1)
		return sin(glm::radians(180.0f) * freq * time + displacement);
	}

	glm::vec4 get_sin_vals(float time, float offset, float degree_step) {

		glm::vec4 times;
		times.x = get_sin_value(time, offset + degree_step);
		times.y= get_sin_value(time, offset + degree_step*2);
		times.z = get_sin_value(time, offset + degree_step*3);
		times.w = get_sin_value(time, offset + degree_step*4);
		//times.x = sin(time*glm::radians(90.0) + offset + degree_step);
		//times.y = sin(time*glm::radians(90.0) + offset + degree_step * 2);
		//times.z = sin(time*glm::radians(90.0) + offset + degree_step * 3);
		//times.w = sin(time*glm::radians(90.0) + offset + degree_step * 4);
		
		return times;
	}

	void drawGrass(float time) {

		if (tile_height > 0.1f) {
			grass_shader->use();
			glm::mat4 model = glm::mat4(1.0f);

			glm::vec4 sin_val = get_sin_vals(time, glm::radians(0.0f), glm::radians(20.0f));
			glm::vec4 sin_val2 = get_sin_vals(time, glm::radians(80.0f), glm::radians(20.0f));

			grass_shader->setMat4("model", model);
			grass_shader->setFloat("tile_height", tile_height);
			grass_shader->setVec3("colour", glm::vec3(0.28, 0.43, 0.21));
			grass_shader->setMat4("model", glm::mat4(1.0f));
			grass_shader->setVec3("lightColor", 1.0f, 1.0f, 1.0f);
			grass_shader->setVec3("lightPos", 0.0f, 10000.0f, 0.0f);
			grass_shader->setVec4("sin_values", sin_val);
			grass_shader->setVec4("sin_values2", sin_val2);
			grass_shader->setVec3("wind_normal", wind_normal);
			//grass_shader->setVec3("wind_normal", glm::vec3(0, 0, 0));

			glBindVertexArray(grass_VAO);
			glDrawArrays(GL_POINTS, 0, (TEXTURE_HEIGHT) * (TEXTURE_WIDTH));
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			glBindVertexArray(grass_VAO);
		}
	}

private:

	
	void createTextures() {
	
		glGenTextures(1, &grass_texture1);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass_texture1);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindImageTexture(0, grass_texture1, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, grass_texture1);
		
		glGenTextures(1, &grass_texture2);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, grass_texture2);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);

		glBindImageTexture(1, grass_texture2, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, grass_texture2);
	}


	// this function uses compute shaders to create a random texture map for grass offset
	// TODO: implement culling based on occlusion
	void runGrassDisplacementAlg() {

		compute_shader->use();
		compute_shader->setVec2("position", position.xz());
		glDispatchCompute((unsigned int)TEXTURE_WIDTH, (unsigned int)TEXTURE_HEIGHT, 1);

		// make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		float displacements[TEXTURE_WIDTH * TEXTURE_HEIGHT * 2]; // 3 channels (RGB)

		float heights[TEXTURE_WIDTH * TEXTURE_HEIGHT]; // 3 channels (RGB)
		
		float grass_params[TEXTURE_WIDTH * TEXTURE_HEIGHT*3]; // 3 channels (RGB)
		

		// Get the pixel values
		glBindTexture(GL_TEXTURE_2D, grass_texture1);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RG, GL_FLOAT, displacements);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_BLUE, GL_FLOAT, heights);

		glBindTexture(GL_TEXTURE_2D, grass_texture2);
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_FLOAT, grass_params);

		float grass_positions[TEXTURE_WIDTH * TEXTURE_HEIGHT * 3];
		//float length = sqrt(grass_positions.size());
		for (int i = 0; i < TEXTURE_HEIGHT; i++) {
			for (int j = 0; j < TEXTURE_WIDTH; j++) {
				float x_dis = tile_step / (float)TEXTURE_WIDTH * j + (tile_step / 2) / (float)TEXTURE_WIDTH;
				float z_dis = tile_step / (float)TEXTURE_HEIGHT * i + (tile_step / 2) / (float)TEXTURE_HEIGHT;
				glm::vec3 temp_position = position;
				temp_position.x += x_dis + tile_step / (float)TEXTURE_WIDTH * (displacements[(i*31+j)*2] - 0.5);
				temp_position.z += z_dis + tile_step / (float)TEXTURE_WIDTH * (displacements[((i * 31 + j) * 2) + 1] - 0.5);
				
				grass_positions[((i * (TEXTURE_HEIGHT)) + j)*3] = temp_position.x;
				grass_positions[((i * (TEXTURE_HEIGHT)) + j)*3+1] = temp_position.y;
				grass_positions[((i * (TEXTURE_HEIGHT)) + j)*3+2] = temp_position.z;
			}
			
		}
		/*
		for (int i = 0; i < 64; i++) {
			std::cout << "float val: " << curve_coefficients[i] << "\n";
		}*/


		glGenVertexArrays(1, &grass_VAO);
		glBindVertexArray(grass_VAO);

		add_buffer_element(grass_positions, sizeof(grass_positions), 0, 3);
		add_buffer_element(heights, sizeof(heights), 1, 1);
		add_buffer_element(grass_params, sizeof(grass_params), 2, 3);



		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void add_buffer_element(float vertices[], int size, int attribNum, int num_vert) {
		unsigned int VBO;
		glGenBuffers(1, &VBO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

		glVertexAttribPointer(attribNum, num_vert, GL_FLOAT, GL_FALSE, num_vert * sizeof(float), (void*)0);
		glEnableVertexAttribArray(attribNum);	
	}
	
};

#endif
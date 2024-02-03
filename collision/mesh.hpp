#ifndef __CMESH
#define __CMESH

class CMesh
{

public:



	std::vector<glm::vec3> OBJ_vertices;
	std::vector<glm::vec3> OBJ_normals;
	std::vector<glm::vec2> OBJ_uvs;

	bool isAlive = true;
	bool isAlien = false;

	float rotationAngle = 0.0f;
    float rotationSpeed = 0.5f; 
    float currentBounce = 0.0f;

    glm::mat4x4 matModel = glm::mat4(1.0);


    void Init(const char *_obj_file, const char *_tex_file, bool _isAlien = false)
    {

		if (!loadOBJ(_obj_file, OBJ_vertices, OBJ_uvs, OBJ_normals))
		{
			printf("OBJ error!\n");
			exit(1);
		}
		GLuint vBuffer_pos, vBuffer_uv;
		glGenVertexArrays( 1, &idVAO );
		glBindVertexArray( idVAO );
		glGenBuffers( 1, &vBuffer_pos );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_pos );
		glBufferData( GL_ARRAY_BUFFER, OBJ_vertices.size() * sizeof(glm::vec3), &(OBJ_vertices)[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 0 );
		glGenBuffers( 1, &vBuffer_uv );
		glBindBuffer( GL_ARRAY_BUFFER, vBuffer_uv );
		glBufferData( GL_ARRAY_BUFFER, OBJ_uvs.size() * sizeof(glm::vec2), &(OBJ_uvs)[0], GL_STATIC_DRAW );
		glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, 0, NULL );
		glEnableVertexAttribArray( 1 );
		glBindVertexArray( 0 );

		int tex_width, tex_height, tex_n;
		unsigned char *tex_data;
		tex_data = stbi_load(_tex_file, &tex_width, &tex_height, &tex_n, 0);
		if (!tex_data)
		{
			printf("Texture error!\n");
			exit(1);
		}
		glGenTextures(1, &idTexture);
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		isAlien = _isAlien;

    }

    void Draw()
    {
		if (!isAlive) return;
		GLint idProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &idProgram);

        glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );

		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(idProgram, "tex0"), 0);
		glBindTexture(GL_TEXTURE_2D, idTexture);

        glBindVertexArray( idVAO );
       if (isMulti) {
            for(int i = 0; i < 50; i++) {
				glm::mat4 transform = glm::mat4(1.0f);
				transform = glm::translate(transform, glm::vec3((float) i, 0.0f, 0.0f));
				glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(transform) );
				glDrawArraysInstanced(GL_TRIANGLES, 0, OBJ_vertices.size(), instanceTransforms.size());
			}
        } else {
            glDrawArrays(GL_TRIANGLES, 0, OBJ_vertices.size());
        }
        glBindVertexArray( 0 );
    }

	void Destroy()
	{
		glDeleteVertexArrays(1, &idVAO);
		glDeleteTextures(1, &idTexture);
		isAlive = false;
	}

	glm::vec3 CalculatePositionForInstance(int i)
	{
		// Implement this
		return glm::vec3((float) i, 0.0f, 0.0f);
	}

	float CalculateRotationForInstance(int i)
	{
		// Implement this
		return 0.0f;
	}

	glm::vec3 CalculateScaleForInstance(int i)
	{
		// Implement this
		return glm::vec3(1.0f, 1.0f, 1.0f);
	}

};

class CSceneObject : public CMesh
{
public:

	CCollider *Collider = NULL;
	glm::vec3 Position;

	void SetPosition(glm::vec3 _pos)
	{
		this->Position = _pos;
		matModel = glm::translate(glm::mat4(1.0), this->Position);
	}

	void Animate(int time)
	{

		int fps = (time % 1000) % 60;
		if (fps == 0)
			currentBounce = 0.0f;
		if (fps < 30)
			currentBounce += 0.01f;
		else 
			currentBounce -= 0.01f;
	
        matModel = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, currentBounce, 0.0f));
		
		this->SetPosition(glm::vec3(matModel[3]));

	}

};


#endif

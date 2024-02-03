#ifndef __CMISSLE
#define __CMISSLE

// ----------------------------------------------
// Przyklad implementacji klasy do obslugi
// obiektow 3D renderowanych w OpenGL
// ----------------------------------------------

class CMissle
{

public:


	// potok openGL
    GLuint idVAO;	// identyfikator VAO
    GLuint idTexture; // identyfikator tekstury

	std::vector<glm::vec3> OBJ_vertices;
	std::vector<glm::vec3> OBJ_normals;
	std::vector<glm::vec2> OBJ_uvs;


    // macierz modelu (do renderingu)
    glm::mat4x4 matModel = glm::mat4(1.0);

	glm::vec3 Position = glm::vec3(0.0, 0.0, 0.0);
	glm::vec3 FirstPos = glm::vec3(0.0, 0.0, 0.0);

	glm::vec3 Direction = glm::vec3(1.0, 0.0, 0.0);

	float Angle = 0.0f;

	bool isAlive = false;
	CCollider *Collider = NULL;

    // inicjalizacja (obecnie tylko to co jest
	// potrzebne do renderingu
    void Init(const char *_obj_file, const char *_tex_file)
    {

		// OBJ
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

		// Tekstura
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

    }

    // rendering na scenie
    void Draw()
    {
		if(isAlive == false)
			return;
	
		GLint idProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &idProgram);

		glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 0.1f, 0.1f));

		// Modyfikacja macierzy modelu o skalowanie
		glm::mat4 scaledModelMatrix = matModel * scaleMatrix;

		// wyslanie macierzy modelu
        glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(scaledModelMatrix) );

		// aktywacja tekstury
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(idProgram, "tex0"), 0);
		glBindTexture(GL_TEXTURE_2D, idTexture);

        // rendering
        glBindVertexArray( idVAO );
        glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
        glBindVertexArray( 0 );
    }

	void Update()
	{

		matModel = glm::translate(glm::mat4(1.0), Position);
		matModel = glm::rotate(matModel, Angle, glm::vec3(0.0, 1.0, 0.0));
	}

	// ustawienie polozenia na scenie
	int SetPosition(glm::vec3 pos, float angle, float val, std::vector<CSceneObject>& others)
	{
		if (isAlive == false) {
			Position = pos;
			FirstPos = pos;
			Position.y += 1.0f;
			Angle = angle;
			Direction.x = cos(Angle);
			Direction.z = -sin(Angle);
		}
		isAlive = true;

		// aktualizujemy polozenie
		Position += Direction * val;

		// odleglosc od poczatku
		float dist = glm::distance(FirstPos, Position);
		if (dist > 30.0f)
		{
			isAlive = false;
			return 1;
		}

		bool isColliding = false;
		for(auto& other : others) {
			// wyświetlanie kolidera
			
			if (this->Collider->isCollision(other.Collider->Radius, other.Position) && other.isAlive) {
				isColliding = true;
				other.isAlive = false;
				break; 
			}
		}

		if (isColliding) {
			isAlive = false;
			return 2;

		}

		Update();
		return 0;
	}

};


#endif

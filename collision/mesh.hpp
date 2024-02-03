#ifndef __CMESH
#define __CMESH

// ----------------------------------------------
// Przyklad implementacji klasy do obslugi
// obiektow 3D renderowanych w OpenGL
// ----------------------------------------------

class CMesh
{

public:


	// potok openGL
    GLuint idVAO;	// identyfikator VAO
    GLuint idTexture; // identyfikator tekstury

	std::vector<glm::vec3> OBJ_vertices;
	std::vector<glm::vec3> OBJ_normals;
	std::vector<glm::vec2> OBJ_uvs;

	bool isAlive = true;
	bool isAlien = false;

	float rotationAngle = 0.0f; // Aktualny kąt obrotu
    float rotationSpeed = 0.5f; 
    float currentBounce = 0.0f;

    // macierz modelu (do renderingu)
    glm::mat4x4 matModel = glm::mat4(1.0);


    // inicjalizacja (obecnie tylko to co jest
	// potrzebne do renderingu
    void Init(const char *_obj_file, const char *_tex_file, bool _isAlien = false)
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
		isAlien = _isAlien;

    }

    // rendering na scenie
    void Draw()
    {
		if (!isAlive) return;
		GLint idProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &idProgram);

		// wyslanie macierzy modelu
        glUniformMatrix4fv( glGetUniformLocation( idProgram, "matModel" ), 1, GL_FALSE, glm::value_ptr(matModel) );

		// aktywacja tekstury
		glActiveTexture(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(idProgram, "tex0"), 0);
		glBindTexture(GL_TEXTURE_2D, idTexture);

        // rendering
        glBindVertexArray( idVAO );
        glDrawArrays( GL_TRIANGLES, 0, OBJ_vertices.size() );
        glBindVertexArray( 0 );
    }

	// zniszczenie obiektu
	void Destroy()
	{
		glDeleteVertexArrays(1, &idVAO);
		glDeleteTextures(1, &idTexture);
		isAlive = false;
	}

	// ------------------------------------------
	// animacja obiektu
	// ------------------------------------------



};

// ----------------------------------------------
// Przyklad nowej klasy do obslugi obiektow
// ktore dodatkowo udostepniaja obsluge testu
// kolizji
// ----------------------------------------------
// UWAGA! To tylko propozycja
// ----------------------------------------------

class CSceneObject : public CMesh
{
public:

	// Dodatkowe skladowe wzgledem CMesh

	// ------------------------------------------
	// NOWE: Collider do testu kolizji
	// ------------------------------------------
	CCollider *Collider = NULL;
	glm::vec3 Position;

	// ------------------------------------------
	// NOWE: Ustawienie pozycji obiektu na scenie
	// ------------------------------------------
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
	

        // Uwzględnienie efektu odbijania w macierzy modelu
        matModel = glm::translate(glm::mat4(1.0), glm::vec3(0.0f, currentBounce, 0.0f));
		
		this->SetPosition(glm::vec3(matModel[3]));

	}

};


#endif

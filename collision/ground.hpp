#ifndef __CGROUND
#define __CGROUND

#define __ALTITUDE_ERROR	99999


// -------------------------------
// Klasa reprezentujaca trojkat
class CTriangle
{
public:

	// wspolrzedne punktow trojkata
	glm::vec3 p[3];

	// rownanie plaszczyzny Ax + By + Cz + D = 0
	float A,B,C,D;

	// konstruktory
	CTriangle() { }
	CTriangle(const glm::vec3 &p1, const glm::vec3 &p2, const glm::vec3 &p3)
	{
		p[0] = p1;
		p[1] = p2;
		p[2] = p3;

		calculateEquation();
	}

	// obliczanie wspolczynnikow rownania
	void calculateEquation(void)
	{
		glm::vec3 v1, v2;

		v1 = p[1] - p[0];
		v2 = p[2] - p[0];

		A = v1.y * v2.z - v1.z * v2.y;
		B = v1.z * v2.x - v1.x * v2.z;
		C = v1.x * v2.y - v1.y * v2.x;

		D = - (A*p[0].x + B*p[0].y + C*p[0].z);

		if (C == 0)
		{
			printf("Uwaga! Trojkat pionowy.\n");
		}

	}

	// czy punkt p jest po lewej stronie odcinka (A, B)
	// obliczanie wyznacznika
	inline float det (glm::vec2 p, glm::vec3 A, glm::vec3 B)
	{
		return (p.x - B.x) * (A.z - B.z) - (A.x - B.x) * (p.y - B.z);
	}

	// czy punkt jest wewnatrz trojkata rzutowanego na plaszczyzne xy
	inline bool isInside (glm::vec2 point)
	{
		float d1, d2, d3;
		bool isNeg, isPos;

		d1 = det(point, p[0], p[1]);
		d2 = det(point, p[1], p[2]);
		d3 = det(point, p[2], p[0]);

		// czy jeden z wyznacznikow ma znak ujemny
		isNeg = (d1 < 0) || (d2 < 0) || (d3 < 0);
		// czy jeden z wyznacznikow ma znak dodatni
		isPos = (d1 > 0) || (d2 > 0) || (d3 > 0);

		// czy sa dodatnie i ujemne wartosci wyznacznikow
		// jesli tak, wtedy punkt nie lezy wewnatrz trojkata
		if (isNeg && isPos)
			return false;
		else
			return true;
	}

	// obliczamy wysokosc punktu w trojkacie z rownania plaszczyzny
	inline float calulateY(glm::vec2 point)
	{
		if (B) return -(A*point.x + C*point.y + D)/B;
		else { return __ALTITUDE_ERROR; }
	}

};



// ---------------------------------------
// Klasa reprezentujaca podloze
// ---------------------------------------
class CGround
{
public:

	std::vector<CTriangle> triangles;
	glm::mat4x4 matModel = glm::mat4(1.0);

	
	GLuint idVAO;
	GLuint idTexture;
	std::vector<glm::vec3> OBJ_vertices;
	std::vector<glm::vec2> OBJ_uvs;
	std::vector<glm::vec3> OBJ_normals;

	// stworzenie struktury z listy trojkatow
	// na przyklad przy okazji ladowania z pliku OBJ
	void Init()
	{
		// Tekstura
		int tex_width, tex_height, tex_n;
		unsigned char *tex_data;
		tex_data = stbi_load("assets/moon.jpg", &tex_width, &tex_height, &tex_n, 0);

		glGenTextures(1, &idTexture);
		glBindTexture(GL_TEXTURE_2D, idTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, tex_width, tex_height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex_data);
		glGenerateMipmap(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);


		// VAO z pliku OBJ
		if (!loadOBJ("assets/scene-large.obj", OBJ_vertices, OBJ_uvs, OBJ_normals))
		{
			printf("Not loaded!\n");
			exit(1);
		}
		glGenVertexArrays( 1, &idVAO );
		glBindVertexArray( idVAO );
		GLuint vBuffer_pos, vBuffer_uv;
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

		glm::vec3 p1,p2,p3;

		std::vector<glm::vec3>::iterator it = OBJ_vertices.begin();
		while (it != OBJ_vertices.end())
		{
			p1 = *it++;
			p2 = *it++;
			p3 = *it++;

			// utworzenie nowego obiektu CTriangle
			triangles.push_back(CTriangle( p1, p2, p3) );

		}

		printf("Utworzono CGround zawierajacy %d trojkatow.\n", triangles.size());
	}

	// ----------------------------------------
	// Glowna funkcja obliczajaca wysokosci nad podlozem w punkcie X Z
	// - przeszukanie po wszystkich trojkatach
	// - gdy zostanie znaleziony trojkat, obliczana jest wysokosc Y
	float getAltitute(glm::vec2 point)
	{

		// obliczenie listy trojkatow nad ktorymi jestesmy
		CTriangle *collTriangle = NULL;

		for (auto it = triangles.begin(); it != triangles.end(); it++)
		{
			if ((*it).isInside(point))
			{
				collTriangle = &(*it);
				break;
			}
		}

		// jesli zostal znaleziony
		if (collTriangle)
		{
			return collTriangle->calulateY(point);
		}
		else
		{
			printf("Brak ziemi pod nogami!\n");
			return 9999999.0;
		}
	}

	void Draw()
	{
		
		matModel = glm::mat4(1.0);
		GLint idProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &idProgram);

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

};


#endif

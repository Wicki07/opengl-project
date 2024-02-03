#ifndef __CPLAYER
#define __CPLAYER

// ----------------------------------------------
// Klasa do reprezentacji postaci
// - obiektu, ktory porusza sie po scenie
// ----------------------------------------------
class CPlayer
{

public:

	// pozycja obiektu
	glm::vec3 Position = glm::vec3(0.0, 0.0, 0.0);

	// kat orientacji obiektu
	float Angle = 0.0;

	float timer = 0.0;

	// wektor orientacji (UWAGA! wyliczany z Angle)
	glm::vec3 Direction = glm::vec3(1.0, 0.0, 0.0);

	// wskaznik do obiektu podloza
	// TO_DO: uwzglednic CGround
	// CGround *myGround = NULL;

	// do renderingu wykorzystujemy klase CMesh
	CMesh Mesh;
	CGround *myGround = NULL;


	// ------------------------------------------
	// NOWE: Collider do testu kolizji
	// ------------------------------------------
	CCollider *Collider = NULL;


	CPlayer() { }

	// Inicjalizacja obiektu
	void Init(const char *_obj_file, const char *_tex_file, CGround *ground)
	{


		// ----------------------------------
		// TO_DO: Uzupelnij wedle uznania
		// Najlepiej aby wykorzystac klase CMesh w tym miejscu
		// ----------------------------------

		Mesh.Init(_obj_file, _tex_file);

		myGround = ground;
		float y =  myGround->getAltitute(glm::vec2(Position.x, Position.z));
		Position.y = y;
		// Aktualizacja polozenia/macierzy itp.
		Update();

	}

	// renderowanie obiektu
	void Draw()
	{


		// TO_DO: a moze wykorzystac metode Draw() z klasy CMesh?
		Mesh.Draw();
	}
 

	// Obliczenie wysokosci nad ziemia
	void Update()
	{
		// TO_DO: Wykorzystaj klase CGround i zaktualizuj
		// wspolrzedna Y obiektu zgodnie z podlozem

		// ....

		// TO_DO: nastepnie wylicz nowa macierz modelu i macierz normalna
		Mesh.matModel = glm::translate(glm::mat4(1.0), Position);
		Mesh.matModel = glm::rotate(Mesh.matModel, Angle, glm::vec3(0.0, 1.0, 0.0));
	}



	// ustawienie polozenia na scenie
	void SetPosition(glm::vec3 pos)
	{
		Position = pos;
		Update();
	}

	// zmiana polozenia na scenie bez testu kolizji
	// void Move(float val)
	// {
	// 	Position += Direction * val;

	// 	// aktualizacja
	// 	Update();
	// }
	
	void Jump(float val)
	{
		Position.y += val;
		Update();
	}

	// ----------------------------------------------
	// NOWE: zmiana polozenia uwzgledniajaca kolizje
	// ----------------------------------------------
	// void Move(float val)
	void Move(float val, std::vector<CSceneObject>& others)
	{

		// kopia polozenia
		glm::vec3 oldPosition = Position;

		// aktualizujemy polozenie
		Position += Direction * val;
		float y =  myGround->getAltitute(glm::vec2(Position.x, Position.z));
		float _y = myGround->getAltitute(glm::vec2(Position.x, Position.z + 6.0f));
		Position.y = y;
		if (y == 9999999.0) {
			Position = oldPosition;
			return;	
		}
		float cameraY = Position.y + 2.0f;
		if(cameraY < _y) {
			cameraY = _y + 1.0f;
		}
		CameraTranslate_x = Position.x;
		CameraTranslate_y = cameraY; 
		CameraTranslate_z = Position.z + 6.0f;
		
		bool isColliding = false;
		glm::vec3 objPosition;
		for(auto& other : others) {
			// wyświetlanie kolidera
			
			if (this->Collider->isCollision(other.Collider->Radius, other.Position)) {
				objPosition = other.Position;
				isColliding = true;
				break; 
			}
		}

		if (isColliding) {
			glm::vec3 directionToObjectToPlayer = glm::normalize(oldPosition - objPosition);
			glm::vec3 newDirection = glm::normalize(Direction + directionToObjectToPlayer);

			Position += newDirection * val;
			return;

		}
		// aktualizacja
		Update();
	}


	// zmiana orientacji obiektu
	void Rotate(float angle)
	{
		Angle += angle;
		Direction.x = cos(Angle);
		Direction.z = -sin(Angle);

		// aktualizacja
		Update();
	}
};


#endif

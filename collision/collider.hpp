#ifndef __CCOLLIDER
#define __CCOLLIDER

// ----------------------------------------------------------------
// Klasa abstrakcyjna obiektu kolizyjnego
// ----------------------------------------------------------------
class CCollider
{

public:

	float Radius;
	CCollider(float r = 0.0f) : Radius(r) { }

	// zwraca true jesli koliduje z _other
	virtual bool isCollision(CCollider *_other) const = 0;
	virtual bool isCollision(float Radius, glm::vec3 Position) const = 0;


};



// ----------------------------------------------------------------
// Klasy dziedziczace po Collider, ktore realizuja
// konkretna implementacje obiektu kolizyjnego
// ----------------------------------------------------------------
class CSphereCollider : public CCollider
{

public:

	// ----------------------------------------------
	// polozenie obiektu UWAGA! zauwaz, ze
	// przechowujemy wskaznik a nie kopie, aby nie
	// dublowac danych i nie przejmowac ew. animacja
	// poruszaniem sie obiektu
	// ----------------------------------------------
	glm::vec3 *Position = NULL;

	// Promien sfery
	float Radius;


	CSphereCollider(glm::vec3 *_pos, float _r)
		: CCollider(_r), Position(_pos),  Radius(_r)
	{ }

	virtual bool isCollision(CCollider *_other) const
	{
		// sprawdzanie czy drugi kolider jest tego samego typu
		//if (dynamic_cast<CSphereCollider*> (_other) == NULL)
		//	printf("Wrong type!\n");

		CSphereCollider  *other = dynamic_cast<CSphereCollider *>(_other);

		float distance = glm::distance(*this->Position, *other->Position);

		// glowny test kolizji sferycznej
		if (distance < this->Radius + other->Radius)
		{
			return true;
		}
		return false;
	}


	virtual bool isCollision(float Radius, glm::vec3 Position) const
	{
		// sprawdzanie czy drugi kolider jest tego samego typu
		//if (dynamic_cast<CSphereCollider*> (_other) == NULL)
		//	printf("Wrong type!\n");

		float distance = glm::distance(*this->Position, Position);
		
		// glowny test kolizji sferycznej
        if (distance < this->Radius + Radius)
        {
            return true;
        }
        return false;
	}

};

// ----------------------------------------------------------------
// Axis Aligend Bounding Box
// Wersja uproszczona (Cube)
// ----------------------------------------------------------------
class CAABBCollider : public CCollider
{

public:

	// wskaznik do polozenia obiektu
	glm::vec3 *Position;

	// Dlugosc kazdego boku boxa
	float Length;

	CAABBCollider (glm::vec3 *_pos, float _length)
		: Position(_pos), Length(_length)
	{ }


	// ----------------------------------------------
	virtual bool isCollision(CCollider *_other) const
	{
		CAABBCollider  *other = dynamic_cast<CAABBCollider *>(_other);

		glm::vec3 minA = get_min();
		glm::vec3 maxA = get_max();

		glm::vec3 minB = other->get_min();
		glm::vec3 maxB = other->get_max();


		return 	(minA.x <= maxB.x && maxA.x >= minB.x) &&
        		(minA.y <= maxB.y && maxA.y >= minB.y) &&
        		(minA.z <= maxB.z && maxA.z >= minB.z) ;
	}

	// ----------------------------------------------
	inline glm::vec3 get_min() const { return *Position - glm::vec3(Length); }
	inline glm::vec3 get_max() const { return *Position + glm::vec3(Length); }

	// ----------------------------------------------
	bool isPointInsideAABB(glm::vec3 _point) const
	{
		glm::vec3 min = get_min();
		glm::vec3 max = get_max();

  		return 	(_point.x >= min.x && _point.x <= max.x) &&
    	     	(_point.y >= min.y && _point.y <= max.y) &&
         		(_point.z >= min.z && _point.z <= max.z) ;
	}


};


#endif

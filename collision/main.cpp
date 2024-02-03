// ------------------------------------------------------
// Programowanie grafiki 3D w OpenGL / UG
// ------------------------------------------------------
// Przyklad implementacji klasy CSceneObject
// obslugujacej obiekty CMesh wraz z koliderem CCollider
// ------------------------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "obj_loader.hpp"
#include "utilities.hpp"
#include "text-ft.hpp"

// Klasa CColider i pochodne
#include "collider.hpp"

// Klasa CMesh i CSceneObject
// dziedziczaca po CMesh
#include "mesh.hpp"
#include "missle.hpp"

// Klasa CPlayer
#include "ground.hpp"
#include "player.hpp"


glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);


const int KEY_COUNT = 256;
bool keyStates[KEY_COUNT];


// potok
GLuint idProgram;

// ---------------------------------------
// NOWE: Obecnie mamy 3 rodzaje obiektow:
// ---------------------------------------

// A. Obiekty bez testu kolizji (zwykle meshe)
CGround ground;

// B. Obiekty z obsluga testu kolizji (dziedzicza
// po CMesh ale maja dodatkowa skladowa CColider)
std::vector<CSceneObject> stones;
std::vector<CSceneObject> aliens;


// C. Obiekt postaci, ktory udostepnia metody
// poruszania sie po scenie wraz z testem kolizji
CPlayer myPlayer;

CMissle missle;

int frame = 0;
int fps = 0;
int currentTime = 0, previousTime = 0;

int score;

// ---------------------------------------
void DisplayScene()
{
	__CHECK_FOR_ERRORS

	// Obliczanie macierzy widoku
	matView = UpdateViewMatrix(myPlayer.Position, myPlayer.Direction);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );


	matModel = glm::mat4x4( 1.0 );


    glUseProgram( idProgram );

		// Wyslanie macierzy rzutowania
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(matProj) );
		glUniformMatrix4fv( glGetUniformLocation( idProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(matView) );


		// GROUND
		ground.Draw();

		// STONE
		for(auto& stone : stones)
		{
			stone.Draw();
		}
		for(auto& alien : aliens)
		{
			alien.Draw();
		}

		// Rendering postaci, ktora sie poruszamy
		myPlayer.Draw();

		missle.Draw();


    glUseProgram( 0 );

	char fpsTxt[255];
    sprintf(fpsTxt, "fps: %d", fps);

    RenderText(fpsTxt, 10, 480, 0.7f, glm::vec3(1, 0.7f, 0.0f));

	char scoreTxt[255];
	sprintf(scoreTxt, "Score: %d", score);

	int textX = 10;
	int textY = 10;
	float scale = 1.2f;
	glm::vec3 color = glm::vec3(0.1, 0.1, 1.0f);
	float width = 0.5f;
    for (GLfloat i = -width; i <= width; i += width) {
        for (GLfloat j = -width; j <= width; j += width) {
            RenderText(scoreTxt, textX + i, textY + j, scale, color);
        }
    }

	glutSwapBuffers();
}



// ---------------------------------------------------
void Initialize()
{
	InitText("arial.ttf", 16);
	// Tylko raz w programie (na potrzeby tekstur)
	stbi_set_flip_vertically_on_load(true);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	// ustawienia openGL
	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );


	// Ustawienie domyslnego odsuniecia kamery od polozenia (0,0,0)
	CameraTranslate_x = 0.0;
	CameraTranslate_y = 0.0;
	CameraTranslate_z = -8.0;


	// potok
	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );



	// A. Inicjalizacja obiektow bez colliderow
	// ground.Init("assets/scene-large.obj", "assets/chess.jpg");
	ground.Init();

	// B. Inicjalizacja obiektu z koliderem
	int numberOfStones = 1; 
	int numberOfAliens = 1;
    for(int i = 0; i < numberOfStones; ++i)
    {
        CSceneObject stone;
        // float x = (rand() % 16000 / 100.0f) - 80;
        // float z = (rand() % 16000 / 100.0f) - 80;
		float x = -5.0f;
		float z = -5.0f;
        float y = ground.getAltitute(glm::vec2(x, z)); 
        stone.Init("assets/stone.obj", "assets/rock.jpg");
        stone.SetPosition(glm::vec3(x, y, z));
        stone.Collider = new CSphereCollider(&stone.Position, 1.5f);
        stones.push_back(stone);
    }

	for(int i = 0; i < numberOfAliens; ++i)
	{
		CSceneObject alien;
		// float x = (rand() % 16000 / 100.0f) - 80;
		// float z = (rand() % 16000 / 100.0f) - 80;
		float x = 5.0f;
		float z = 5.0f;
		float y = ground.getAltitute(glm::vec2(x, z)) + 2.0f; 
		alien.Init("assets/alien.obj", "assets/alien.jpg");
		alien.SetPosition(glm::vec3(x, y, z));
		alien.Collider = new CSphereCollider(&alien.Position, 2.0f);
		aliens.push_back(alien);
	}

    // C. Inicjalizacja playera
    myPlayer.Init("assets/ufo.obj", "assets/ufo.jpg", &ground);
	myPlayer.Collider = new CSphereCollider(&myPlayer.Position, 0.7f);

	// D. Inicjalizacja pocisku
	missle.Init("assets/sphere.obj", "assets/missle.jpg");
	missle.Collider = new CSphereCollider(&missle.Position, 0.5f);



}


// ---------------------------------------
void Reshape( int width, int height )
{
	glViewport( 0, 0, width, height );
	matProj = glm::perspectiveFov(glm::radians(60.0f), (float)width, (float)height, 0.1f, 100.f );
}

void PlayerJump(int frame)
{
	float jump_vec = 0.1;
	if (frame < 20)
	{
		myPlayer.Jump(jump_vec);
		glutTimerFunc(1000/60, PlayerJump, frame+1);
	}
	else if (frame < 40)
	{
		myPlayer.Jump(-jump_vec);
		glutTimerFunc(1000/60, PlayerJump, frame+1);
	}

}

void InitializeKeyStates() {
    for (int i = 0; i < KEY_COUNT; ++i) {
        keyStates[i] = false;
    }
}

void KeyboardUp(unsigned char key, int x, int y) {
    if(key != 'q') {
		keyStates[key] = false;
	} 
	
}

// --------------------------------------------------------------
void Keyboard( unsigned char key, int x, int y )
{
	keyStates[key] = true;
}

// ---------------------------------------------------
void Animation(int frame)
{
	++frame;

    float move_vec = 0.1;

	if(keyStates['w']) {
		myPlayer.Move(move_vec, stones);
	}
	if(keyStates['s']) {
		myPlayer.Move(-move_vec, stones);
	}
	if(keyStates['d']) {
		myPlayer.Rotate(-(move_vec/2));
	}
	if(keyStates['a']) {
		myPlayer.Rotate(move_vec/2);
	}
	if(keyStates[32]) {
		PlayerJump(0);
		keyStates[32] = false;
	}
	if(keyStates[27]) {
		glutLeaveMainLoop();
	}
	if(keyStates['q']) {
		int con = missle.SetPosition(myPlayer.Position, myPlayer.Angle, 0.5, aliens);
		if (con == 1) {
			keyStates['q'] = false;
		} else if (con == 2) {
			score += 100;
			keyStates['q'] = false;
		}
	}
	for(auto& alien : aliens)
	{
		alien.Animate(frame);
	}
	currentTime = glutGet(GLUT_ELAPSED_TIME);
    int timeInterval = currentTime - previousTime;
    if (timeInterval > 1000) {
        fps = frame / (timeInterval / 1000.0f);
        previousTime = currentTime;
        frame = 0;
    }
	glutTimerFunc(1000/60, Animation, frame);
	glutPostRedisplay();
}


// ---------------------------------------------------
int main( int argc, char *argv[] )
{
	// GLUT
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitWindowSize( 500, 500 );
	glutCreateWindow( "OpenGL" );

	// GLEW
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		printf("GLEW Error\n");
		exit(1);
	}

	// OpenGL
	if( !GLEW_VERSION_3_2 )
	{
		printf("Brak OpenGL 3.2!\n");
		exit(1);
	}


	Initialize();
	glutDisplayFunc( DisplayScene );
	glutReshapeFunc( Reshape );
	glutMouseFunc( MouseButton );
	glutMotionFunc( MouseMotion );
	glutMouseWheelFunc( MouseWheel );
	glutKeyboardFunc( Keyboard );
    glutKeyboardUpFunc(KeyboardUp);
	glutSpecialFunc( SpecialKeys );
	glutTimerFunc(1000/60, Animation, 0);

	glutMainLoop();


	exit(0);
}

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

#include "collider.hpp"

#include "mesh.hpp"
#include "missle.hpp"

#include "ground.hpp"
#include "player.hpp"
#include "skybox.hpp"

glm::mat4x4 matProj;
glm::mat4x4 matView;
glm::mat4x4 matModel = glm::mat4(1.0);

int windowWidth = 500;
int windowHeight = 500;

const int KEY_COUNT = 256;
bool keyStates[KEY_COUNT];

GLuint idProgram;
GLuint multiProgram;

CGround ground;

std::vector<CSceneObject> stones;
std::vector<CSceneObject> aliens;

CMesh meteor;

CPlayer myPlayer;

CMissle missle;

int frame = 0;
int fps = 0;
int currentTime = 0, previousTime = 0;
int time = 0;

int score;

GLuint minimapFBO, minimapTexture, minimapShaderProgram;
int minimapWidth = 100; // Rozmiar minimapy
int minimapHeight = 100;
unsigned int minimapVBO, minimapVAO, minimapEBO;
float vertices[] = {
    1.0f,  1.0f,     1.0f, 1.0f, 
    1.0f,  0.6f,     1.0f, 0.0f, 
    0.6f,  0.6f,     0.0f, 0.0f, 
    0.6f,  1.0f,     0.0f, 1.0f 
};

unsigned int indices[] = {
    0, 1, 3, 
    1, 2, 3  
};

void RenderMinimap() {
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);
	glViewport(0, 0, minimapWidth, minimapHeight);
	glClear(GL_COLOR_BUFFER_BIT);

	glm::vec3 cameraPos = myPlayer.Position;
	glm::vec3 minimapCameraPos = cameraPos + glm::vec3(0, 10, 0);
	glm::vec3 target = cameraPos;

	matView = glm::lookAt(minimapCameraPos, target, glm::vec3(0, 0, -1));

	matModel = glm::mat4x4(1.0);
	glUseProgram(idProgram);
	glUniformMatrix4fv(glGetUniformLocation(idProgram, "matProj"), 1, GL_FALSE, glm::value_ptr(matProj));
	glUniformMatrix4fv(glGetUniformLocation(idProgram, "matView"), 1, GL_FALSE, glm::value_ptr(matView));
	ground.Draw();
	for (auto& stone : stones) {
		stone.Draw();
	}
	for (auto& alien : aliens) {
		alien.Draw();
	}
	myPlayer.Draw();
	missle.Draw();
	glUseProgram(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DisplayMinimap() {
	glUseProgram(minimapShaderProgram);
	glBindVertexArray(minimapVAO);
	glBindTexture(GL_TEXTURE_2D, minimapTexture);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	glUseProgram(0);
}

void DisplayScene()
{
	__CHECK_FOR_ERRORS

	RenderMinimap();
	glViewport(0, 0, windowWidth, windowHeight);
	matView = UpdateViewMatrix(myPlayer.Position, myPlayer.Direction);

	glm::vec3 cameraPos = ExtractCameraPos(matView);
	float y = ground.getAltitute(glm::vec2(cameraPos.x, cameraPos.z));
	if (cameraPos.y < y + 0.5) {
		cameraPos.y = y + 0.5;
		matView = glm::lookAt(cameraPos, myPlayer.Position, glm::vec3(0, 1, 0));
	}

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	matModel = glm::mat4x4( 1.0 );

    glUseProgram( idProgram );

	glUniformMatrix4fv( glGetUniformLocation( idProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(matProj) );
	glUniformMatrix4fv( glGetUniformLocation( idProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(matView) );
	
	ground.Draw();

	for(auto& stone : stones)
	{
		stone.Draw();
	}
	for(auto& alien : aliens)
	{
		alien.Draw();
	}

	myPlayer.Draw();

	missle.Draw();

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, SkyBox_Texture);
	glUniform1i(glGetUniformLocation(idProgram, "tex_skybox"), 1);

	glUseProgram(multiProgram);
	glUniformMatrix4fv( glGetUniformLocation( multiProgram, "matProj" ), 1, GL_FALSE, glm::value_ptr(matProj) );
	glUniformMatrix4fv( glGetUniformLocation( multiProgram, "matView" ), 1, GL_FALSE, glm::value_ptr(matView) );

	meteor.Draw();

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

	DisplayMinimap();
	
	DrawSkyBox(matProj, matView);

	glutSwapBuffers();
}

void InitializeMinimap() {
	minimapShaderProgram = glCreateProgram();
	glAttachShader(minimapShaderProgram, LoadShader(GL_VERTEX_SHADER, "minimap-vertex.glsl"));
	glAttachShader(minimapShaderProgram, LoadShader(GL_FRAGMENT_SHADER, "minimap-fragment.glsl"));
	LinkAndValidateProgram(minimapShaderProgram);

	glGenFramebuffers(1, &minimapFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, minimapFBO);

	glGenTextures(1, &minimapTexture);
	glBindTexture(GL_TEXTURE_2D, minimapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, minimapWidth, minimapHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, minimapTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer not complete!\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glGenVertexArrays(1, &minimapVAO);
	glGenBuffers(1, &minimapVBO);
	glGenBuffers(1, &minimapEBO);

	glBindVertexArray(minimapVAO);
	glBindBuffer(GL_ARRAY_BUFFER, minimapVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, minimapEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

void Initialize()
{
	InitText("arial.ttf", 16);
	stbi_set_flip_vertically_on_load(true);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glEnable( GL_DEPTH_TEST );
	glClearColor( 0.5f, 0.5f, 0.5f, 1.0f );

	CameraTranslate_x = 0.0;
	CameraTranslate_y = 0.0;
	CameraTranslate_z = -8.0;

	idProgram = glCreateProgram();
	glAttachShader( idProgram, LoadShader(GL_VERTEX_SHADER, "vertex.glsl"));
	glAttachShader( idProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( idProgram );

	multiProgram = glCreateProgram();
	glAttachShader( multiProgram, LoadShader(GL_VERTEX_SHADER, "multi-vertex.glsl"));
	glAttachShader( multiProgram, LoadShader(GL_FRAGMENT_SHADER, "fragment.glsl"));
	LinkAndValidateProgram( multiProgram );

	ground.Init();

	int numberOfStones = 20; 
	int numberOfAliens = 10;
    for(int i = 0; i < numberOfStones; ++i)
    {
        CSceneObject stone;
        float x = (rand() % 16000 / 100.0f) - 80;
        float z = (rand() % 16000 / 100.0f) - 80;
        float y = ground.getAltitute(glm::vec2(x, z)); 
        stone.Init("assets/stone.obj", "assets/rock.jpg");
        stone.SetPosition(glm::vec3(x, y, z));
        stone.Collider = new CSphereCollider(&stone.Position, 1.5f);
        stones.push_back(stone);
    }

	for(int i = 0; i < numberOfAliens; ++i)
	{
		CSceneObject alien;
		float x = (rand() % 16000 / 100.0f) - 80;
		float z = (rand() % 16000 / 100.0f) - 80;
		float y = ground.getAltitute(glm::vec2(x, z)) + 2.0f; 
		alien.Init("assets/alien.obj", "assets/alien.jpg");
		alien.SetPosition(glm::vec3(x, y, z));
		alien.Collider = new CSphereCollider(&alien.Position, 2.0f);
		aliens.push_back(alien);
	}

    myPlayer.Init("assets/ufo.obj", "assets/ufo.jpg", &ground);
	myPlayer.Collider = new CSphereCollider(&myPlayer.Position, 0.7f);

	missle.Init("assets/sphere.obj", "assets/missle.jpg");
	missle.Collider = new CSphereCollider(&missle.Position, 0.5f);

	InitializeMinimap();
	CreateSkyBox();

	__CHECK_FOR_ERRORS
	meteor.Init("assets/meteor.obj", "assets/rock.jpg", false, true);

	}


// ---------------------------------------
void Reshape( int width, int height )
{
	windowWidth = width;
	windowHeight = height;
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

void Keyboard( unsigned char key, int x, int y )
{
	keyStates[key] = true;
}

void Animation(int frame)
{

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
		alien.Animate(time);
	}
	++frame;
	currentTime = glutGet(GLUT_ELAPSED_TIME);
    int timeInterval = currentTime - previousTime;
    if (timeInterval > 1000) {
        fps = frame / (timeInterval / 1000.0f);
        previousTime = currentTime;
        frame = 0;
    }
	time += 1;
	glutTimerFunc(1000/60, Animation, frame);
	glutPostRedisplay();
}

int main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
	glutInitContextVersion( 3, 2 );
	glutInitContextProfile( GLUT_CORE_PROFILE );
	glutInitWindowSize( windowWidth, windowHeight );
	glutCreateWindow( "OpenGL" );

	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if( GLEW_OK != err )
	{
		printf("GLEW Error\n");
		exit(1);
	}

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

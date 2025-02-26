
#include <windows.h>        //	Utilizarea functiilor de sistem Windows (crearea de ferestre, manipularea fisierelor si directoarelor);
#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;
#include "glm/glm.hpp"		//	Bibloteci utilizate pentru transformari grafice;
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include<stack>
#include <ctime>
#include <vector>
#include "objloader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
//
//#include <SDL3/SDL.h>
//#include <SDL3/SDL_main.h>


//  Identificatorii obiectelor de tip OpenGL;
GLuint
VaoId,
VboId,
VaoIdCat, 
VboIdCat,
starVAO,
starVBO,
asteroidsVAO,
asteroidsVBO,
EboId,
EboIdStars,
EboIdAsteroids,
ProgramId,
viewModelLocation,
projLocation,
codColLocation,
nrVertLocation,
myMatrixLocation,
viewLocation,
objectColorLoc, lightColorLoc, lightPosLoc, viewPosLoc;
;

float const PI = 3.141592f;

// Elemente pentru reprezentarea suprafetei
// (1) intervalele pentru parametrii considerati (u si v)
float const U_MIN = -PI / 2, U_MAX = PI / 2, V_MIN = 0, V_MAX = 2 * PI;
// (2) numarul de paralele/meridiane, de fapt numarul de valori ptr parametri
int const NR_PARR = 10, NR_MERID = 20;
// (3) pasul cu care vom incrementa u, respectiv v
float step_u = (U_MAX - U_MIN) / NR_PARR, step_v = (V_MAX - V_MIN) / NR_MERID;

// alte variabile
int codCol;
float radius = 50;
int index, index_aux;

//	Dimensiunile ferestrei de afisare;
GLfloat
winWidth = 1400, winHeight = 600;

// Variabila pentru timpul scurs
float timeElapsed;

//	Elemente pentru matricea de vizualizare;
float 
obsX = 0.0, obsY = 0.0, obsZ = 300.f,
refX = 0.0f, refY = 0.0f,
refZ = -100.f,
vX = 0.0;
//	Elemente pentru matricea de proiectie;
float xMin = -700.f, xMax = 700.f, yMin = -300.f, yMax = 300.f,
zNear = 100.f, zFar = 500.f
, width = 1400.f, height = 600.f, fov = 60.f * PI / 180;

//	Vectori pentru matricea de vizualizare;
glm::vec3
obs, pctRef, vert;

//	Variabile catre matricile de transformare;
glm::mat4
view, projection,
translateSystem,
rotateSun,
scalePlanet, rotatePlanetAxis, rotatePlanet, translatePlanet;

// Stiva de matrice - inglobeaza matricea de modelare si cea de vizualizare
std::stack<glm::mat4> mvStack;

const int numStars = 200; // Number of stars
const int numAsteroids = 150;
float starPositions[numStars][3]; // Store positions for stars

//cat variables
// Variabile globale
int nrVertices, numIndices;
std::vector<glm::vec3> vertices;
std::vector<glm::vec2> uvs;
std::vector<glm::vec3> normals;
glm::mat4 myMatrix;
float 
vY = 0.0f, vZ = 1.0f;
float alpha = 0.0f, beta = 0.0f, dist = 6.0f, incrAlpha1 = 0.01, incrAlpha2 = 0.01;
float dNear = 4.f;

void ProcessNormalKeys(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'l':			//	Apasarea tastelor `l` si `r` modifica pozitia verticalei in planul de vizualizare;
		vX += 0.1;
		break;
	case 'r':
		vX -= 0.1;
		break;
	case '+':			//	Apasarea tastelor `+` si `-` schimba pozitia observatorului (se departeaza / aproprie);
		obsZ -= 10;
		break;
	case '-':
		obsZ += 10;
		break;
	}
	if (key == 27)
		exit(0);
}

void ProcessSpecialKeys(int key, int xx, int yy)
{
	switch (key)				//	Procesarea tastelor 'LEFT', 'RIGHT', 'UP', 'DOWN';
	{							//	duce la deplasarea observatorului pe axele Ox si Oy;
	case GLUT_KEY_LEFT:
		obsX -= 20;
		break;
	case GLUT_KEY_RIGHT:
		obsX += 20;
		break;
	case GLUT_KEY_UP:
		obsY += 20;
		break;
	case GLUT_KEY_DOWN:
		obsY -= 20;
		break;
	}
}

//  Crearea si compilarea obiectelor de tip shader;
void CreateShaders(void)
{
	ProgramId = LoadShaders("08_01_Shader.vert", "08_01_Shader.frag");
	glUseProgram(ProgramId);
}

//  Se initializeaza un vertex Buffer Object (VBO) pentru tranferul datelor spre memoria placii grafice (spre shadere);
//  In acesta se stocheaza date despre varfuri (coordonate, culori, indici, texturare etc.);

void CreateVBOSfera(void)
{
	// varfurile 
	// (4) Matricele pentru varfuri, culori, indici
	glm::vec4 Vertices[(NR_PARR + 1) * NR_MERID];
	glm::vec3 Colors[(NR_PARR + 1) * NR_MERID];
	glm::vec3 Normals[(NR_PARR + 1) * NR_MERID];
	GLushort Indices[2 * (NR_PARR + 1) * NR_MERID + 4 * (NR_PARR + 1) * NR_MERID];

	numIndices = sizeof(Indices) / sizeof(GLushort);


	for (int merid = 0; merid < NR_MERID; merid++)
	{
		for (int parr = 0; parr < NR_PARR + 1; parr++)
		{
			// implementarea reprezentarii parametrice 
			float u = U_MIN + parr * step_u; // valori pentru u si v
			float v = V_MIN + merid * step_v;
			float x_vf = radius * cosf(u) * cosf(v); // coordonatele varfului corespunzator lui (u,v)
			float y_vf = radius * cosf(u) * sinf(v);
			float z_vf = radius * sinf(u);

			// identificator ptr varf; coordonate + culoare + indice la parcurgerea meridianelor
			index = merid * (NR_PARR + 1) + parr;
			Vertices[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0);
			//Colors[index] = glm::vec3(0.1f + sinf(u), 0.1f + cosf(v), 0.1f + -1.5 * sinf(u));
			Colors[index] = glm::vec3(0.8f + 0.2f * sinf(u), 0.2f + 0.1f * cosf(v), 0.0);
			Normals[index] = glm::vec3(x_vf, y_vf, z_vf);

			Indices[index] = index;

			// indice ptr acelasi varf la parcurgerea paralelelor
			index_aux = parr * (NR_MERID)+merid;
			Indices[(NR_PARR + 1) * NR_MERID + index_aux] = index;

			// indicii pentru desenarea fetelor, pentru varful curent sunt definite 4 varfuri
			if ((parr + 1) % (NR_PARR + 1) != 0) // varful considerat sa nu fie Polul Nord
			{
				int AUX = 2 * (NR_PARR + 1) * NR_MERID;
				int index1 = index; // varful v considerat
				int index2 = index + (NR_PARR + 1); // dreapta lui v, pe meridianul urmator
				int index3 = index2 + 1;  // dreapta sus fata de v
				int index4 = index + 1;  // deasupra lui v, pe acelasi meridian
				if (merid == NR_MERID - 1)  // la ultimul meridian, trebuie revenit la meridianul initial
				{
					index2 = index2 % (NR_PARR + 1);
					index3 = index3 % (NR_PARR + 1);
				}
				Indices[AUX + 4 * index] = index1;  // unele valori ale lui Indices, corespunzatoare Polului Nord, au valori neadecvate
				Indices[AUX + 4 * index + 1] = index2;
				Indices[AUX + 4 * index + 2] = index3;
				Indices[AUX + 4 * index + 3] = index4;
			}
		}
	};


	// Generate UV texture coordinates
	glm::vec2 UVs[(NR_PARR + 1) * NR_MERID];
	for (int merid = 0; merid < NR_MERID; merid++)
	{
		for (int parr = 0; parr < NR_PARR + 1; parr++)
		{
			float u = parr * 1.0f / NR_PARR; // Texture u-coordinate
			float v = merid * 1.0f / NR_MERID; // Texture v-coordinate
			UVs[merid * (NR_PARR + 1) + parr] = glm::vec2(u, v);
		}
	}


	//// generare VAO/buffere
	//glGenBuffers(1, &VboId); // atribute
	//glGenBuffers(1, &EboId); // indici

	//// legare+"incarcare" buffer
	//glBindBuffer(GL_ARRAY_BUFFER, VboId);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Colors) + sizeof(Normals), NULL, GL_STATIC_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Normals), Normals);
	//glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Normals), sizeof(Colors), Colors);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	//// atributele; 
	//glEnableVertexAttribArray(0); // atributul 0 = pozitie
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	//glEnableVertexAttribArray(1); // atributul 1 = normala
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices)));

	//glEnableVertexAttribArray(2); // atributul 2 = culoare
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices) + sizeof(Normals)));



	// Generate VAO and VBOs
	glGenBuffers(1, &VboId); // Vertex data buffer
	glGenBuffers(1, &EboId); // Index data buffer

	// Bind and upload data to the buffers
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);

	// Allocate memory for all attributes
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Normals) + sizeof(Colors) + sizeof(UVs), NULL, GL_STATIC_DRAW);

	// Upload data for each attribute
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(Vertices), Vertices);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices), sizeof(Normals), Normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Normals), sizeof(Colors), Colors);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(Vertices) + sizeof(Normals) + sizeof(Colors), sizeof(UVs), UVs);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

	// Configure attributes
	glEnableVertexAttribArray(0); // Position attribute
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glEnableVertexAttribArray(1); // Normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices)));

	glEnableVertexAttribArray(2); // Color attribute
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Normals)));

	glEnableVertexAttribArray(3); // Texture coordinate attribute
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(sizeof(Vertices) + sizeof(Normals) + sizeof(Colors)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (GLvoid*)(sizeof(Vertices) + sizeof(Normals) + sizeof(Colors)));


	// Unbind VAO and buffers
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);


}

void CreateVAOCat() {
	// Crearea și legarea unui VAO
	glGenVertexArrays(1, &VaoIdCat);
	glBindVertexArray(VaoIdCat);

	// Crearea și umplerea unui VBO
	glGenBuffers(1, &VboIdCat);
	glBindBuffer(GL_ARRAY_BUFFER, VboIdCat);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec3), &vertices[0]);
	glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), normals.size() * sizeof(glm::vec3), &normals[0]);

	// Configurarea atributelor de poziție și normale
	glEnableVertexAttribArray(0); // Poziție
	glVertexAttribPointer(0,  3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	glEnableVertexAttribArray(1); // Normale
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(glm::vec3)));

	glEnableVertexAttribArray(2); // culoare
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)(vertices.size() * sizeof(glm::vec3) + normals.size() * sizeof(glm::vec3)));

	// Debind VAO pentru a evita modificări neintenționate
	glBindVertexArray(0);
}



void CreateVAOStars() {

	const int numStars = 500;

	glm::vec3 starPositions[numStars];
	glm::vec3 starColors[numStars];
	GLushort starIndices[numStars];

	for (int i = 0; i < numStars; ++i) {
		// Assign random positions
		starPositions[i] = glm::vec3(
			(rand() % 1200 - 600), // X: [-600, 600]
			(rand() % 1000 - 500),  // Z: [-500, 500]
			(rand() % 500 - 250)  // Z: [-250, 250]
		);

		//Assign random colors
		starColors[i] = glm::vec3(
			static_cast<float>(rand()) / RAND_MAX,  // R
			static_cast<float>(rand()) / RAND_MAX,  // G
			static_cast<float>(rand()) / RAND_MAX   // B
		);

		// Each star has its own index
		starIndices[i] = static_cast<GLushort>(i);
	}

	// Generate the VAO
	glGenVertexArrays(1, &starVAO);
	glBindVertexArray(starVAO);

	// Generate the VBO for positions and colors
	GLuint starVBO, colorVBO;
	glGenBuffers(1, &starVBO);
	glGenBuffers(1, &colorVBO);

	// Upload star positions to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, starVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starPositions), starPositions, GL_STATIC_DRAW);

	// Define the layout of the vertex position data
	glEnableVertexAttribArray(0); // Attribute 0 = Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Upload star colors to the GPU
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(starColors), starColors, GL_STATIC_DRAW);

	// Define the layout of the vertex color data
	glEnableVertexAttribArray(2); // Attribute 2 = Color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Generate and bind the EBO
	glGenBuffers(1, &EboIdStars);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdStars);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(starIndices), starIndices, GL_STATIC_DRAW);

	// Unbind VAO 
	glBindVertexArray(0);
}

//trebuie in shader pus pt sclaes???? sunt porsti indicii l enable 
void CreateVAOAsteroidRing() {
	const float radius = 100.0f;    // Radius of the ring
	const float offset = 5.0f;     // Random offset for natural look

	glm::vec3 asteroidPositions[numAsteroids];
	glm::vec3 asteroidColors[numAsteroids];
	float asteroidScales[numAsteroids]; // Array for random scales
	GLushort asteroidIndices[numAsteroids];

	// Generate circular positions and scales for the asteroid ring
	for (int i = 0; i < numAsteroids; ++i) {
		float angle = (float)i / (float)numAsteroids * 360.0f; // Spread evenly around the circle
		float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset; // Random offset
		float x = sin(glm::radians(angle)) * radius + displacement;
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float y = displacement * 0.4f; // Smaller variation in height
		displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
		float z = cos(glm::radians(angle)) * radius + displacement;

		asteroidPositions[i] = glm::vec3(x, y, z);

		// Assign random sizes (scale between 0.5 and 2.0)
		asteroidScales[i] = 0.5f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 5.5f));

		asteroidColors[i] = glm::vec3(1.0f, 1.0f, 1.0f); // Default white
		asteroidIndices[i] = static_cast<GLushort>(i);
	}

	// Generate the VAO
	glGenVertexArrays(1, &asteroidsVAO);
	glBindVertexArray(asteroidsVAO);

	// VBOs for positions, colors, and scales
	GLuint positionVBO, colorVBO, scaleVBO;
	glGenBuffers(1, &positionVBO);
	glGenBuffers(1, &colorVBO);
	glGenBuffers(1, &scaleVBO);

	// Upload asteroid positions
	glBindBuffer(GL_ARRAY_BUFFER, positionVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(asteroidPositions), asteroidPositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0); // Attribute 0 = Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Upload asteroid colors
	glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(asteroidColors), asteroidColors, GL_STATIC_DRAW);
	glEnableVertexAttribArray(2); // Attribute 1 = Color
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Upload asteroid scales
	glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(asteroidScales), asteroidScales, GL_STATIC_DRAW);
	glEnableVertexAttribArray(3); // Attribute 2 = Scale
	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

	// Generate and bind the EBO
	glGenBuffers(1, &EboIdAsteroids);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboIdAsteroids);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(asteroidIndices), asteroidIndices, GL_STATIC_DRAW);

	// Unbind VAO
	glBindVertexArray(0);
}


//  Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(2);

	//  Stergerea bufferelor pentru VARFURI (Coordonate, Culori), INDICI;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);
	glDeleteBuffers(1, &starVBO);
	glDeleteVertexArrays(1, &starVAO);
	glDeleteBuffers(1, &asteroidsVBO);
	glDeleteVertexArrays(1, &asteroidsVAO);
	glDeleteBuffers(1, &VaoIdCat);
	glDeleteVertexArrays(1, &VboIdCat);

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}



GLuint LoadTexture(const char* filePath) {
	int width, height, nrChannels;
	unsigned char* data = stbi_load(filePath, &width, &height, &nrChannels, 0);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	return textureID;
}


void InitAudio() {
	//if (SDL_Init(SDL_INIT_AUDIO) < 0) { // Initialize SDL 3 Audio
	//	exit(1);
	//}

	//if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) { // Initialize SDL Mixer
	//	exit(1);
	//}
}

void PlayMusic(const char* filepath) {
	//Mix_Music* music = Mix_LoadMUS(filepath); // Load music file
	//if (!music) {
	//	std::cerr << "Failed to load music: " << Mix_GetError() << std::endl;
	//	exit(1);
	//}

	//Mix_PlayMusic(music, -1); // Play the music (-1 = loop indefinitely)
}


void CleanAudio() {
	/*Mix_CloseAudio();
	SDL_Quit();*/
}



//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);		//  Culoarea de fond a ecranului;
	bool model = loadOBJ("bigCat.obj", vertices, uvs, normals);

	nrVertices = vertices.size();

	CreateVAOCat();
	CreateVBOSfera();								//  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateVAOStars();
	CreateVAOAsteroidRing();
	CreateShaders();							//  Initilizarea shaderelor;


	//	Instantierea variabilelor uniforme pentru a "comunica" cu shaderele;
	viewModelLocation = glGetUniformLocation(ProgramId, "viewModel");
	projLocation = glGetUniformLocation(ProgramId, "projection");
	codColLocation = glGetUniformLocation(ProgramId, "codCol");
	viewLocation = glGetUniformLocation(ProgramId, "view");

	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	//lumina locatii ptr shader
	objectColorLoc = glGetUniformLocation(ProgramId, "objectColor");
	lightColorLoc = glGetUniformLocation(ProgramId, "lightColor");
	lightPosLoc = glGetUniformLocation(ProgramId, "lightPos");
	viewPosLoc = glGetUniformLocation(ProgramId, "viewPos");
}


void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		//  Se curata ecranul OpenGL pentru a fi desenat noul continut (bufferul de culoare & adancime);
	glEnable(GL_DEPTH_TEST);                                //  Activarea testului de adancime

	// Variabila care indica timpul scurs de la initializare
	timeElapsed = glutGet(GLUT_ELAPSED_TIME);

	//	Matricea de vizualizare - actualizare
	obs = glm::vec3(obsX, obsY, obsZ); 	//Pozitia observatorului;
	refX = obsX; refY = obsY;
	pctRef = glm::vec3(refX, refY, refZ); 	//Pozitia punctului de referinta;
	vert = glm::vec3(vX, 1.0f, 0.0f); //Verticala din planul de vizualizare; 
	view = glm::lookAt(obs, pctRef, vert); //matricea de vizuzlizare

	projection = glm::infinitePerspective(fov, width / height, dNear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	// Matrice pentru miscarea obiectelor din sistem
	// Intregul sistem se deplaseaza prin translatie„
	translateSystem = glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.0));

	// Soarele se roteste in jurul propriei axe
	rotateSun = glm::rotate(glm::mat4(1.0f), -(float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	//rotateSun = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0, 1.0, 0.0));

	// Planeta se obtine scaland cubul initial
	scalePlanet = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5));
	// Planeta se roteste in jurul propriei axe
	rotatePlanetAxis = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0));
	// Planeta se roteste in jurul astrului central
	rotatePlanet = glm::rotate(glm::mat4(1.0f), (float)0.0003 * timeElapsed, glm::vec3(-0.1, 0.5, 0.0));
	// Planeta este translatata in raport cu astrul central
	translatePlanet = glm::translate(glm::mat4(1.0f), glm::vec3(100.0, 0.0, 0.0));


	// Desenarea primitivelor + manevrarea stivei de matrice 
	// Matricea de vizualizare este adaugata in varful stivei de matrice
	mvStack.push(view);                  // In varful stivei:   view 


	// 0) Pentru intregul sistem
	// Matrice de translatie pentru intregul sistem
	mvStack.top() *= translateSystem;     // In varful stivei:  view * translateSystem 
	mvStack.push(mvStack.top());         // Pe poz 2 a stivei: view * translateSystem 

	// 1) Pentru Soare (astrul central)
	 //Actualizare a matricei din varful stivei
	 //Rotatie in jurul axei proprii
	mvStack.top() *= rotateSun;         // In varful stivei:  view * translateSystem * rotateSun          

	// Transmitere matrice de deplasare a Soarelui catre shader, apoi eliminare din varful stivei
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));
	mvStack.pop();


	//	Desenarea propriu-zisa a obiectului 3D
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++)
	{
		if ((patr + 1) % (NR_PARR + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort)));
	}


	// 2) Pentru planeta

	// Actualizare a matricei din varful stivei
	// Rotatie in jurul Soarelui
	mvStack.top() *= rotatePlanet;		// In varful stivei:  view * translateSystem * rPl
	// Deplasare fata de centrul Soarelui
	mvStack.top() *= translatePlanet;   // In varful stivei:  view * translateSystem * rPl * tPl
	// Rotatie in jurul axei proprii
	mvStack.top() *= rotatePlanetAxis;  // In varful stivei:  view * translateSystem * rPl * tPl * rPlAx
	// Scalare (redimensionare obiect 3D)
	mvStack.top() *= scalePlanet;       // In varful stivei:  view * translateSystem * rPl * tPl * rPlAx * scPl

	// Transmitere matrice de deplasare a planetei catre shader, apoi eliminare din varful stivei
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	//	Desenarea propriu-zisa a obiectului 3D
	codCol = 2;  //	Regula de colorare este diferita;		 													
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++)
	{
		if ((patr + 1) % (NR_PARR + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort)));
	}

	//4) Pentru planeta

	mvStack.top() *= rotatePlanet;		// In varful stivei:  view * translateSystem * rPl
	mvStack.top() *= translatePlanet;   // In varful stivei:  view * translateSystem * rPl * tPl
	mvStack.top() *= rotatePlanetAxis;  // In varful stivei:  view * translateSystem * rPl * tPl * rPlAx
	mvStack.top() *= scalePlanet;       // In varful stivei:  view * translateSystem * rPl * tPl * rPlAx * scPl

	// Transmitere matrice de deplasare a planetei catre shader, apoi eliminare din varful stivei
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	//	Desenarea propriu-zisa a obiectului 3D
	codCol = 1;  //	Regula de colorare este diferita;		 													
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++)
	{
		if ((patr + 1) % (NR_PARR + 1) != 0) // nu sunt considerate fetele in care in stanga jos este Polul Nord
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort)));
	}


	//cat
	codCol = 4;
	glUniform1i(codColLocation, codCol);
	myMatrix =glm::scale(glm::mat4(1.0f), glm::vec3(0.8, 0.8, 0.8));

	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(myMatrix));

	//// Matricea de proiecție	
	projection = glm::infinitePerspective(fov, width / height, dNear);
	glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);

	//   // Legarea VAO și desenarea modelului
	glBindVertexArray(VaoIdCat);
	glDrawArrays(GL_TRIANGLES, 0, nrVertices);
	glBindVertexArray(0);

	//for stars
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	glBindVertexArray(starVAO);
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(view)); // Matrice de vizualizare
	glPointSize(5.0f); // Dimensiunea punctelor
	glDrawArrays(GL_POINTS, 0, numStars);
	glBindVertexArray(0);


	//lumina
	// variabile uniforme pentru iluminare
	codCol = 1;
	glUniform3f(objectColorLoc, 0.0f, 0.0f, 1.0f);
	glUniform3f(lightColorLoc, 0.0f, 0.0f, 1.0f);
	glUniform3f(lightPosLoc, 0.f, 0.f, 70.f); //pozitia luminii
	glUniform3f(viewPosLoc, obsX, obsY, obsZ);
	//glUniform3f(viewPosLoc, 0.f, 0.f, 300.f);  //pozitia obs


	//new planet
	// Matrices for the second planet
	glm::mat4 scalePlanet2 = glm::scale(glm::mat4(1.0f), glm::vec3(0.5, 0.5, 0.5)); // Smaller size
	glm::mat4 rotatePlanetAxis2 = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0)); // Rotation on its axis
	glm::mat4 rotatePlanet2 = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.2, 1.0, -0.1)); // Orbiting around the sun
	glm::mat4 translatePlanet2 = glm::translate(glm::mat4(1.0f), glm::vec3(150.0, 0.0, 0.0)); // Farther from the sun

	// Push view matrix onto the stack
	mvStack.push(view);

	// Update the stack for the new planet
	mvStack.top() *= translateSystem;      // Start with the system's translation
	mvStack.top() *= rotatePlanet2;        // Apply the planet's orbit
	mvStack.top() *= translatePlanet2;     // Translate the planet from the sun
	mvStack.top() *= rotatePlanetAxis2;    // Rotate the planet on its axis
	mvStack.top() *= scalePlanet2;         // Scale the planet to its size

	// Send the transformation matrix to the shader
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	// Draw the new planet
	codCol = 1; // Different color code for this planet
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++) {
		if ((patr + 1) % (NR_PARR + 1) != 0) {
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort))
			);
		}
	}

	//asteroids
	glm::mat4 asteroidRingTransform = mvStack.top();
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(asteroidRingTransform));
	codCol = 6;
	glUniform1i(codColLocation, codCol);
	glBindVertexArray(asteroidsVAO);
	glPointSize(5.0f);
	glDrawArrays(GL_POINTS, 0, numAsteroids);
	glBindVertexArray(0);

	// Pop the stack after rendering the ring
	mvStack.pop();

	//new planet
	// Matrices for the second planet
	glm::mat4 scalePlanet3 = glm::scale(glm::mat4(1.0f), glm::vec3(0.3, 0.3, 0.3)); // Smaller size
	glm::mat4 rotatePlanetAxis3 = glm::rotate(glm::mat4(1.0f), (float)0.001 * timeElapsed, glm::vec3(0.0, 1.0, 0.0)); // Rotation on its axis
	glm::mat4 rotatePlanet3 = glm::rotate(glm::mat4(1.0f), (float)0.0007 * timeElapsed, glm::vec3(0.0, 1.0, 0.0)); // Orbiting around the sun
	glm::mat4 translatePlanet3 = glm::translate(glm::mat4(1.0f), glm::vec3(200.0, 0.0, 0.0)); // Farther from the sun

	// Push view matrix onto the stack
	mvStack.push(view);

	// Update the stack for the new planet
	mvStack.top() *= translateSystem;      // Start with the system's translation
	mvStack.top() *= rotatePlanet3;        // Apply the planet's orbit
	mvStack.top() *= translatePlanet3;     // Translate the planet from the sun
	mvStack.top() *= rotatePlanetAxis3;    // Rotate the planet on its axis
	mvStack.top() *= scalePlanet3;         // Scale the planet to its size

	// Send the transformation matrix to the shader
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	// Draw the new planet
	codCol = 5; // Different color code for this planet
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++) {
		if ((patr + 1) % (NR_PARR + 1) != 0) {
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort))
			);
		}
	}

	mvStack.top() *= rotatePlanet3;        // Apply the planet's orbit
	mvStack.top() *= translatePlanet;     // Translate the planet from the sun
	mvStack.top() *= rotatePlanetAxis3;    // Rotate the planet on its axis
	mvStack.top() *= scalePlanet;         // Scale the planet to its size

	// Send the transformation matrix to the shader
	glUniformMatrix4fv(viewModelLocation, 1, GL_FALSE, glm::value_ptr(mvStack.top()));

	// Draw the new planet
	codCol = 3; // Different color code for this planet
	glUniform1i(codColLocation, codCol);
	for (int patr = 0; patr < (NR_PARR + 1) * NR_MERID; patr++) {
		if ((patr + 1) % (NR_PARR + 1) != 0) {
			glDrawElements(
				GL_QUADS,
				4,
				GL_UNSIGNED_SHORT,
				(GLvoid*)((2 * (NR_PARR + 1) * (NR_MERID)+4 * patr) * sizeof(GLushort))
			);
		}
	}


	//texturize 
	GLuint textureMoon = LoadTexture("moon.jpg");
	//GLuint textureJupiter = LoadTexture("jupiter.jpg");
	GLuint textureJupiter = LoadTexture("galben.jpg");
	GLuint textureUranus = LoadTexture("uranus.jpg");
	GLuint textureEarth = LoadTexture("earth_clouds.jpg");

	glUseProgram(ProgramId);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureMoon);
	// Set parameters for the second texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(glGetUniformLocation(ProgramId, "moonTexture"), 0);
	
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, textureJupiter);
	// Set parameters for the second texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(glGetUniformLocation(ProgramId, "jupiterTexture"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, textureUranus);
	glUniform1i(glGetUniformLocation(ProgramId, "uranusTexture"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, textureEarth);
	// Set parameters for the second texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glUniform1i(glGetUniformLocation(ProgramId, "earthTexture"), 3);
	
	glBindVertexArray(VaoId);
	glDrawElements(GL_QUADS, numIndices, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
	

	glutSwapBuffers();	//	Inlocuieste imaginea deseneata in fereastra cu cea randata; 
	glFlush();			//  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
}


//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);

	//InitAudio();

	//PlayMusic("song.mp3"); // Replace with your music file path

	//CleanAudio();


	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);		//	Se folosesc 2 buffere pentru desen (unul pentru afisare si unul pentru randare => animatii cursive) si culori RGB + 1 buffer pentru adancime;
	glutInitWindowSize(winWidth, winHeight);						//  Dimensiunea ferestrei;
	glutInitWindowPosition(100, 100);								//  Pozitia initiala a ferestrei;
	glutCreateWindow("Miscare relativa. Utilizarea stivelor de matrice");		//	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();							//  Setarea parametrilor necesari pentru fereastra de vizualizare; 
	glutDisplayFunc(RenderFunction);		//  Desenarea scenei in fereastra;
	glutIdleFunc(RenderFunction);			//	Asigura rularea continua a randarii;
	glutKeyboardFunc(ProcessNormalKeys);	//	Functii ce proceseaza inputul de la tastatura utilizatorului;
	glutSpecialFunc(ProcessSpecialKeys);
	glutCloseFunc(Cleanup);					//  Eliberarea resurselor alocate de program;F

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}

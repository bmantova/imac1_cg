#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define NB_COL 8

static unsigned int WINDOW_WIDTH = 800;	/* Dimensions de la fenêtre */
static unsigned int WINDOW_HEIGHT = 600;

static const unsigned int BIT_PER_PIXEL = 32;	/* Nombre de bits par pixel de la fenêtre */

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 120;	/* Nombre minimal de millisecondes separant le rendu de deux images */
static const int nbSegmentCircle = 256;

static const int screenScaleX = 160;
static const int screenScaleY = 120;

typedef struct Point{
	float x,y;
	unsigned int r,g,b;
	struct Point *next;
} Point, *PointList;

typedef struct Primitive{
	GLenum primitiveType;
	PointList points;
	struct Primitive* next;
} Primitive, *PrimitiveList;

typedef struct Color{
	int r,g,b;
} Color;

int cursX = 0;
int cursY = 0;

int clickX = 0;
int clickY = 0;

float getX(int x)
{
	return (-1+2.*x/WINDOW_WIDTH)*screenScaleX;
}
float getY(int y)
{
	return (1-2.*y/WINDOW_HEIGHT)*screenScaleY;
}

void resizeWindow(int w, int h)
{
	WINDOW_WIDTH = w;
	WINDOW_HEIGHT = h;
	SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, BIT_PER_PIXEL, SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE);

	glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluOrtho2D(-1.0,1.0,-1.0,1.0);

}

Point* allocPoint(float x,float y, unsigned int r,unsigned int g, unsigned int b)
{
	Point *p = malloc(sizeof(Point));
	if(p == NULL)
	{
		printf("Erreur dans le malloc d'un point !\n");
		return 0;
	}
	p->x = x;
	p->y = y;
	p->r = r;
	p->g = g;
	p->b = b;
	p->next = NULL;
	return p;
}
void addPointToList(Point *p,PointList *l)
{
	PointList tl = *l;
	if(tl == NULL)
	{
		*l = p;	
	}
	else
	{
		while(tl->next != NULL)
		{
			tl = tl->next;
		}
		tl->next = p;
	}
}
void drawPoints(PointList l, GLenum type, int relie)
{
	PointList tl = l;
	if(tl != NULL)
	{
		glBegin(type);
		while(tl != NULL)
		{
			glColor3ub(tl->r,tl->g,tl->b);
			glVertex2f(getX(tl->x),getY(tl->y));
			tl = tl->next;
		}
		if(relie)
		{
			glVertex2f(getX(cursX),getY(cursY));
		}
		glEnd();
	}
}
void deletePoint(PointList l)
{
	if(l != NULL)
	{
		deletePoint(l->next);
		free(l);
	}
}

Primitive* allocPrimitive(GLenum primitiveType)
{
	Primitive *p = malloc(sizeof(Primitive));
	if(p == NULL)
	{
		printf("Erreur dans le malloc d'une primitive !\n");
		return 0;
	}
	p->primitiveType = primitiveType;
	p->points = NULL;
	p->next = NULL;
	return p;
}
void addPrimitiveToList(Primitive *p,PrimitiveList *l)
{
	PrimitiveList tl = *l;
	if(tl == NULL)
	{
		*l = p;	
	}
	else
	{
		while(tl->next != NULL)
		{
			tl = tl->next;
		}
		tl->next = p;
	}
}
void drawPrimitive(PrimitiveList l)
{
	int relie;
	PrimitiveList tl = l;
	if(tl != NULL)
	{
		while(tl != NULL)
		{
			relie = 0;
			if(tl->next == NULL)
			{
				relie = 1;
			}
			drawPoints(tl->points,tl->primitiveType,relie);
			tl = tl->next;
		}
	}
}
void deletePrimitive(PrimitiveList l)
{
	if(l != NULL)
	{
		deletePrimitive(l->next);
		deletePoint(l->points);
		free(l);
	}
}

void showPalette(Color *c)
{
	int n = NB_COL;
	int i = 0,unit = WINDOW_WIDTH/n;
	
	for(i = 0;i < n;i++)
	{
		glColor3ub(c[i].r,c[i].g,c[i].b);
		glRectd(getX(i*unit),getY(0),getX((i+1)*unit),getY(WINDOW_HEIGHT));
	}
}
void getPaletteColor(int x,Color* c,Color* pal)
{
	int n;
	n = ((float)x/WINDOW_WIDTH)*NB_COL;
	c->r = pal[n].r;
	c->g = pal[n].g;
	c->b = pal[n].b;
}
void drawSquare(int fill)
{
	if(fill == 1)
	{
		glBegin(GL_POLYGON);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}
	glVertex2f(-0.5,-0.5);
	glVertex2f(0.5,-0.5);
	glVertex2f(0.5,0.5);
	glVertex2f(-0.5,0.5);
	glEnd();
}
void drawLandmark()
{
	glColor3ub(0,255,0);
	glBegin(GL_LINES);
	glVertex2f(0,0);
	glVertex2f(0,0.5);
	glColor3ub(255,0,0);
	glVertex2f(0,0);
	glVertex2f(0.5,0);
	glEnd();
}
void drawCircle(int fill)
{
	float i = 0;
	if(fill == 1)
	{
		glBegin(GL_POLYGON);
	}
	else
	{
		glBegin(GL_LINE_LOOP);
	}
	for(i = 0;i < M_PI*2;i = i + (M_PI*2)/nbSegmentCircle)
	{
		glVertex2f(cos(i)/2,sin(i)/2);
	}
	glEnd();
	
}
void cross(int x,int y,Color c)
{
	glColor3ub(c.r,c.g,c.b);
	glRectd(getX(x-1),getY(y-4),getX(x+1),getY(y+4));
	glRectd(getX(x-4),getY(y-1),getX(x+4),getY(y+1));
}
int randint(int a, int b){
	static int first = 0;
	if(first == 0)
	{
		srand (time (NULL) );
		first = 1;
	}
	return rand()%((b+1)-a) + a;
}
void drawRoundedSquare()
{
	float r = 0.8;
	glPushMatrix();
		glScalef(r,1.0,1.0);
		drawSquare(1);
	glPopMatrix();
	glPushMatrix();
		glScalef(1.0,r,1.0);
		drawSquare(1);
	glPopMatrix();

	glPushMatrix();
		glTranslatef(-r/2,-r/2,0.0);
		glScalef(1-r,1-r,1.0);
		drawCircle(1);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(r/2,-r/2,0.0);
		glScalef(1-r,1-r,1.0);
		drawCircle(1);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(-r/2,r/2,0.0);
		glScalef(1-r,1-r,1.0);
		drawCircle(1);
	glPopMatrix();
	glPushMatrix();
		glTranslatef(r/2,r/2,0.0);
		glScalef(1-r,1-r,1.0);
		drawCircle(1);
	glPopMatrix();
}
GLuint createThirdArmIDList()
{
	GLuint id = glGenLists(1);
	glNewList(id, GL_COMPILE);
		glColor3ub(64,64,64);
		glPushMatrix();
			glPushMatrix();
				glTranslatef(20.0,0.0,0.0);
				glScalef(40.0,4.0,1.0);
				drawSquare(1);
			glPopMatrix();
			glPushMatrix();
				glScalef(6.0,6.0,1.0);
				drawSquare(1);
			glPopMatrix();
			glColor3ub(0,0,0);
			glPushMatrix();
				glTranslatef(40.0,0.0,0.0);
				glScalef(8.0,8.0,1.0);
				drawCircle(1);
			glPopMatrix();
		glPopMatrix();
	glEndList();
	return id;
}
GLuint createSecondArmIDList()
{
	GLuint id = glGenLists(1);
	glNewList(id, GL_COMPILE);
		glPushMatrix();
			glColor3ub(128,128,150);
			glPushMatrix();
				glTranslatef(25.0,0.0,0.0);
				glScalef(46.0,7.0,1.0);
				drawSquare(1);
			glPopMatrix();
			glColor3ub(0,0,0);
			glPushMatrix();
				glScalef(10.0,10.0,1.0);
				drawRoundedSquare();
			glPopMatrix();
			glPushMatrix();
				glTranslatef(50.0,0.0,0.0);
				glPushMatrix();
					glScalef(10.0,10.0,1.0);
					drawRoundedSquare();
				glPopMatrix();
			glPopMatrix();
		glPopMatrix();
	glEndList();
	return id;
}
GLuint createFirstArmIDList()
{
	GLuint id = glGenLists(1);
	glNewList(id, GL_COMPILE);
		glColor3ub(128,128,150);
		glPushMatrix();
			glBegin(GL_QUAD_STRIP);
				glVertex2f(0.0,20.0);
				glVertex2f(60.0,10.0);
				glVertex2f(0.0,-20.0);
				glVertex2f(60.0,-10.0);
			glEnd();
		glPopMatrix();
		glColor3ub(30,30,30);
		glPushMatrix();
			glScalef(40,40,1.0);
			drawCircle(1);
		glPopMatrix();
		glPushMatrix();
			glTranslatef(60.0,0.0,0.0);
			glPushMatrix();
				glScalef(20.0,20.0,1.0);
				drawCircle(1);
			glPopMatrix();
		glPopMatrix();
	glEndList();
	return id;
}

int main(int argc, char** argv)
{
	if(-1 == SDL_Init(SDL_INIT_VIDEO))	/* Initialisation de la SDL */
	{
		fprintf(stderr, "Impossible d'initialiser la SDL. Fin du programme.\n");
		return EXIT_FAILURE;
	}

	if(NULL == SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, BIT_PER_PIXEL, SDL_OPENGL | SDL_GL_DOUBLEBUFFER | SDL_RESIZABLE))	/* Ouverture d'une fenêtre et création d'un contexte OpenGL */
	{
		fprintf(stderr, "Impossible d'ouvrir la fenetre. Fin du programme.\n");
		return EXIT_FAILURE;
	}

	/*SDL_ShowCursor( SDL_DISABLE ); *//* Cacher le cursor */

	SDL_WM_SetCaption("Paint 1D", NULL);	/* Titre de la fenêtre */

	int loop = 1;

	int paletteMode = 0;
	int onRightClick = 0;

	PointList pts = NULL;
	Point *tmpPt;

	PrimitiveList prims = NULL;
	Primitive *tmpPrim = allocPrimitive(GL_POINTS);

	addPrimitiveToList(tmpPrim,&prims);

	Color *palCol = malloc(sizeof(Color)*NB_COL);
	Color curCol;
	curCol.r = curCol.g = curCol.b = 0;

	palCol[0].r = palCol[0].g = palCol[0].b = 0;	/* Noir */
	palCol[1].r = palCol[1].g = palCol[1].b = 255;	/* Blanc */
	palCol[2].r = 255;		/* Rouge */
	palCol[2].g = palCol[2].b = 0;
	palCol[3].g = 255;		/* Vert */
	palCol[3].r = palCol[3].b = 0;
	palCol[4].b = 255;		/* Bleu */
	palCol[4].r = palCol[4].g = 0;
	palCol[5].b = 0;		/* Jaune */
	palCol[5].r = palCol[5].g = 255;
	palCol[6].r = 0;		/* Cyan */
	palCol[6].b = palCol[6].g = 255;
	palCol[7].g = 0;		/* Magenta */
	palCol[7].b = palCol[7].r = 255;

	GLuint firstArm = createFirstArmIDList();
	GLuint secondArm = createSecondArmIDList();
	GLuint thirdArm = createThirdArmIDList();

	int alpha = 45;
	int beta = -10;
	int gamma = 35;

	while(loop)
	{
		glClearColor(1,1,1,1);
		glClear(GL_COLOR_BUFFER_BIT);
		Uint32 startTime = SDL_GetTicks();	/* Récupération du temps au début de la boucle */

		if(paletteMode)
		{
			showPalette(palCol);
		}
		else
		{
			/*drawPoints(pts);*/

			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			glScalef(1.0/screenScaleX,1.0/screenScaleY,1.0);

			drawPrimitive(prims);

			drawLandmark();

			glColor3ub(0,0,0);
			glPushMatrix();
				glRotatef(alpha,0.0,0.0,1.0);
				glCallList(firstArm);
				glTranslatef(60.0,0.0,0.0);;
				glPushMatrix();
					glRotatef(beta,0.0,0.0,1.0);
					glCallList(secondArm);
					glTranslatef(50.0,0.0,0.0);
					glRotatef(gamma,0.0,0.0,1.0);
					glCallList(thirdArm);
				glPopMatrix();
			glPopMatrix();

			alpha = alpha + 10;
			beta = beta + 10;
			gamma = gamma + 10;
		}
		cross(cursX,cursY,curCol);
	
		SDL_GL_SwapBuffers(); /* Echange du front et du back buffer : mise à jour de la fenêtre */

		SDL_Event e;	/* Boucle traitant les evenements */
		while(SDL_PollEvent(&e))
		{
			switch(e.type)	/* Quelques exemples de traitement d'evenements : */
			{
				case SDL_QUIT:
					loop = 0;
					break;
				case SDL_MOUSEBUTTONUP:
					if(e.button.button == SDL_BUTTON_LEFT)
					{
						cursX = e.button.x;
						cursY = e.button.y;

						clickX = e.button.x;
						clickY = e.button.y;

						if(paletteMode)
						{
							getPaletteColor(cursX,&curCol,palCol);
						}
						else
						{
							tmpPt = allocPoint(cursX, cursY, curCol.r, curCol.g, curCol.b);
							addPointToList(tmpPt,&tmpPrim->points);
						}
					}
					else if(e.button.button == SDL_BUTTON_RIGHT)
					{
						onRightClick = 0;
						if(tmpPrim->primitiveType == GL_LINE_STRIP)
						{
							tmpPrim->primitiveType = GL_LINE_LOOP;
							tmpPrim = allocPrimitive(GL_LINE_STRIP);
							addPrimitiveToList(tmpPrim,&prims);
						}
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					if(e.button.button == SDL_BUTTON_RIGHT)
					{
						onRightClick = 1;
					}
					break;
				case SDL_MOUSEMOTION:
					cursX = e.button.x;
					cursY = e.button.y;
					/*glClearColor((float)cursX/WINDOW_WIDTH,(float)cursY/WINDOW_HEIGHT,0,1);*/
					break;
				case SDL_KEYUP:	/* Touche clavier */
					switch(e.key.keysym.sym)
					{
						case 'q':
							loop = 0;
							break;
						case 27: /* Echap */
							loop = 0;
							break;
						case 'p':
							tmpPrim = allocPrimitive(GL_POINTS);
							addPrimitiveToList(tmpPrim,&prims);
							break;
						case 'l':
							tmpPrim = allocPrimitive(GL_LINES);
							addPrimitiveToList(tmpPrim,&prims);
							break;
						case 't':
							tmpPrim = allocPrimitive(GL_TRIANGLES);
							addPrimitiveToList(tmpPrim,&prims);
							break;
						case 's':
							if(tmpPrim->primitiveType == GL_LINE_STRIP)
							{
								tmpPrim->primitiveType = GL_LINE_LOOP;
							}
							tmpPrim = allocPrimitive(GL_LINE_STRIP);
							addPrimitiveToList(tmpPrim,&prims);
							break;
						case 32:
							paletteMode = 0;
							break;
						default:
							printf("keyup = %d\n",e.key.keysym.sym);
							break;
					}
					break;
				case SDL_KEYDOWN:	/* Touche clavier */
					switch(e.key.keysym.sym)
					{
						case 32:
							paletteMode = 1;
							break;
						default:
							break;
					}
					break;
				case SDL_VIDEORESIZE:	/* redimentionnement de la fenêtre */
					resizeWindow(e.resize.w,e.resize.h);
					break;
			}
		}
		Uint32 elapsedTime = SDL_GetTicks() - startTime;	/* Calcul du temps écoulé */

		if(elapsedTime < FRAMERATE_MILLISECONDS)	/* Si trop peu de temps s'est écoulé, on met en pause le programme */
		{
		    SDL_Delay(FRAMERATE_MILLISECONDS - elapsedTime);
		}
	}
	deletePoint(pts);
	deletePrimitive(prims);
	SDL_Quit();	/* Liberation des ressources associées à la SDL */ 

	return EXIT_SUCCESS;
}

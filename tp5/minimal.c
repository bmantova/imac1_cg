#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define NB_COL 8

static unsigned int WINDOW_WIDTH = 600;	/* Dimensions de la fenêtre */
static unsigned int WINDOW_HEIGHT = 600;

static const unsigned int BIT_PER_PIXEL = 32;	/* Nombre de bits par pixel de la fenêtre */

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 120;	/* Nombre minimal de millisecondes separant le rendu de deux images */
static const int nbSegmentCircle = 256;

static const int screenScaleX = 100;
static const int screenScaleY = 100;

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

struct planette {
	int diam;
	float dist;
	float angle;
	struct planette *next;
} Planette, *LPlanette;

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
void backgroundHorloge()
{
	int i = 0;
	glPushMatrix();
		glScalef(190,190,1);
		glColor3ub(0,0,0);
		drawCircle(1);
	glPopMatrix();

	glPushMatrix();
		glScalef(185,185,1);
		glColor3ub(255,255,255);
		drawCircle(1);
	glPopMatrix();

	glPushMatrix();
		glScalef(183,183,1);
		glColor3ub(0,0,0);
		drawCircle(1);
	glPopMatrix();

	glPushMatrix();
		glScalef(180,180,1);
		glColor3ub(255,255,255);
		drawCircle(1);
	glPopMatrix();

	
	glColor3ub(0,0,0);
	for(i = 0;i < 360;i+=30)
	{
		glPushMatrix();
			glRotatef(i,0.0,0.0,1.0);
			glTranslatef(80,0,0);
			glScalef(10,2,1);
			drawSquare(1);
		glPopMatrix();
	}
	for(i = 0;i < 360;i+=6)
	{
		glPushMatrix();
			glRotatef(i,0.0,0.0,1.0);
			glTranslatef(80,0,0);
			glScalef(8,1,1);
			drawSquare(1);
		glPopMatrix();
	}
}
void aiguilles()
{
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	glPushMatrix();
		glRotatef(90-timeinfo->tm_sec*6,0.0,0.0,1.0);
		glTranslatef(30,0,0);
		glScalef(100,1,1); // secondes
		drawSquare(1);
	glPopMatrix();
	glPushMatrix();
		glRotatef(90-timeinfo->tm_min*6,0.0,0.0,1.0);
		glTranslatef(30,0,0);
		glScalef(70,2,1); // secondes
		drawSquare(1);
	glPopMatrix();
	glPushMatrix();
		glRotatef(90-timeinfo->tm_hour*30,0.0,0.0,1.0);
		glTranslatef(20,0,0);
		glScalef(40,3,1); // secondes
		drawSquare(1);
	glPopMatrix();
}

Planette* allocPlanette(float dist,int diam, float angle)
{
	Planette *p = malloc(sizeof(Point));
	if(p == NULL)
	{
		printf("Erreur dans le malloc d'une planette !\n");
		return 0;
	}
	p->dist = dist;
	p->diam = diam;
	p->angle = angle;
	p->next = NULL;
	return p;
}
void addPlanette(Lplanette *l,float dist,int diam, float angle)
{
	Planette *p = allocPlanette(dist,diam,angle);
	if(l == NULL)
	{
		*l = p;
	}
	else
	{
		p->next = *l;
		*l = p;
	}
}
void showPlanette(Planette p)
{
	unsigned int dist = p.dist * 149597;	/* En million de KM */
	int diam = p.diam / 1000;		/* ' ' */

	
	glPushMatrix();
		glRotatef(p.angle,0.0,0.0,1.0);
		glTranslatef(p.dist,0,0);
		glScalef(p.diam,p.diam,1); // secondes
		drawSquare(1);
	glPopMatrix();
}
void showAllPlanette(LPlanette l)
{
	
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

	LPlanette lp;
	addPlanette(&lp,0,1392000); /* Soleil */
	addPlanette(&lp,0.4,4878); /* Mercure */

	SDL_WM_SetCaption("Horloge", NULL);	/* Titre de la fenêtre */

	int loop = 1;
	resizeWindow(WINDOW_WIDTH, WINDOW_HEIGHT);

	while(loop)
	{
   		glClearColor(0.1, 0.1, 0.1 ,1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		Uint32 startTime = SDL_GetTicks();	/* Récupération du temps au début de la boucle */

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glScalef(1.0/screenScaleX,1.0/screenScaleY,1.0);

		backgroundHorloge();
		aiguilles();
	
		SDL_GL_SwapBuffers(); /* Echange du front et du back buffer : mise à jour de la fenêtre */

		SDL_Event e;	/* Boucle traitant les evenements */
		while(SDL_PollEvent(&e))
		{
			switch(e.type)	/* Quelques exemples de traitement d'evenements : */
			{
				case SDL_QUIT:
					loop = 0;
					break;
				case SDL_KEYUP:	/* Touche clavier */
					switch(e.key.keysym.sym)
					{
						case 'q':
							loop = 0;
							break;
						default:
							printf("keyup = %d\n",e.key.keysym.sym);
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
	SDL_Quit();	/* Liberation des ressources associées à la SDL */ 

	return EXIT_SUCCESS;
}

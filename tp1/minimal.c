#include <SDL/SDL.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>

#define NB_COL 8

static unsigned int WINDOW_WIDTH = 800;	/* Dimensions de la fenêtre */
static unsigned int WINDOW_HEIGHT = 600;

static const unsigned int BIT_PER_PIXEL = 32;	/* Nombre de bits par pixel de la fenêtre */

static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;	/* Nombre minimal de millisecondes separant le rendu de deux images */

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

float getX(int x)
{
	return -1+2.*x/WINDOW_WIDTH;
}
float getY(int y)
{
	return 1-2.*y/WINDOW_HEIGHT;
}

void resizeWindow(int w, int h)
{
	WINDOW_WIDTH = w;
	WINDOW_HEIGHT = h;

	glViewport(0,0,WINDOW_WIDTH,WINDOW_HEIGHT);
	glMatrixMode(GL_PROJECTION);
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
void drawPoints(PointList l, GLenum type)
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
	PrimitiveList tl = l;
	if(tl != NULL)
	{
		while(tl != NULL)
		{
			drawPoints(tl->points,tl->primitiveType);
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

	SDL_WM_SetCaption("Paint 1D", NULL);	/* Titre de la fenêtre */

	int loop = 1;
	int cursX = 0;
	int cursY = 0;

	int paletteMode = 0;

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
			drawPrimitive(prims);
		}
	
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
					cursX = e.button.x;
					cursY = e.button.y;
					if(paletteMode)
					{
						getPaletteColor(cursX,&curCol,palCol);
					}
					else
					{
						tmpPt = allocPoint(cursX, cursY, curCol.r, curCol.g, curCol.b);
						addPointToList(tmpPt,&tmpPrim->points);
					}
					break;
				/*case SDL_MOUSEMOTION:
					cursX = e.button.x;
					cursY = e.button.y;*/
					/*glClearColor((float)cursX/WINDOW_WIDTH,(float)cursY/WINDOW_HEIGHT,0,1);*/
					/*break;*/
				case SDL_KEYUP:	/* Touche clavier */
					switch(e.key.keysym.sym)
					{
						case 'q':
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
						case 32:
							paletteMode = 0;
							break;
						default:
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

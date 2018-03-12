#include <SDL/SDL.h>
#include "./SDL_image-1.2.12/SDL_image.h"
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

static unsigned int WINDOW_WIDTH = 800;
static unsigned int WINDOW_HEIGHT = 800;
static const unsigned int BIT_PER_PIXEL = 32;
static const Uint32 FRAMERATE_MILLISECONDS = 1000 / 60;

void resizeViewport() {
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1., 1., -1., 1.);
    SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, BIT_PER_PIXEL, SDL_OPENGL | SDL_RESIZABLE);
}

const char* filename = "./logo_imac_400x400.jpg";

int main(int argc, char** argv) {

    // Initialisation de la SDL
	SDL_Surface *LogoImac;
	GLuint logoImacId;
    if(-1 == SDL_Init(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Impossible d'initialiser la SDL. Fin du programme.\n");
        return EXIT_FAILURE;
    }

    // Ouverture d'une fenêtre et création d'un contexte OpenGL
    if(NULL == SDL_SetVideoMode(WINDOW_WIDTH, WINDOW_HEIGHT, BIT_PER_PIXEL, SDL_OPENGL | SDL_RESIZABLE)) {
        fprintf(stderr, "Impossible d'ouvrir la fenetre. Fin du programme.\n");
        return EXIT_FAILURE;
    }
    SDL_WM_SetCaption("td04", NULL);

    // TODO: Chargement et traitement de la texture

    LogoImac = IMG_Load(filename);
	if(NULL == LogoImac)
	{
        	fprintf(stderr, "Impossible d'ouvrir l'image. Fin du programme.\n");	
		return EXIT_FAILURE;
	}
	glGenTextures(1,&logoImacId);
	glBindTexture(GL_TEXTURE_2D,logoImacId);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,LogoImac->w,LogoImac->h,0,GL_RGB,GL_UNSIGNED_BYTE,LogoImac->pixels);
	glBindTexture(GL_TEXTURE_2D,0);

    // TODO: Libération des données CPU
	SDL_FreeSurface(LogoImac);

    resizeViewport();
    // Boucle de dessin (à décommenter pour l'exercice 3)
    int loop = 1;
    glClearColor(0.1, 0.1, 0.1 ,1.0);
    while(loop) {

        Uint32 startTime = SDL_GetTicks();

        // TODO: Code de dessin
	//glColor3ub(255,0,0);


        glClear(GL_COLOR_BUFFER_BIT);

        // ...
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,logoImacId);
	
	glPushMatrix();
		glTranslatef(0.5,0.0,0.0);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0,0.0);
			glVertex2f(-0.5,-0.5);

			glTexCoord2f(1.0,0.0);
			glVertex2f(0.5,-0.5);

			glTexCoord2f(1.0,1.0);
			glVertex2f(0.5,0.5);

			glTexCoord2f(0.0,1.0);
			glVertex2f(-0.5,0.5);
		glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,0);

        // Fin du code de dessin

        SDL_Event e;
        while(SDL_PollEvent(&e)) {

            switch(e.type) {

                case SDL_QUIT:
                    loop = 0;
                    break;

                case SDL_VIDEORESIZE:
                    WINDOW_WIDTH = e.resize.w;
                    WINDOW_HEIGHT = e.resize.h;
                    resizeViewport();

                default:
                    break;
            }
        }

        SDL_GL_SwapBuffers();
        Uint32 elapsedTime = SDL_GetTicks() - startTime;
        if(elapsedTime < FRAMERATE_MILLISECONDS) {
            SDL_Delay(FRAMERATE_MILLISECONDS - elapsedTime);
        }
    }

    // TODO: Libération des données GPU
	glDeleteTextures(1,&logoImacId);


    // Liberation des ressources associées à la SDL
    SDL_Quit();

    return EXIT_SUCCESS;
}

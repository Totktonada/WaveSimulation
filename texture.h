#ifndef TEXTURE_H_SENTRY
#define TEXTURE_H_SENTRY

GLuint createTexture(int w, int h, GLint format, const GLvoid * data);

GLuint createTextureFromTga(const char * path);

void freeTexture(GLuint texture);

#endif /* TEXTURE_H_SENTRY */

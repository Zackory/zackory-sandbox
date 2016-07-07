import OpenGL.GL as gl
import OpenGL.GLU as glu
import OpenGL.GLUT as glut
from OpenGL.arrays import vbo
from OpenGL.GL import shaders
from OpenGL.GL.ARB.multitexture import *
from PIL import Image

import numpy as np
import sys, struct

ESCAPE = '\033'

# compileProgram in OpenGL.GL.shaders fails to validate if multiple samplers are used
def compileProgram(*shaders):
    program = gl.glCreateProgram()
    for shader in shaders:
        gl.glAttachShader(program, shader)

    gl.glLinkProgram(program)

    for shader in shaders:
        gl.glDeleteShader(shader)

    return program

class Images:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.shader = None

        self.triangleShadeProgram = None
        self.triangleBlueShadeProgram = None

        self.prepareGLUT()
        self.init()

        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        glu.gluOrtho2D(0, 1, 0, 1)
        # glu.gluOrtho2D(0, self.width, 0, self.height)

        glut.glutMainLoop()

    def prepareGLUT(self):
        glut.glutInit(sys.argv)
        glut.glutInitWindowSize(self.width, self.height)
        glut.glutCreateWindow('Sample Shader Heatmap')
        glut.glutInitDisplayMode(glut.GLUT_DOUBLE | glut.GLUT_RGB)
        # glut.glutInitDisplayMode(glut.GLUT_RGB)
        glut.glutDisplayFunc(self.display)
        glut.glutKeyboardFunc(self.keyPressed)

    def init(self):
        # Compile Triangle Shaders (first with a green color and the second as blue)
        self.triangleShadeProgram = compileProgram(
            shaders.compileShader('''#version 130
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                }''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''#version 130
                void main() {
                    gl_FragColor = vec4(0, 1, 0, 1);
                }''', gl.GL_FRAGMENT_SHADER))

        self.triangleBlueShadeProgram = compileProgram(
            shaders.compileShader('''#version 130
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                }''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''#version 130
                void main() {
                    gl_FragColor = vec4(0, 0, 1, 1);
                }''', gl.GL_FRAGMENT_SHADER))

        # Load Texture Image
        image = Image.open('tank_night.png')
        imageData = image.tostring('raw', 'RGBA', 0, -1)

        im = gl.glGenTextures(1)
        gl.glBindTexture(gl.GL_TEXTURE_2D, im)
        gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP)
        gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP)
        gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_LINEAR)
        gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_LINEAR)
        gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGB, image.size[0], image.size[1], 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, imageData)
        gl.glEnable(gl.GL_TEXTURE_2D)

        self.textureShadeProgram = compileProgram(
            shaders.compileShader('''#version 130
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                }''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''#version 130
                void main() {
                    // gl_FragColor = vec4(0, 0, 1, 1);
                }''', gl.GL_FRAGMENT_SHADER))

    def display(self):
        # gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)
        # gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, 0)

        gl.glUseProgram(self.triangleShadeProgram)
        gl.glBegin(gl.GL_TRIANGLES)
        gl.glVertex2d(0.5, 1)
        gl.glVertex2d(0, 0)
        gl.glVertex2d(1, 0)
        gl.glEnd()
        gl.glFlush()

        gl.glUseProgram(self.triangleBlueShadeProgram)
        gl.glBegin(gl.GL_TRIANGLES)
        gl.glVertex2d(0.25, 1)
        gl.glVertex2d(0, 0.5)
        gl.glVertex2d(0.5, 0.5)
        gl.glEnd()
        gl.glFlush()

        gl.glUseProgram(0)
        gl.glBegin(gl.GL_QUADS)
        gl.glTexCoord2d(0, 0)
        gl.glVertex2d(0.5, 0)
        gl.glTexCoord2d(1, 0)
        gl.glVertex2d(1, 0)
        gl.glTexCoord2d(1, 1)
        gl.glVertex2d(1, 0.5)
        gl.glTexCoord2d(0, 1)
        gl.glVertex2d(0.5, 0.5)
        gl.glEnd()
        gl.glFlush()

    def keyPressed(self, key, x, y):
        if key == ESCAPE or key == 'q':
            sys.exit()


if __name__ == '__main__':
    Images(800, 600)


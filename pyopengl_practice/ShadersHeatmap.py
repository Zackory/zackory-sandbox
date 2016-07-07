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

class ShadersHeatmap:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.vertexPositions = None
        self.indexPositions = None
        self.shader = None
        self.fb = None

        self.colorTransformProgram = None
        self.fadedPointsProgram = None
        self.triangleShadeProgram = None
        self.triangleBlueShadeProgram = None

        self.prepareGLUT()
        self.init()

        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        # glu.gluOrtho2D(0, 1, 0, 1)
        glu.gluOrtho2D(0, self.width, 0, self.height)

        # self.clearFramebuffer()

        glut.glutMainLoop()

    def prepareGLUT(self):
        glut.glutInit(sys.argv)
        glut.glutInitWindowSize(self.width, self.height)
        glut.glutCreateWindow('Sample Shader Heatmap')
        # glut.glutInitDisplayMode(glut.GLUT_DOUBLE | glut.GLUT_RGB)
        glut.glutInitDisplayMode(glut.GLUT_RGB)
        glut.glutDisplayFunc(self.display)
        glut.glutKeyboardFunc(self.keyPressed)

        # Render Flags
        gl.glEnable(gl.GL_BLEND)
        gl.glEnable(gl.GL_TEXTURE_1D)
        gl.glEnable(gl.GL_TEXTURE_2D)
        gl.glEnable(gl.GL_VERTEX_PROGRAM_POINT_SIZE)
	gl.glEnableClientState(gl.GL_VERTEX_ARRAY)

        self.compilePrograms()
        self.loadPalette()
        self.createFramebuffer()

    def init(self):
        self.triangleShadeProgram = compileProgram(
            shaders.compileShader('''#version 120
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                }''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''#version 120
                void main() {
                    gl_FragColor = vec4(0, 1, 0, 1);
                }''', gl.GL_FRAGMENT_SHADER))

        self.triangleBlueShadeProgram = compileProgram(
            shaders.compileShader('''#version 120
                void main() {
                    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                }''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''#version 120
                void main() {
                    gl_FragColor = vec4(0, 0, 1, 1);
                }''', gl.GL_FRAGMENT_SHADER))

    def display(self):
        print 'Yeah! 1'
        # gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

        gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, 0)
        gl.glUseProgram(self.triangleShadeProgram)
        gl.glBegin(gl.GL_TRIANGLES)
        # gl.glVertex2d(0.5, 1)
        # gl.glVertex2d(0, 0)
        # gl.glVertex2d(1, 0)
        gl.glVertex2d(self.width*0.5, self.height)
        gl.glVertex2d(0, 0)
        gl.glVertex2d(self.width, 0)
        gl.glEnd()
        gl.glFlush()

        gl.glUseProgram(self.triangleBlueShadeProgram)
        gl.glBegin(gl.GL_TRIANGLES)
        # gl.glVertex2d(0.25, 1)
        # gl.glVertex2d(0, 0.5)
        # gl.glVertex2d(0.5, 0.5)
        gl.glVertex2d(self.width*0.25, self.height)
        gl.glVertex2d(0, self.height*0.5)
        gl.glVertex2d(self.width*0.5, self.height*0.5)
        gl.glEnd()
        gl.glFlush()

        print 'Yeah!'

        radius = 200
        alpha = 0.1
        points = [(200, 200)]
        left = 0
        bottom = 0

        # Render all points with the specified radius
        gl.glUseProgram(self.fadedPointsProgram)
        gl.glUniform1f(gl.glGetUniformLocation(self.fadedPointsProgram, 'r'), radius)

        pointAttribLocation = gl.glGetAttribLocation(self.fadedPointsProgram, 'point')
        gl.glEnableVertexAttribArray(pointAttribLocation)
        gl.glVertexAttribPointer(pointAttribLocation, 2, gl.GL_FLOAT, False, 0,
                              struct.pack('ff' * 4 * len(points),
                                          *(val for (x, y) in points
                                                for val in (x - left, y - bottom) * 4)))

        print struct.unpack('ff' * 4 * len(points), struct.pack('ff' * 4 * len(points),
                                          *(val for (x, y) in points
                                                for val in (x - left, y - bottom) * 4)) )

        gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.fb)
        gl.glBlendFunc(gl.GL_ONE, gl.GL_ONE_MINUS_SRC_ALPHA)

        # vertices = [point for (x, y) in points
        #                   for point in ((x + radius, y + radius), (x - radius, y + radius),
        #                                 (x - radius, y - radius), (x + radius, y - radius))]
	# gl.glVertexPointerd(vertices)
	# gl.glDrawArrays(gl.GL_QUADS, 0, len(vertices))

        gl.glBegin(gl.GL_QUADS)
        # gl.glVertex2d(1, 1)
        # gl.glVertex2d(0, 1)
        # gl.glVertex2d(0, 0)
        # gl.glVertex2d(1, 0)
        # gl.glVertex2d(0.5, 0.5)
        # gl.glVertex2d(0, 0.5)
        # gl.glVertex2d(0, 0)
        # gl.glVertex2d(0.5, 0)
        gl.glVertex2d(self.width, self.height)
        gl.glVertex2d(0, self.height)
        gl.glVertex2d(0, 0)
        gl.glVertex2d(self.width, 0)
        gl.glEnd()
	gl.glFlush()

        gl.glDisableVertexAttribArray(pointAttribLocation)

        # Transform the color into the proper palette
        gl.glUseProgram(self.colorTransformProgram)
        gl.glUniform1f(gl.glGetUniformLocation(self.colorTransformProgram, 'alpha'), alpha)
        gl.glUniform1i(gl.glGetUniformLocation(self.colorTransformProgram, 'palette'), 0)
        gl.glUniform1i(gl.glGetUniformLocation(self.colorTransformProgram, 'framebuffer'), 1)
        gl.glUniform2f(gl.glGetUniformLocation(self.colorTransformProgram, 'windowSize'), self.width, self.height)

        gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.fb)
        gl.glBlendFunc(gl.GL_ONE, gl.GL_ZERO)
        # gl.glBlendFunc(gl.GL_ONE, gl.GL_ONE_MINUS_SRC_ALPHA)

        # vertices = [(0, 0), (1, 0), (1, 1), (0, 1)]
        vertices = [(0, 0), (self.width, 0), (self.width, self.height), (0, self.height)]
	gl.glVertexPointerd(vertices)
	gl.glDrawArrays(gl.GL_QUADS, 0, len(vertices))

        # gl.glBegin(gl.GL_QUADS)
        # gl.glVertex2d(1, 1)
        # gl.glVertex2d(0, 1)
        # gl.glVertex2d(0, 0)
        # gl.glVertex2d(1, 0)
        # gl.glEnd()

        gl.glDrawBuffers(1, gl.GL_COLOR_ATTACHMENT0)

        gl.glFlush()

        print 'Wooh!'

    def keyPressed(self, key, x, y):
        if key == ESCAPE or key == 'q':
            sys.exit()

    def clearFramebuffer(self):
        # glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, self.fbo)
        gl.glClear(gl.GL_COLOR_BUFFER_BIT)

    def loadPalette(self, path='palettes/classic.png'):
        image = Image.open(path)

        self.palette = gl.glGenTextures(1)
        # glActiveTextureARB(gl.GL_TEXTURE0)
        gl.glBindTexture(gl.GL_TEXTURE_1D, self.palette)
        gl.glTexImage1D(gl.GL_TEXTURE_1D, 0, gl.GL_RGB, image.size[1], 0, gl.GL_RGB, gl.GL_UNSIGNED_BYTE, image.tostring('raw', 'RGB', 0, -1))
        gl.glTexParameter(gl.GL_TEXTURE_1D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
        gl.glTexParameter(gl.GL_TEXTURE_1D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
        gl.glTexParameter(gl.GL_TEXTURE_1D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP)
        # gl.glEnable(gl.GL_TEXTURE_1D)

    def createFramebuffer(self):
        self.texture = gl.glGenTextures(1)
        self.fb = gl.glGenFramebuffers(1)

        # glActiveTextureARB(gl.GL_TEXTURE1)
        gl.glBindTexture(gl.GL_TEXTURE_2D, self.texture)
        gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGBA, self.width, self.height, 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, None)

        gl.glTexParameter(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_NEAREST)
        gl.glTexParameter(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_NEAREST)
        gl.glTexParameter(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP)
        gl.glTexParameter(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP)
        # gl.glEnable(gl.GL_TEXTURE_2D)

        gl.glBindFramebuffer(gl.GL_FRAMEBUFFER, self.fb)
        gl.glFramebufferTexture2D(gl.GL_FRAMEBUFFER,  gl.GL_COLOR_ATTACHMENT0, gl.GL_TEXTURE_2D, self.texture, 0)

        # status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)
        # assert status == GL_FRAMEBUFFER_COMPLETE_EXT, status

    def compilePrograms(self):
        # Shader program to transform color into the proper palette based on the alpha channel
        self.colorTransformProgram = compileProgram(
            shaders.compileShader('''
                void main() {
                    gl_Position = ftransform();
                    // gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
                }
            ''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''
                uniform float alpha;
                uniform sampler1D palette;
                uniform sampler2D framebuffer;
                uniform vec2 windowSize;

                void main() {
                    gl_FragColor.rgb = texture1D(palette, texture2D(framebuffer,  gl_FragCoord.xy / windowSize).a).rgb;
                    gl_FragColor.a = alpha;
                    // gl_FragColor = vec4(0, 0, 0, alpha);
                }
            ''', gl.GL_FRAGMENT_SHADER))

        # Shader program to place heat points
        self.fadedPointsProgram = compileProgram(
            shaders.compileShader('''
                // layout(pixel_center_integer) in vec4 gl_FragCoord;
                uniform float r;
                attribute vec2 point;
                varying vec2 center;

                void main() {
                    gl_Position = ftransform();
                    center = point;
                }
            ''', gl.GL_VERTEX_SHADER),
            shaders.compileShader('''
                uniform float r;
                varying vec2 center;

                void main() {
                    float d = distance(gl_FragCoord.xy, center);
                    if (d > r) discard;

                    gl_FragColor.rgb = vec3(1.0, 1.0, 1.0);
                    gl_FragColor.a = (0.5 + cos(d * 3.14159265 / r) * 0.5) * 0.25;

                    // Alternate fading algorithms
                    //gl_FragColor.a = (1.0 - (log(1.1+d) / log(1.1+r)));
                    //gl_FragColor.a = (1.0 - (pow(d, 0.5) / pow(r, 0.5)));
                    //gl_FragColor.a = (1.0 - ((d*d) / (r*r))) / 2.0;
                    //gl_FragColor.a = (1.0 - (d / r)) / 2.0;

                    // Clamp the alpha to the range [0.0, 1.0]
                    gl_FragColor.a = clamp(gl_FragColor.a, 0.0, 1.0);
                }
            ''', gl.GL_FRAGMENT_SHADER))


if __name__ == '__main__':
    ShadersHeatmap(800, 600)



from OpenGL.GL import *
from OpenGL.GLUT import *
from OpenGL.GLU import *
from OpenGL.GL.shaders import *
from OpenGL.GL.EXT.framebuffer_object import *
from OpenGL.GL.ARB.multitexture import *

import os, sys, serial, struct
from PIL import Image

ESCAPE = '\033'

# https://github.com/amccollum/pyheat
# http://openglsamples.sourceforge.net/cube_py.html

# compileProgram in OpenGL.GL.shaders fails to validate if multiple samplers are used
def compileProgram(*shaders):
    program = glCreateProgram()
    for shader in shaders:
        glAttachShader(program, shader)

    glLinkProgram(program)

    for shader in shaders:
        glDeleteShader(shader)

    return program

class Cube:
    def __init__(self, width, height):
        self.fbo = None
        self.texture = None
        self.palette = None
        self.faded_points_program = None
        self.color_transform_program = None

        self.width = width
        self.height = height

        # Rotation
        self.xAxis = 0
        self.yAxis = 0
        self.zAxis = 0

        # Perform Initialization
        self.cleanup()
        self.initGLUT()
        # Prepare heatmap functions
        self.prepareHeatmap()
        # Begin simulation
        self.initGL()
        glutMainLoop()

    def initGLUT(self):
        glutInit(sys.argv)
        glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH)
        glutInitWindowSize(self.width, self.height)
        glutInitWindowPosition(200, 200)

        glutCreateWindow('OpenGL Python Cube')

        glutDisplayFunc(self.drawGLScene)
        glutIdleFunc(self.drawGLScene)
        glutKeyboardFunc(self.keyPressed)

    def initGL(self):
        glClearColor(0.0, 0.0, 0.0, 0.0)
        glClearDepth(1.0)
        glDepthFunc(GL_LESS)
        glEnable(GL_DEPTH_TEST)
        glShadeModel(GL_SMOOTH)
        glMatrixMode(GL_PROJECTION)
        glLoadIdentity()
        gluPerspective(45.0, float(self.width) / float(self.height), 0.1, 100.0)
        glMatrixMode(GL_MODELVIEW)
        self.clearFramebuffer()

    def keyPressed(self, key, x, y):
        if key == ESCAPE or key == 'q':
            sys.exit()

    def drawGLScene(self):
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT)

        glLoadIdentity()
        glTranslatef(0.0, 0.0, -6.0)

        glRotatef(self.xAxis, 1.0, 0.0, 0.0)
        glRotatef(self.yAxis, 0.0, 1.0, 0.0)
        glRotatef(self.zAxis, 0.0, 0.0, 1.0)

        # Draw headmap on first side of cube

        radius = 0.5
        # points = [(0, 1, 0)]
        points = [(0, 1)]
        left = 0; bottom = 0; depth = 0

        # Render all points with the specified radius
        glUseProgram(self.faded_points_program)
        glUniform1f(glGetUniformLocation(self.faded_points_program, 'r'), radius)

        point_attrib_location = glGetAttribLocation(self.faded_points_program, 'point')
        glEnableVertexAttribArray(point_attrib_location)
        glVertexAttribPointer(point_attrib_location, 2, GL_FLOAT, False, 0,
                              struct.pack("ff" * 4 * len(points),
                                          *(val for (x, y) in points
                                                for val in (x, y) * 4)))

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, self.fbo)
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)

        vertices = [point for (x, y) in points
                          for point in ((x + radius, y + radius), (x - radius, y + radius),
                                        (x - radius, y - radius), (x + radius, y - radius))]
	glVertexPointerd(vertices)
	glDrawArrays(GL_QUADS, 0, len(vertices))
	glFlush()

        glDisableVertexAttribArray(point_attrib_location)


        # glUseProgram(self.faded_points_program)
        # glUniform1f(glGetUniformLocation(self.faded_points_program, 'r'), radius)

        # point_attrib_location = glGetAttribLocation(self.faded_points_program, 'point')
        # glEnableVertexAttribArray(point_attrib_location)
        # glVertexAttribPointer(point_attrib_location, 3, GL_FLOAT, False, 0,
        #                       struct.pack('fff' * 4 * len(points),
        #                                   *(val for (x, y, z) in points
        #                                         for val in (x - left, y - bottom, z - depth) * 4)))

        # glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, self.fbo)
        # glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA)

        # vertices = [point for (x, y, z) in points
        #                   for point in ((x + radius, y, z - radius), (x - radius, y, z - radius),
        #                                 (x - radius, y, z + radius), (x + radius, y, z + radius))]
        # glVertexPointerd(vertices)
        # glDrawArrays(GL_QUADS, 0, len(vertices))
        # glFlush()

        # glDisableVertexAttribArray(point_attrib_location)

        self.xAxis = self.xAxis - 0.30
        self.zAxis = self.zAxis - 0.30

        # print 'xAxis:', self.xAxis, 'zAxis:', self.zAxis

        glutSwapBuffers()

    def cleanup(self):
        if self.fbo:
            glDeleteFramebuffersEXT(self.fbo)
        if self.texture:
            glDeleteTextures(self.texture)
        if self.palette:
            glDeleteTextures(self.palette)
        self.fbo = self.texture = self.palette = None

    def prepareHeatmap(self):
        # Render Flags
        glEnable(GL_BLEND)
        glEnable(GL_TEXTURE_1D)
        glEnable(GL_TEXTURE_2D)
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE)
        glEnableClientState(GL_VERTEX_ARRAY)

        self.compilePrograms()
        self.loadPalette()
        self.createFramebuffer()

    def loadPalette(self, path='palettes/classic.png'):
        image = Image.open(path)

        self.palette = glGenTextures(1)
        glActiveTextureARB(GL_TEXTURE0)
        glBindTexture(GL_TEXTURE_1D, self.palette)
        glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, image.size[1], 0, GL_RGB, GL_UNSIGNED_BYTE, image.tostring('raw', 'RGB', 0, -1))

        glTexParameter(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameter(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameter(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP)

    def createFramebuffer(self):
        self.texture = glGenTextures(1)
        self.fbo = glGenFramebuffersEXT(1)

        glActiveTextureARB(GL_TEXTURE1)
        glBindTexture(GL_TEXTURE_2D, self.texture) #
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, self.width, self.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, None) #

        glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST)
        glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST)
        glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP)
        glTexParameter(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP)

        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, self.fbo) #
        glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, self.texture, 0) #

        status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT)
        assert status == GL_FRAMEBUFFER_COMPLETE_EXT, status

    def clearFramebuffer(self):
        glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, self.fbo)
        glClear(GL_COLOR_BUFFER_BIT)

    def compilePrograms(self):
        # Shader program to transform color into the proper palette based on the alpha channel
        self.color_transform_program = compileProgram(
            compileShader('''
                void main() {
                    gl_Position = ftransform();
                }
            ''', GL_VERTEX_SHADER),
            compileShader('''
                uniform float alpha;
                uniform sampler1D palette;
                uniform sampler2D framebuffer;
                uniform vec2 windowSize;

                void main() {
                    gl_FragColor.rgb = texture1D(palette, texture2D(framebuffer,  gl_FragCoord.xy / windowSize).a).rgb;
                    gl_FragColor.a = alpha;
                }
            ''', GL_FRAGMENT_SHADER))

        # Shader program to place heat points
        self.faded_points_program = compileProgram(
            compileShader('''
                uniform float r;
                attribute vec2 point;
                varying vec2 center;

                void main() {
                    gl_Position = ftransform();
                    center = point;
                }
            ''', GL_VERTEX_SHADER),
            compileShader('''
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
            ''', GL_FRAGMENT_SHADER))


if __name__ == "__main__":
    Cube(640, 480)


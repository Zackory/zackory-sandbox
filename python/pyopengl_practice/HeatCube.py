import OpenGL.GL as gl
import OpenGL.GLU as glu
import OpenGL.GLUT as glut

import sys

ESCAPE = '\033'

class HeatCube:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.xAxis = 0
        self.yAxis = 0
        self.zAxis = 0

        self.prepare()
        glut.glutMainLoop()

    def prepare(self):
        glut.glutInit(sys.argv)
        glut.glutInitDisplayMode(glut.GLUT_RGBA | glut.GLUT_DOUBLE | glut.GLUT_DEPTH)
        glut.glutInitWindowSize(self.width, self.height)
        glut.glutCreateWindow('Python Heat Cube')
        glut.glutDisplayFunc(self.display)
        glut.glutIdleFunc(self.display)
        glut.glutKeyboardFunc(self.keyPressed)

        gl.glClearColor(0.0, 0.0, 0.0, 0.0)
        gl.glClearDepth(1.0)
        gl.glDepthFunc(gl.GL_LESS)
        gl.glEnable(gl.GL_DEPTH_TEST)
        gl.glShadeModel(gl.GL_SMOOTH)
        gl.glMatrixMode(gl.GL_PROJECTION)
        gl.glLoadIdentity()
        glu.gluPerspective(45.0, self.width/self.height, 0.1, 100.0)
        gl.glMatrixMode(gl.GL_MODELVIEW)

    def display(self):
        gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)

        gl.glLoadIdentity()
        gl.glTranslatef(0.0,0.0,-6.0)

        gl.glRotatef(self.xAxis, 1.0, 0.0, 0.0)
        gl.glRotatef(self.yAxis, 0.0, 1.0, 0.0)
        gl.glRotatef(self.zAxis, 0.0, 0.0, 1.0)

        # Draw Cube (multiple quads)
        gl.glBegin(gl.GL_QUADS)

        gl.glColor3f(0.0, 1.0, 0.0)
        gl.glVertex3f( 1.0, 1.0,-1.0)
        gl.glVertex3f(-1.0, 1.0,-1.0)
        gl.glVertex3f(-1.0, 1.0, 1.0)
        gl.glVertex3f( 1.0, 1.0, 1.0)

        gl.glColor3f(1.0, 0.0, 0.0)
        gl.glVertex3f( 1.0,-1.0, 1.0)
        gl.glVertex3f(-1.0,-1.0, 1.0)
        gl.glVertex3f(-1.0,-1.0,-1.0)
        gl.glVertex3f( 1.0,-1.0,-1.0)

        gl.glColor3f(0.0, 1.0, 0.0)
        gl.glVertex3f( 1.0, 1.0, 1.0)
        gl.glVertex3f(-1.0, 1.0, 1.0)
        gl.glVertex3f(-1.0,-1.0, 1.0)
        gl.glVertex3f( 1.0,-1.0, 1.0)

        gl.glColor3f(1.0, 1.0, 0.0)
        gl.glVertex3f( 1.0,-1.0,-1.0)
        gl.glVertex3f(-1.0,-1.0,-1.0)
        gl.glVertex3f(-1.0, 1.0,-1.0)
        gl.glVertex3f( 1.0, 1.0,-1.0)

        gl.glColor3f(0.0, 0.0, 1.0)
        gl.glVertex3f(-1.0, 1.0, 1.0)
        gl.glVertex3f(-1.0, 1.0,-1.0)
        gl.glVertex3f(-1.0,-1.0,-1.0)
        gl.glVertex3f(-1.0,-1.0, 1.0)

        gl.glColor3f(1.0, 0.0, 1.0)
        gl.glVertex3f( 1.0, 1.0,-1.0)
        gl.glVertex3f( 1.0, 1.0, 1.0)
        gl.glVertex3f( 1.0,-1.0, 1.0)
        gl.glVertex3f( 1.0,-1.0,-1.0)

        gl.glEnd()

        self.xAxis -= 0.30
        self.zAxis -= 0.30

        points = [((1.2, 1.2, 0), 0.1), ((1.3, 0, .5), 0.25), ((0.3, 1.6, 0.8), 0.5)]

        gl.glColor3f(1.0, 1.0, 1.0)
        for (point, radius) in points:
            gl.glPushMatrix()
            gl.glTranslatef(point[0], point[1], point[2])
            glut.glutSolidSphere(radius, 30, 30)
            gl.glPopMatrix()

        glut.glutSwapBuffers()

    def keyPressed(self, key, x, y):
        if key == ESCAPE or key == 'q':
            sys.exit()

if __name__ == "__main__":
    HeatCube(800, 600)

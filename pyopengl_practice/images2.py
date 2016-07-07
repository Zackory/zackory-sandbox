import OpenGL.GL as gl
import OpenGL.GLU as glu
import OpenGL.GLUT as glut
from OpenGL.GL import shaders
# import pygame
from PIL import Image
import numpy as np

shader = None
def init():
    global shader

    # img = pygame.image.load('tank_night.png')
    # imgData = pygame.image.tostring(img, "RGB", 1)
    # width = img.get_width()
    # height = img.get_height()
    img = Image.open('tank_night.png')
    imgData = img.tostring('raw', 'RGBA', 0, -1)
    width = img.size[0]
    height = img.size[1]

    # gl.glViewport(0, 0, width, height)

    im = gl.glGenTextures(1)
    # gl.glPixelStorei(gl.GL_UNPACK_ALIGNMENT, 1)
    gl.glBindTexture(gl.GL_TEXTURE_2D, im)
    gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_S, gl.GL_CLAMP)
    gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_WRAP_T, gl.GL_CLAMP)
    gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MAG_FILTER, gl.GL_LINEAR)
    gl.glTexParameterf(gl.GL_TEXTURE_2D, gl.GL_TEXTURE_MIN_FILTER, gl.GL_LINEAR)
    gl.glTexImage2D(gl.GL_TEXTURE_2D, 0, gl.GL_RGB, width, height, 0, gl.GL_RGBA, gl.GL_UNSIGNED_BYTE, imgData)
    gl.glEnable(gl.GL_TEXTURE_2D)

    vertex = shaders.compileShader("""#version 130
    void main (void)
    {
        vec4 vertex = gl_Vertex;
        gl_Position = gl_ModelViewProjectionMatrix * vertex;
        gl_TexCoord[0] = gl_MultiTexCoord0;
    }""", gl.GL_VERTEX_SHADER)

    fragment = shaders.compileShader("""#version 130
    uniform sampler2D heatmap;
    uniform sampler1D colormap;
    void main (void)
    {
        // float temp = texture2D(heatmap, gl_TexCoord[1].st).r; // [0 - 50] degrees celcius
        // float r = temp/50.0f;
        // r = clamp(r, 0.0f, 1.0f);
        // gl_FragColor = texture1D(colormap, r);
    }""", gl.GL_FRAGMENT_SHADER)

    # vertex = shaders.compileShader("""#version 120
    # void main() {
    #     gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    # }""", gl.GL_VERTEX_SHADER)

    # fragment = shaders.compileShader("""#version 120
    # void main() {
    #     gl_FragColor = vec4( 0, 1, 0, 1 );
    # }""", gl.GL_FRAGMENT_SHADER)

    # FRAGMENT_SHADER = shaders.compileShader("""
    #     #version 120
    #     uniform sampler2D tex;
    #     void main()
    #     {
    #      vec4 pixcol = texture2D(tex, gl_TexCoord[0].xy);
    #      vec4 colors[3];
    #      colors[0] = vec4(0.,0.,1.,1.);
    #      colors[1] = vec4(1.,1.,0.,1.);
    #      colors[2] = vec4(1.,0.,0.,1.);
    #      float lum = (pixcol.r+pixcol.g+pixcol.b)/3.;
    #      int ix = (lum < 0.5)? 0:1;
    #      vec4 thermal = mix(colors[ix],colors[ix+1],(lum-float(ix)*0.5)/0.5);
    #      gl_FragColor = thermal;
    #     }
    #                                         """, gl.GL_FRAGMENT_SHADER)

    shader = shaders.compileProgram(vertex, fragment)

def display():
    # gl.glMatrixMode(gl.GL_PROJECTION);
    gl.glLoadIdentity()
    glu.gluPerspective(45, 1, 0, 1)
    # gl.glTranslatef(0, 0, 0)
    # gl.glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);

    gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
    # gl.glUseProgram(shader)

    x = 4
    y = 4
    z = 16
    gl.glBegin(gl.GL_QUADS)
    gl.glTexCoord2f(0, 0)
    gl.glVertex3f(-x, -y, -z)
    gl.glTexCoord2f(0, 1)
    gl.glVertex3f(-x, y, -z)
    gl.glTexCoord2f(1, 1)
    gl.glVertex3f(x, y, -z)
    gl.glTexCoord2f(1, 0)
    gl.glVertex3f(x, -y, -z)
    gl.glEnd()

    vertices = np.array([[0,1,0],[-1,-1,0],[1,-1,0]], dtype=np.uint16)
    gl.glVertexPointer(2, gl.GL_SHORT, 0, vertices)
    indices = [0, 1, 2, 3]
    gl.glDrawElements(gl.GL_QUADS, 1, gl.GL_UNSIGNED_SHORT, indices)

    gl.glFlush()
    print 'Display'


if __name__ == '__main__':
    glut.glutInit()
    glut.glutInitWindowSize(800, 600)
    glut.glutCreateWindow("Sample Shader")
    glut.glutInitDisplayMode(glut.GLUT_DOUBLE | glut.GLUT_RGB)
    glut.glutDisplayFunc(display)
    init()
    glut.glutMainLoop()


import OpenGL.GL as gl
import OpenGL.GLUT as glut
from OpenGL.arrays import vbo
from OpenGL.GL import shaders
import numpy as np

vertexPositions = None
indexPositions = None
shader = None
def init():
    global vertexPositions, indexPositions, shader
    #Create the VBO
    vertices = np.array([[0,1,0],[-1,-1,0],[1,-1,0]], dtype='f')
    vertexPositions = vbo.VBO(vertices)

    #Create the index buffer object
    indices = np.array([[0,1,2]], dtype=np.int32)
    indexPositions = vbo.VBO(indices, target=gl.GL_ELEMENT_ARRAY_BUFFER)

    VERTEX_SHADER = shaders.compileShader("""#version 120
    void main() {
        gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    }""", gl.GL_VERTEX_SHADER)

    FRAGMENT_SHADER = shaders.compileShader("""#version 120
    void main() {
        gl_FragColor = vec4( 0, 1, 0, 1 );
    }""", gl.GL_FRAGMENT_SHADER)

    shader = shaders.compileProgram(VERTEX_SHADER, FRAGMENT_SHADER)

def display():
    gl.glClear(gl.GL_COLOR_BUFFER_BIT | gl.GL_DEPTH_BUFFER_BIT)
    gl.glUseProgram(shader)

    indexPositions.bind()

    vertexPositions.bind()
    gl.glEnableVertexAttribArray(0);
    gl.glVertexAttribPointer(0, 3, gl.GL_FLOAT, False, 0, None)

    #glDrawArrays(GL_TRIANGLES, 0, 3) #This line still works
    gl.glDrawElements(gl.GL_TRIANGLES, 3, gl.GL_UNSIGNED_INT, None) #This line does work too!
    gl.glFlush()


if __name__ == '__main__':
    glut.glutInit()
    glut.glutInitWindowSize(800, 600)
    glut.glutCreateWindow("Sample Shader")
    glut.glutInitDisplayMode(glut.GLUT_DOUBLE | glut.GLUT_RGB)
    glut.glutDisplayFunc(display)
    init()
    glut.glutMainLoop()


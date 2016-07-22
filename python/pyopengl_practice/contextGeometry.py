from OpenGLContext import testingcontext, arrays
from OpenGL.arrays import vbo
import OpenGL.GL as gl

BaseContext = testingcontext.getInteractive()

class TestContext(BaseContext):
    """Creates a simple vertex shader..."""
    def OnInit(self):
        VERTEX_SHADER = gl.shaders.compileShader("""#version 120
        void main() {
            gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
        }""", gl.GL_VERTEX_SHADER)

        FRAGMENT_SHADER = gl.shaders.compileShader("""#version 120
        void main() {
            gl_FragColor = vec4( 0, 1, 0, 1 );
        }""", gl.GL_FRAGMENT_SHADER)

        self.shader = gl.shaders.compileProgram(VERTEX_SHADER,FRAGMENT_SHADER)

        self.vbo = vbo.VBO(
            array( [
                [  0, 1, 0 ],
                [ -1,-1, 0 ],
                [  1,-1, 0 ],
                [  2,-1, 0 ],
                [  4,-1, 0 ],
                [  4, 1, 0 ],
                [  2,-1, 0 ],
                [  4, 1, 0 ],
                [  2, 1, 0 ],
            ],'f')
        )

    def Render( self, mode):
        """Render the geometry for the scene."""
        gl.shaders.glUseProgram(self.shader)

        try:
            self.vbo.bind()
            try:
                gl.glEnableClientState(GL_VERTEX_ARRAY);
                gl.glVertexPointerf(self.vbo)
                gl.glDrawArrays(GL_TRIANGLES, 0, 9)
            finally:
                self.vbo.unbind()
                gl.glDisableClientState(GL_VERTEX_ARRAY);
        finally:
            gl.shaders.glUseProgram(0)

if __name__ == "__main__":
    TestContext.ContextMainLoop()


#include <GL/gl.h>
#include <GL/glut.h>

struct Vector {
    GLfloat x;
    GLfloat y;
    GLfloat z;
};

struct State {
    GLfloat length;
    Vector tVector;
    GLfloat alpha;
    GLfloat beta;
    GLfloat gamma;
    GLfloat scale;
} state;

void initState() {
    state.length = 1.0f;
    state.tVector.x = state.tVector.y = state.tVector.z = 0.0f;
    state.alpha = state.beta = state.gamma = 0.0f;
    state.scale = 1.0f;
}

void changeSize(int w, int h) {
	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
		h = 1;

	// Compute window's aspect ratio 
	float ratio = w * 1.0 / h;

	// Set the projection matrix as current
	glMatrixMode(GL_PROJECTION);
	// Load Identity Matrix
	glLoadIdentity();
	
	// Set the viewport to be the entire window
    glViewport(0, 0, w, h);

	// Set perspective
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

	// Return to the model view matrix mode
	glMatrixMode(GL_MODELVIEW);
}

void drawAxes() {
    glBegin(GL_LINES);
    
    // X axis in red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);
    
    // Y axis in green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    
    // Z axis in blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();
}


//Plane (a square in the XZ plane, centred in the origin, subdivided in both X and Z directions)
void drawPlane(int divisions) {
 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glColor3f(1.0f, 1.0f, 1.0f);

    glBegin(GL_TRIANGLES);

    int fixDiv = 0;
    float ofset = 0;
    if (divisions%2 == 1){
        fixDiv = 1;
        //TODO: set to length/2
        ofset = 0.5f; 
    }
    for (int i = -divisions/2 - fixDiv; i < divisions/2 ; i++) {
        for (int j = -divisions/2 - fixDiv; j < divisions/2 ; j++) {
            glVertex3f(state.length * j + ofset, 0.0f, state.length * i + ofset);
            glVertex3f(state.length * j + ofset, 0.0f, state.length * (i+1) + ofset);
            glVertex3f(state.length * (j+1) + ofset, 0.0f, state.length * i + ofset);
            glVertex3f(state.length * j + ofset, 0.0f, state.length * (i+1) + ofset);
            glVertex3f(state.length * (j+1) + ofset, 0.0f, state.length * (i+1) + ofset);
            glVertex3f(state.length * (j+1) + ofset, 0.0f, state.length * i + ofset);
        }
    }
    glEnd();
}

void renderScene(void) {
    // Clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Set the camera
    // TODO: Get values from config.xml
	glLoadIdentity();
	gluLookAt(3.0,2.0,1.0, 
		      0.0,0.0,0.0,
			  0.0f,1.0f,0.0f);

    drawAxes();
    
    // Geometric Transformations
    glTranslatef(state.tVector.x, state.tVector.y, state.tVector.z);
    glRotatef(state.alpha, 1.0f, 0.0f, 0.0f);
    glRotatef(state.beta, 0.0f, 1.0f, 0.0f);
    glRotatef(state.gamma, 0.0f, 0.0f, 1.0f);
    glScalef(state.scale, state.scale, state.scale);

    // Drawings
    drawPlane(5);

	// End of frame
	glutSwapBuffers();
}

// Simple translations, rotations and scale
void processKeys(unsigned char key, int xx, int yy) {
    switch (key) {
        case 'a':
            state.tVector.x += 0.5f; 
            break;
        case 'A':
            state.tVector.x -= 0.5f; 
            break;
        case 's':
            state.tVector.y += 0.5f; 
            break;
        case 'S':
            state.tVector.y -= 0.5f; 
            break;
        case 'd':
            state.tVector.z += 0.5f; 
            break;
        case 'D':
            state.tVector.z -= 0.5f; 
            break;
        case 'j':
            state.alpha += 5.0f; 
            break;
        case 'J':
            state.alpha -= 5.0f; 
            break;
        case 'k':
            state.beta += 5.0f; 
            break;
        case 'K':
            state.beta -= 5.0f; 
            break;
        case 'l':
            state.gamma += 5.0f; 
            break;
        case 'L':
            state.gamma -= 5.0f; 
            break;
        case 'i':
            state.scale += 0.1f;
            break;
        case 'I':
            state.scale -= 0.1f;
            break;
        case 'r':
            state.alpha = state.beta = state.gamma = 0.0f;
            state.tVector.x = state.tVector.y = state.tVector.z = 0.0f;
            state.scale = 1.0f;
            break;
    }
    
    glutPostRedisplay();
}

int main(int argc, char **argv) {

    initState();

    // Init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
    // Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	
    glutKeyboardFunc(processKeys);

    //  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	
    // Enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}

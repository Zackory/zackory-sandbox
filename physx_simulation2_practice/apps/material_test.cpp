


#include <iostream>
#include <fstream>
#include <PxPhysicsAPI.h> //Single header file to include all features of PhysX API

// Include OpenGL based on OS 
#ifdef __APPLE__
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <OpenGL/glext.h>
#include <GLUT/glut.h>
#else
#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>
#include <GL/glut.h>
#endif

#include <vector>
#include <list>
#include <time.h>
#include <Eigen/Dense>
#include "config.h"

#include "simulator/MatSimulator.h"
#include "renderer/Renderer.h"
#include "myUtils/ScreenShot.h"

#include "visualization/CurvePlotter.h"
#include "gui/Slider.h"

using namespace std;
using namespace physx;
using namespace Eigen;


//========== Global variables ============//

int gWindowWidth  = 800; //Screen width
int gWindowHeight = 600; //Screen height


//---Scene navigation----
int gOldMouseX = 0;
int gOldMouseY = 0;

bool isMouseLeftBtnDown  = false;
bool isMouseRightBtnDown = false;

float gCamRoateX	= 15;
float gCamRoateY	= 0;
float gCamDistance	= -2;
PxVec3 gCamTranslate = PxVec3(0);

//---Simulation related variables----
bool simulating = false;
bool recording = false;

//---Simulator----
MatSimulator simulator;
int simulated_steps = 0;
//--------------------------

//---Mock Experiment----
int experiment_id = 0;
//--------------------------

//---Timing-------------
clock_t current_exp_duration = 0;
vector<double> exp_duration;
//----------------------

//---GUI stuff----------
vector<Slider> sliders;
vector<CurvePlotter> plotters;

// Functions for initialization
void initialize();

//Functions for glut callbacks
void OnRender();					//Display callback for the current glut window
void OnIdle();						//Called whenever the application is idle
void OnReshape(int, int);			//Called whenever the application window is resized
void OnShutdown();					//Called on application exit
void OnMouseMotion(int,int);		//Called when the mouse is moving
void OnMousePress(int,int,int,int); //Called when any mouse button is pressed
void OnKeyPress(unsigned char, int, int); // Called when a key is pressed

int main(int argc, char** argv)
{
    glutInit(&argc, argv);								//Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);		//Enable double buffering
    glutInitWindowSize(gWindowWidth, gWindowHeight);	//Set window's initial width & height
    
    glutCreateWindow("Learning Physics Modeling with PhysX (ISBN: 978-1-84969-814-6) Examples"); // Create a window with the given title
    
    glutDisplayFunc(OnRender);	//Display callback for the current glut window
    glutIdleFunc(OnIdle);		//Called whenever the application is idle
    glutReshapeFunc(OnReshape); //Called whenever the app window is resized
    
    glutMouseFunc(OnMousePress);	//Called on mouse button event
    glutMotionFunc(OnMouseMotion);	//Called on mouse motion event
    glutKeyboardFunc(OnKeyPress);
    
    simulator.initialize();
    
    initialize();
    
    glutMainLoop();				//Enter the event-processing loop
    atexit(OnShutdown);			//Called on application exit
    
    return 0;
}

void initialize() {
    float stx = 0.03;
    float sty = 0.03;
    float h = 0.02;
    float w = 0.5;
    float dif = 0.025;
    /*sliders.resize(10);
    sliders[0].initialize(stx, sty, w, h, 1, 0.0, 0.3, "Friction");
    sliders[0].setRelatedValue(&simulator.cloth.friction);
    
    sliders[1].initialize(stx, sty+dif, w, h, 1, 0.0, 0.0, "SelfFriction");
    sliders[1].setRelatedValue(&simulator.cloth.friction);
    
    sliders[2].initialize(stx, sty+dif*2, w, h, 0.03, 0.0, 0.015, "SC distance");
    sliders[2].setRelatedValue(&simulator.cloth.self_collision_distance);
    
    sliders[3].initialize(stx, sty+dif*3, w, h, 1, 0.0, 1.0, "SC stiff");
    sliders[3].setRelatedValue(&simulator.cloth.self_collision_stiff);
    
    sliders[4].initialize(stx, sty+dif*4, w, h, 1, 0.0, 1.0, "Vertical Stiff");
    sliders[4].setRelatedValue(&simulator.cloth.vstretch_stiff[0]);
    
    sliders[5].initialize(stx, sty+dif*5, w, h, 1, 0.0, 1.0, "Horizontal Stiff");
    sliders[5].setRelatedValue(&simulator.cloth.hstretch_stiff[0]);
    
    sliders[6].initialize(stx, sty+dif*6, w, h, 1, 0.0, 1.0, "Bending");
    sliders[6].setRelatedValue(&simulator.cloth.bend_stiff[0]);
    
    sliders[7].initialize(stx, sty+dif*7, w, h, 1, 0.0, 1.0, "Shear");
    sliders[7].setRelatedValue(&simulator.cloth.shear_stiff[0]);
    
    sliders[8].initialize(stx, sty+dif*8, w, h, 100, 1.0, 1.0, "Iteration Number");
    sliders[8].setRelatedValue(&simulator.iteration_number);
    
    sliders[9].initialize(stx, sty+dif*9, w, h, 1, -1, 0.1, "Damping (y)");
    sliders[9].setRelatedValue(&simulator.cloth.damping[1]);*/
    
    
    plotters.resize(6);
    plotters[0].setColor(Vector3d(0.3, 0.6, 1));
    plotters[0].setScale(Vector2d(-5, 0.2), Vector2d(-1, 1));
    plotters[0].setLineWidth(2);
    plotters[0].toggleDyanmicChange();
    plotters[0].setScreenPlace(Vector2d(0.01, 0.01), Vector2d(0.32, 0.07));
    plotters[0].setTitle("force X");
    
    plotters[1].setColor(Vector3d(0.3, 0.6, 1));
    plotters[1].setScale(Vector2d(-5, 0.2), Vector2d(-1, 1));
    plotters[1].setLineWidth(2);
    plotters[1].toggleDyanmicChange();
    plotters[1].setScreenPlace(Vector2d(0.33, 0.01), Vector2d(0.65, 0.07));
    plotters[1].setTitle("force Y");
    
    plotters[2].setColor(Vector3d(0.3, 0.6, 1));
    plotters[2].setScale(Vector2d(-5, 0.2), Vector2d(-1, 1));
    plotters[2].setLineWidth(2);
    plotters[2].toggleDyanmicChange();
    plotters[2].setScreenPlace(Vector2d(0.66, 0.01), Vector2d(0.98, 0.07));
    plotters[2].setTitle("force Z");
    
    plotters[3].setColor(Vector3d(0.3, 0.6, 1));
    plotters[3].setScale(Vector2d(-5, 0.2), Vector2d(-0.1, 0.1));
    plotters[3].setLineWidth(2);
    plotters[3].toggleDyanmicChange();
    plotters[3].setScreenPlace(Vector2d(0.01, 0.08), Vector2d(0.32, 0.14));
    plotters[3].setTitle("torque X");
    
    plotters[4].setColor(Vector3d(0.3, 0.6, 1));
    plotters[4].setScale(Vector2d(-5, 0.2), Vector2d(-0.1, 0.1));
    plotters[4].setLineWidth(2);
    plotters[4].toggleDyanmicChange();
    plotters[4].setScreenPlace(Vector2d(0.33, 0.08), Vector2d(0.65, 0.14));
    plotters[4].setTitle("torque Y");
    
    plotters[5].setColor(Vector3d(0.3, 0.6, 1));
    plotters[5].setScale(Vector2d(-5, 0.2), Vector2d(-0.1, 0.1));
    plotters[5].setLineWidth(2);
    plotters[5].toggleDyanmicChange();
    plotters[5].setScreenPlace(Vector2d(0.66, 0.08), Vector2d(0.98, 0.14));
    plotters[5].setTitle("torque Z");
}

void directional_light (int i, const float *dir, const float *dif) {
    float diffuse[4] = {dif[0], dif[1], dif[2], 1};
    float position[4] = {dir[0], dir[1], dir[2], 0};
    glEnable(GL_LIGHT0+i);
    glLightfv(GL_LIGHT0+i, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0+i, GL_POSITION, position);
}

void ambient_light (const float* a) {
    float ambient[4] = {a[0], a[1], a[2], 1};
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
}

void updatePlotters() {
    if (simulator.recorded_time.size() > 0) {
        plotters[0].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_forces[simulator.rig_parts[1].recorded_forces.size()-1].x));
        plotters[1].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_forces[simulator.rig_parts[1].recorded_forces.size()-1].y));
        plotters[2].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_forces[simulator.rig_parts[1].recorded_forces.size()-1].z));
        
        plotters[3].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_torques[simulator.rig_parts[1].recorded_torques.size()-1].x));
        plotters[4].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_torques[simulator.rig_parts[1].recorded_torques.size()-1].y));
        plotters[5].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_torques[simulator.rig_parts[1].recorded_torques.size()-1].z));
    }
    
}

void OnRender()
{
    if (simulating) {
        clock_t time_pre_sim = clock();
        simulator.simulate(3);
        current_exp_duration += clock() - time_pre_sim;
        simulated_steps++;
        
        updatePlotters();
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.7, 0.7, 0.7, 1);
    glEnable(GL_COLOR_MATERIAL);
    glLoadIdentity();
    
    float dir[3] = {10,-10,0};
    float dif[3] = {0.9, 0.9, 0.9};
    directional_light(0, dir, dif);
    ambient_light(dif);
    glEnable(GL_LIGHTING);
    
    glTranslatef(gCamTranslate.x, -gCamTranslate.y, gCamTranslate.z);
    glTranslatef(0,0,gCamDistance);
    glRotatef(gCamRoateX,1,0,0);
    glRotatef(gCamRoateY,0,1,0);
    glEnable(GL_DEPTH_TEST);
    
    // render scene
    Renderer renderer;
    glEnable(GL_LIGHTING);
    double cloth_color[3] = {0.6, 0.5, 0.4};
    double obj_color[3] = {0.4, 0.5, 0.7};
    glEnable(GL_CULL_FACE);
    renderer.renderMesh(simulator.cloth.cloth_mesh, cloth_color, false);
    cloth_color[0] = 0.3;
    cloth_color[2] = 0.6;
    renderer.renderMesh(simulator.cloth.cloth_mesh, cloth_color, false, true);
    glDisable(GL_CULL_FACE);
    for (int i = 0; i < simulator.rig_parts.size(); i++) {
        renderer.renderRigPart(&simulator.rig_parts[i], obj_color);
    }
    
    glDisable(GL_LIGHTING);
    
    if (recording && simulating && simulated_steps % 4 == 0) {
        string filename = string(PHYSX_ROOT_PATH)+"/Output/Images/";
        if (simulated_steps < 10) {
            filename = filename + "0000" + to_string(simulated_steps);
        } else if (simulated_steps < 100) {
            filename = filename + "000" + to_string(simulated_steps);
        } else if (simulated_steps < 1000) {
            filename  = filename + "00" + to_string(simulated_steps);
        } else if (simulated_steps < 10000) {
            filename  = filename + "0" + to_string(simulated_steps);
        }
        
        filename = filename + ".png";
        
        save_screenshot(filename);
    }
    
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    int max_size = max(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    gluOrtho2D(0, 1.0*glutGet(GLUT_WINDOW_WIDTH)/max_size, 0, 1.0 * glutGet(GLUT_WINDOW_HEIGHT)/max_size);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glScalef(1, -1, 1);
    glTranslatef(0, -glutGet(GLUT_WINDOW_HEIGHT)*1.0/max_size, 0);
    for (int i = 0; i < sliders.size(); i++) {
        sliders[i].Render();
    }
    
    for (int i = 0; i < plotters.size(); i++) {
        plotters[i].draw();
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    
    glutSwapBuffers();
}

void OnReshape(int w, int h)
{
    glViewport(0,0,w,h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60, (GLfloat)w / (GLfloat)h, 0.1f, 100000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void OnIdle()
{
    glutPostRedisplay();
}

void OnShutdown()
{
    simulator.destroy();
}

void OnMouseMotion(int curMouseX, int curMouseY)
{
    int max_size = max(glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
    if(isMouseLeftBtnDown)
    {
        bool ui_cont = false;
        for (int i = 0; i < sliders.size(); i++) {
            if (sliders[i].IsClicked(curMouseX*1.0/max_size, curMouseY * 1.0/max_size)) {
                ui_cont = true;
                break;
            }
        }
        if (!ui_cont) {
            gCamRoateY += (curMouseX - gOldMouseX)/5.0f;
            gCamRoateX += (curMouseY - gOldMouseY)/5.0f;
        } else {
            simulator.cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eVERTICAL,PxClothStretchConfig(simulator.cloth.vstretch_stiff[0], simulator.cloth.vstretch_stiff[1], simulator.cloth.vstretch_stiff[2], simulator.cloth.vstretch_stiff[3]));
            simulator.cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, PxClothStretchConfig(simulator.cloth.hstretch_stiff[0], simulator.cloth.hstretch_stiff[1], simulator.cloth.hstretch_stiff[2], simulator.cloth.hstretch_stiff[3]));
            simulator.cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eSHEARING,PxClothStretchConfig(simulator.cloth.shear_stiff[0], simulator.cloth.shear_stiff[1], simulator.cloth.shear_stiff[2], simulator.cloth.shear_stiff[3]));
            simulator.cloth.mCloth->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(simulator.cloth.bend_stiff[0], simulator.cloth.bend_stiff[1], simulator.cloth.bend_stiff[2], simulator.cloth.bend_stiff[3]));
            simulator.cloth.mCloth->setFrictionCoefficient(simulator.cloth.friction);
            simulator.cloth.mCloth->setSelfFrictionCoefficient(simulator.cloth.self_friction);
            simulator.cloth.mCloth->setDampingCoefficient(PxVec3(simulator.cloth.damping[0], simulator.cloth.damping[1], simulator.cloth.damping[2]));
            
            simulator.cloth.mCloth->setSelfCollisionDistance(simulator.cloth.self_collision_distance);
            simulator.cloth.mCloth->setSelfCollisionStiffness(simulator.cloth.self_collision_stiff);
        }
    }
    
    if(isMouseRightBtnDown)
    {
        gCamDistance -= (curMouseY - gOldMouseY)/25.0f;
    }
    
    gOldMouseX = curMouseX;
    gOldMouseY = curMouseY;
    
}


void OnMousePress(int mouseBtn, int mouseBtnState, int curMouseX, int curMouseY)
{
    if (mouseBtnState == GLUT_DOWN)
    {
        if(mouseBtn== GLUT_LEFT_BUTTON)
        isMouseLeftBtnDown = true;
        
        else if(mouseBtn == GLUT_RIGHT_BUTTON)
        isMouseRightBtnDown = true;
        
        gOldMouseX = curMouseX;
        gOldMouseY = curMouseY;
        
    }
    
    if (mouseBtnState == GLUT_UP )
    {
        isMouseLeftBtnDown = false;
        isMouseRightBtnDown = false;
    }
}


void OnKeyPress(unsigned char key, int x, int y) {
    // used keys: ' ', 's', 'r', 'R', 'f'
    // ------ basic control ------- //
    if (key == 27) {
        exit(EXIT_SUCCESS);
    } else if (key == ' ') {
        simulating = !simulating;
    } else if (key == 's') {
        simulating = true;
        OnRender();
        simulating = false;
    } else if (key == 'R') {
        recording = !recording;
    } else if (key == 'r') {
        simulator.reset();
        for (int i = 0; i < plotters.size(); i++){
            plotters[i].clearData();
        }
        simulating = false;
    } else if (key == 'f') {
        string pathname = string(PHYSX_ROOT_PATH)+"Output/tensile_test/";
        ofstream ofile((pathname+"test.txt").c_str());
        for (int i = 0; i < simulator.recorded_strain.size(); i++) {
            ofile << simulator.recorded_strain[i] << " " << abs(simulator.rig_parts[1].recorded_forces[i].x) + abs(simulator.rig_parts[2].recorded_forces[i].x) << endl;
        }
        ofile.close();
    } else if (key == '+') {    // change display scale
        for (int i = 0; i < plotters.size(); i++) {
            Vector2d xscale = plotters[i].getXscale();
            Vector2d yscale = plotters[i].getYscale();
            plotters[i].setScale(xscale, yscale/1.3);
        }
    } else if (key == '-') {
        for (int i = 0; i < plotters.size(); i++) {
            Vector2d xscale = plotters[i].getXscale();
            Vector2d yscale = plotters[i].getYscale();
            plotters[i].setScale(xscale, yscale*1.3);
        }
    }
    // ---------------------------- //
}




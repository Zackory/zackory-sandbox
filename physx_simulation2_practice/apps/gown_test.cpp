


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

#include "simulator/gownSimulator.h"
#include "renderer/Renderer.h"
#include "myUtils/ScreenShot.h"
#include "CMAES/HapticObjective.h"
#include "visualization/CurvePlotter.h"
#include "config.h"

#include <boost/filesystem.hpp>

using namespace std;
using namespace Eigen;
using namespace physx;


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
gownSimulator simulator;
int simulated_steps = 0;
//--------------------------

//---Mock Experiment----
int experiment_id = 0;
int total_experiment = 100;
//--------------------------

//---Timing-------------
clock_t current_exp_duration = 0;
vector<double> exp_duration;
//----------------------

//---Rendering Related-----
bool show_obstacle = true;
//-------------------------

//---Objective Related-----
HapticObjective simp_good;
HapticObjective simp_miss;
//-------------------------

//---Visualization----------
vector<CurvePlotter> plotters;
//--------------------------

//Functions for glut callbacks
void OnRender();					//Display callback for the current glut window
void OnIdle();						//Called whenever the application is idle
void OnReshape(int, int);			//Called whenever the application window is resized
void OnShutdown();					//Called on application exit
void OnMouseMotion(int,int);		//Called when the mouse is moving
void OnMousePress(int,int,int,int); //Called when any mouse button is pressed
void OnKeyPress(unsigned char, int, int); // Called when a key is pressed

void initialize() {
    simulator.initialize();
    simp_good.ReadData(string(PHYSX_ROOT_PATH)+"/Data/caughtmiss/subject7/1.txt");
    //simp_good.ReadData("Data/gown_good.txt");
    simp_miss.ReadData(string(PHYSX_ROOT_PATH)+"/Data/miss/subject4/2.txt");
    
    plotters.resize(4);
    plotters[0].setColor(Vector3d(0.3, 0.6, 1));
    plotters[0].setScale(Vector2d(-0.1, 10), Vector2d(-1, 10));
    plotters[0].setLineWidth(2);
    //plotters[0].toggleDyanmicChange();
    plotters[0].setScreenPlace(Vector2d(0.01, 0.01), Vector2d(0.49, 0.09));
    plotters[0].setTitle("force X");
    
    plotters[1].setColor(Vector3d(0.3, 0.6, 1));
    plotters[1].setScale(Vector2d(-0.1, 10), Vector2d(-5, 2));
    plotters[1].setLineWidth(2);
    //plotters[1].toggleDyanmicChange();
    plotters[1].setScreenPlace(Vector2d(0.5, 0.01), Vector2d(0.99, 0.09));
    plotters[1].setTitle("force Y");
    
    plotters[2].setColor(Vector3d(1, 0.6, 0.3));
    plotters[2].setScale(Vector2d(-0.1, 10), Vector2d(-1, 10));
    plotters[2].setLineWidth(2);
    plotters[2].setScreenPlace(Vector2d(0.01, 0.1), Vector2d(0.49, 0.18));
    plotters[2].setTitle("exp force X");
    
    plotters[3].setColor(Vector3d(1, 0.6, 0.3));
    plotters[3].setScale(Vector2d(-0.1, 10), Vector2d(-5, 2));
    plotters[3].setLineWidth(2);
    plotters[3].setScreenPlace(Vector2d(0.5, 0.1), Vector2d(0.99, 0.18));
    plotters[3].setTitle("exp force Y");
    
    // set the random seed
    srand( (unsigned int) time (NULL) );
}

void updatePlotters() {
    if (simulator.recorded_time.size() > 0) {
        plotters[0].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_forces[simulator.rig_parts[1].recorded_forces.size()-1].x));
        plotters[1].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], simulator.rig_parts[1].recorded_forces[simulator.rig_parts[1].recorded_forces.size()-1].y));
        
        // exp data
        int ind1, ind2;
        double frac1, frac2;
        HapticObjective& hap_obj = simp_miss;
        hap_obj.getInterpolationFrac(hap_obj.exp_time, simulator.recorded_time[simulator.recorded_time.size()-1] - simulator.recorded_time[0], ind1, frac1, ind2, frac2);
        double xforce = -(hap_obj.exp_fmove[ind1] * frac1 + hap_obj.exp_fmove[ind2] * frac2);
        double yforce = (hap_obj.exp_fgravity[ind1] * frac1 + hap_obj.exp_fgravity[ind2] * frac2);
        
        plotters[2].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], xforce));
        plotters[3].updateData(Vector2d(simulator.recorded_time[simulator.recorded_time.size()-1], yforce));
    }
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);								//Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);		//Enable double buffering
    glutInitWindowSize(gWindowWidth, gWindowHeight);	//Set window's initial width & height
    
    glutCreateWindow("Learning Physics Modeling with PhysX (ISBN: 978-1-84969-814-6) Examples"); // Create a window with the given title
    
    initialize();
    
    glutDisplayFunc(OnRender);	//Display callback for the current glut window
    glutIdleFunc(OnIdle);		//Called whenever the application is idle
    glutReshapeFunc(OnReshape); //Called whenever the app window is resized
    
    glutMouseFunc(OnMousePress);	//Called on mouse button event
    glutMotionFunc(OnMouseMotion);	//Called on mouse motion event
    glutKeyboardFunc(OnKeyPress);
    
    glutMainLoop();				//Enter the event-processing loop
    atexit(OnShutdown);			//Called on application exit
    
    return 0;
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


void OnRender()
{
    if (simulating) {
        clock_t time_pre_sim = clock();
        simulator.simulate(3);
        current_exp_duration += clock() - time_pre_sim;
        simulated_steps++;
        if (simulator.simulated_step < -1) {
            OnKeyPress('f', 0, 0);
            OnKeyPress('r', 0, 0);
            experiment_id++;
            exp_duration.push_back(((double)current_exp_duration)/CLOCKS_PER_SEC);
            current_exp_duration = 0;
            if (experiment_id == total_experiment) {
                double sum = 0;
                for (int i = 0; i < exp_duration.size(); i++) {
                    cout << "Experiment " << i << " took: " << exp_duration[i] << " seconds." << endl;
                    sum += exp_duration[i];
                }
                cout << "Total: " << sum << " seconds." << endl;
                exit(EXIT_SUCCESS);
            }
            // change speed
            if (experiment_id > total_experiment / 2) {
                simulator.moving_speed = 0.15;
                simulator.move_step[0] = - simulator.moving_speed * simulator.gTimeStep;
            }
            //exit(EXIT_SUCCESS);
        }
        updatePlotters();
    }
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_COLOR_MATERIAL);
    glLoadIdentity();
    
    float dir[3] = {10,10,0};
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
    renderer.renderMesh(simulator.cloth.cloth_mesh, cloth_color);
    if (show_obstacle) {
        for (int i = 0; i < simulator.rig_parts.size(); i++) {
            renderer.renderRigPart(&simulator.rig_parts[i], obj_color);
        }
        
        renderer.renderSphereManager(obj_color);
    }
    
    glDisable(GL_LIGHTING);
    
    // draw texts or 2d graphics
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
    for (int i = 0; i < plotters.size(); i++) {
        plotters[i].draw();
    }
    
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glEnable(GL_DEPTH_TEST);
    
    if (recording && simulating && simulated_steps % 4 == 0) {
        string filename = string(PHYSX_ROOT_PATH)+"/Output/Images/cma_together/all_height"+to_string(experiment_id)+"/";
        
        //if (simulated_steps < 10) {
            boost::filesystem::path p{filename.c_str()};
            try
            {
                boost::filesystem::create_directory(p);
            }
            catch (boost::filesystem::filesystem_error &e)
            {
                std::cerr << e.what() << '\n';
            }
        //}
        
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
    if(isMouseLeftBtnDown)
    {
        gCamRoateY += (curMouseX - gOldMouseX)/5.0f;
        gCamRoateX += (curMouseY - gOldMouseY)/5.0f;
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
    
    //cout<<mouseBtn<<" "<<mouseBtnState<<" "<<x<<"|"<<y<<"\n";
}


void OnKeyPress(unsigned char key, int x, int y) {
    // used keys: ' ', 's', 'r', 'R', 'f'
    // ------ basic control ------- //
    if (key == 27) {
        exit(EXIT_SUCCESS);
    } else if (key == ' ') {
        for (int i = 0; i < 4; i++) {
            double randx = (rand()*1.0/RAND_MAX-0.5)/10;
            double randy = (rand()*1.0/RAND_MAX-0.5)/10;
            double randz = (rand()*1.0/RAND_MAX-0.5)/10;
        }
        simulating = !simulating;
    } else if (key == 's') {
        simulating = true;
        OnRender();
        simulating = false;
    } else if (key == 'R') {
        recording = !recording;
    } else if (key == 'r') {
        simulator.reset();
        simulator.simulate(1);
        simulator.reset();
        simulated_steps = 0;
        for (int i = 0; i < plotters.size(); i++) {
            plotters[i].clearData();
        }
    } else if (key == 't') {
        simulator.testbit = !simulator.testbit;
        if (simulator.testbit) {
            simulator.cloth.stiffpower = 10;
        } else {
            simulator.cloth.stiffpower = 1;
        }
        simulator.cloth.setParameters();
    } else if (key == 'o') {
        show_obstacle = !show_obstacle;
    }  else if (key == '+') {    // change display scale
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
    
    // ------ output gripper force sensor data ------ //
    if (key == 'f') {
        string pathname = "Output/gown_experiment/sim_gen2/subject4_newhdamp_dist/";
        boost::filesystem::path p{pathname.c_str()};
        try
        {
            boost::filesystem::create_directory(p);
        }
        catch (boost::filesystem::filesystem_error &e)
        {
            std::cerr << e.what() << '\n';
        }
        
        // don't record if arm_height_perturb is outside [0, -0.05], [-0.08, -0.14], [-0.17, -0.210]
        /*if (simulator.arm_height_perturb > 0 || (simulator.arm_height_perturb < -0.05 && simulator.arm_height_perturb > -0.08) || (simulator.arm_height_perturb < -0.14 && simulator.arm_height_perturb > -0.17) || simulator.arm_height_perturb < -0.21) {
            return;
        }*/
        
        // use lower ring of the sleeve (vertex 150, 153, 146, 143, 141) to decide the outcome
        
        if (simulator.rig_parts[1].components[1]->getGlobalPose().p.x-0.04 < -simulator.forearm_length+0.44-0.85) {
            pathname = pathname + "miss/ms";
            cout << "missed" << endl;
        } else {
            double max_x = -10000;
            for (int vid = 137; vid < 164; vid++) {
                if (max_x < simulator.cloth.cloth_mesh.particles[vid].pos(0)) {
                    max_x = simulator.cloth.cloth_mesh.particles[vid].pos(0);
                }
            }
            cout << "Metric: " << max_x+simulator.forearm_length << endl;
            if (max_x+simulator.forearm_length < 0.06) {
                pathname = pathname + "good/ms";
                cout << "good" << endl;
            }
            else {
                pathname = pathname + "caught/ms";
                cout << "caught" << endl;
            }
        }
        
        if (simulator.moving_speed == 0.15) {
            pathname = pathname + "f";
        }
        
        ofstream ofile((pathname+to_string(experiment_id)+".txt").c_str());
        for (int i = 0; i < simulator.rig_parts[1].recorded_forces.size(); i++) {
            ofile << simulator.recorded_time[i] << "\t"
            << simulator.recorded_positiosn[i] << "\t"
            << -simulator.rig_parts[1].recorded_forces[i][0] << "\t"
            << simulator.rig_parts[1].recorded_forces[i][1] << "\t"
            << simulator.rig_parts[1].recorded_forces[i][2] << "\t"
            << simulator.rig_parts[1].recorded_torques[i][0] << "\t"
            << simulator.rig_parts[1].recorded_torques[i][1] << "\t"
            << simulator.rig_parts[1].recorded_torques[i][2] << endl;
        }
        ofile.close();
        
        double val = 0;
        if (pathname.find("miss") != std::string::npos) {
            val += simp_miss.evalObjective(&simulator);
        } else if (pathname.find("good") != std::string::npos) {
            val += simp_good.evalObjective(&simulator);
        }
        val = simp_miss.evalObjective(&simulator);
        cout << "Miss Value: " << val << endl;
        
        
        // output the material setup used and the objective value calculated
        ofstream ofile2((pathname+to_string(experiment_id)+"_material").c_str());
        ofile2 << "clothmat:" << endl;
        ofile2 << simulator.cloth.vstretch_stiff[0] << " " << simulator.cloth.hstretch_stiff[0] << " " << simulator.cloth.bend_stiff[0] << " " << simulator.cloth.shear_stiff[0] << endl;
        ofile2 << "damping: " << simulator.cloth.damping[0] << " " << simulator.cloth.damping[1] << " " << simulator.cloth.damping[2] << endl;
        ofile2 << "friction and self friction:" << endl;
        ofile2 << simulator.cloth.mCloth->getFrictionCoefficient() << " " << simulator.cloth.mCloth->getSelfCollisionDistance() << " " << simulator.cloth.mCloth->getSelfCollisionStiffness() << " " << simulator.cloth.mCloth->getSelfFrictionCoefficient() << endl;
        ofile2 << "stiff power: " << simulator.cloth.mCloth->getStiffnessPower() << endl;
        ofile2 << "iteration: " << simulator.cloth_solve_iteration_aftergrip << endl;
        ofile2 << "Arm height: " << simulator.arm_height << endl;
        ofile2 << "Fist Radius: " << simulator.fist_radius << endl;
        ofile2 << "Wirst Radius: " << simulator.wrist_radius << endl;
        ofile2 << "Elbow Radius: " << simulator.elbow_radius << endl;
        ofile2 << "Shoulder Radius: " << simulator.shoulder_radius << endl;
        ofile2 << "Forearm Length: " << simulator.forearm_length << endl;
        ofile2 << "Upperarm Length: " << simulator.upperearm_length << endl;
        ofile2 << "Obj val: " << val << endl;
        ofile2.close();
    }
    // ---------------------------------------------- //
}




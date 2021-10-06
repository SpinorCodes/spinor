/// @file     main.cpp
/// @author   Erik ZORZIN
/// @date     12JAN2021
/// @brief    Single 1/2 spinor.

#define INTEROP       true                                                                           // "true" = use OpenGL-OpenCL interoperability.
#define SX            800                                                                            // Window x-size [px].
#define SY            600                                                                            // Window y-size [px].
#define NAME          "Neutrino - Spinor"                                                            // Window name.
#define OX            0.0f                                                                           // x-axis orbit initial rotation.
#define OY            0.0f                                                                           // y-axis orbit initial rotation.
#define PX            0.0f                                                                           // x-axis pan initial translation.
#define PY            0.0f                                                                           // y-axis pan initial translation.
#define PZ            -2.0f                                                                          // z-axis pan initial translation.
#define ROT           0.01f

#ifdef __linux__
  #define SHADER_HOME "../../Code/shader/"                                                           // Linux OpenGL shaders directory.
  #define KERNEL_HOME "../../Code/kernel/"                                                           // Linux OpenCL kernels directory.
  #define GMSH_HOME   "../../Code/mesh/"                                                             // Linux GMSH mesh directory.
#endif

#ifdef WIN32
  #define SHADER_HOME "..\\..\\Code\\shader\\"                                                       // Windows OpenGL shaders directory.
  #define KERNEL_HOME "..\\..\\Code\\kernel\\"                                                       // Windows OpenCL kernels directory.
  #define GMSH_HOME   "..\\..\\Code\\mesh\\"                                                         // Linux GMSH mesh directory.
#endif

#define SHADER_VERT   "voxel_vertex.vert"                                                            // OpenGL vertex shader.
#define SHADER_GEOM   "voxel_geometry.geom"                                                          // OpenGL geometry shader.
#define SHADER_FRAG   "voxel_fragment.frag"                                                          // OpenGL fragment shader.
#define OVERLAY_VERT  "overlay_vertex.vert"                                                          // OpenGL vertex shader.
#define OVERLAY_GEOM  "overlay_geometry.geom"                                                        // OpenGL geometry shader.
#define OVERLAY_FRAG  "overlay_fragment.frag"                                                        // OpenGL fragment shader.
#define KERNEL_1      "spinor_kernel_1.cl"                                                           // OpenCL kernel source.
#define KERNEL_2      "spinor_kernel_2.cl"                                                           // OpenCL kernel source.
#define KERNEL_3      "spinor_kernel_3.cl"                                                           // OpenCL kernel source.
#define KERNEL_4      "spinor_kernel_4.cl"                                                           // OpenCL kernel source.
#define UTILITIES     "utilities.cl"                                                                 // OpenCL utilities source.
#define MESH_FILE     "spacetime.msh"                                                                // GMSH mesh.
#define MESH          GMSH_HOME MESH_FILE                                                            // GMSH mesh (full path).

// INCLUDES:
#include "nu.hpp"                                                                                    // Neutrino header file.

///////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////// GLOBAL VARIABLES ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
// OPENGL:
nu::opengl*                      gl             = new nu::opengl (NAME, SX, SY, OX, OY, PX, PY, PZ); // OpenGL context.
nu::shader*                      shader_1       = new nu::shader ();                                 // OpenGL shader program.

// OPENCL:
nu::opencl*                      cl             = new nu::opencl (NU_GPU);                           // OpenCL context.
nu::kernel*                      kernel_1       = new nu::kernel ();                                 // OpenCL kernel array.
nu::kernel*                      kernel_2       = new nu::kernel ();                                 // OpenCL kernel array.
nu::kernel*                      kernel_3       = new nu::kernel ();                                 // OpenCL kernel array.
nu::kernel*                      kernel_4       = new nu::kernel ();                                 // OpenCL kernel array.

nu::float4*                      position       = new nu::float4 (0);                                // vec4(position.xyz [m], freedom []).
nu::float4*                      velocity       = new nu::float4 (1);                                // vec4(velocity.xyz [m/s], friction [N*s/m]).
nu::float4*                      velocity_int   = new nu::float4 (2);                                // vec4(velocity.xyz (intermediate) [m/s], number of 1st + 2nd nearest neighbours []).
nu::float4*                      velocity_est   = new nu::float4 (3);                                // vec4(velocity.xyz (estimation) [m/s], radiative energy [J]).
nu::float4*                      acceleration   = new nu::float4 (4);                                // vec4(acceleration.xyz [m/s^2], mass [kg]).

nu::float4*                      color          = new nu::float4 (5);                                // vec4(color.xyz [], alpha []).
nu::float1*                      stiffness      = new nu::float1 (6);                                // Stiffness.
nu::float1*                      resting        = new nu::float1 (7);                                // Resting.
nu::int1*                        central        = new nu::int1 (8);                                  // Central nodes.
nu::int1*                        neighbour      = new nu::int1 (9);                                  // Neighbour.
nu::int1*                        offset         = new nu::int1 (10);                                 // Offset.

nu::int1*                        spinor         = new nu::int1 (11);                                 // Spinor.
nu::int1*                        spinor_num     = new nu::int1 (12);                                 // Spinor cells number.
nu::float4*                      spinor_pos     = new nu::float4 (13);                               // Spinor cells position.
nu::int1*                        frontier       = new nu::int1 (14);                                 // Spacetime frontier.
nu::int1*                        frontier_num   = new nu::int1 (15);                                 // Frontier nodes number.
nu::float4*                      frontier_pos   = new nu::float4 (16);                               // Frontier nodes position.

nu::float1*                      dispersion     = new nu::float1 (17);                               // Dispersion fraction [-0.5...1.0].
nu::float1*                      dt             = new nu::float1 (18);                               // Time step [s].

// MESH:
nu::mesh*                        spacetime      = new nu::mesh (MESH);                               // Spacetime mesh.
size_t                           nodes          = 0;                                                 // Number of nodes.
size_t                           elements       = 0;                                                 // Number of elements.
size_t                           groups         = 0;                                                 // Number of groups.
size_t                           neighbours     = 0;                                                 // Number of neighbours.
size_t                           frontier_nodes = 0;                                                 // Number of frontier nodes.
int                              ABCD           = 13;                                                // "ABCD" surface tag.
int                              EFGH           = 14;                                                // "EFGH" surface tag.
int                              ADHE           = 15;                                                // "ADHE" surface tag.
int                              BCGF           = 16;                                                // "BCGF" surface tag.
int                              ABFE           = 17;                                                // "ABFE" surface tag.
int                              DCGH           = 18;                                                // "DCGH" surface tag.
int                              VOLUME         = 1;                                                 // Entire volume tag.
std::vector<int>                 boundary;                                                           // Boundary array.
float                            px;
float                            py;
float                            pz;
float                            px_new;
float                            py_new;
float                            pz_new;

// SIMULATION VARIABLES:
float                            safety_CFL     = 0.5f;                                              // Courant-Friedrichs-Lewy safety coefficient [].
int                              N              = 3;                                                 // Number of spatial dimensions of the MSM [].
float                            rho            = 1.0E+2f;                                           // Mass density [kg/m^3].
float                            E              = 1.0E+1f;                                           // Young's modulus [Pa];
float                            ni             = -0.99f;                                            // Poisson's ratio [];
float                            beta           = 2.0E+0f;                                           // Damping [kg*s*m].
int                              R              = 1;                                                 // Particle's radius [#cells].

float                            ds;                                                                 // Cell size [m].
float                            dV;                                                                 // Cell volume [m^3].
float                            k;                                                                  // Spring constant [N/m].
float                            K;                                                                  // Bulk modulus [Pa].
float                            v_p;                                                                // Speed of P-waves [m/s].
float                            v_s;                                                                // Speed of S-waves [m/s].
float                            dm;                                                                 // Node mass [kg].
float                            lambda;                                                             // Lamé 1st parameter [Pa].
float                            mu;                                                                 // Lamé 2nd parameter (S-wave modulus) [Pa].
float                            M;                                                                  // P-wave modulus [Pa].
float                            B;                                                                  // Dispersive pressure [Pa].
float                            Q;                                                                  // Dispersive to direct momentum flow ratio [].
float                            C;                                                                  // Interaction momentum carriers pressure [Pa].
float                            D;                                                                  // Dispersion fraction [-0.5...1.0].
float                            dt_CFL;                                                             // Courant-Friedrichs-Lewy critical time step [s].
float                            dt_SIM;                                                             // Simulation time step [s].

// BACKUP:
std::vector<nu_float4_structure> initial_position;                                                   // Backing up initial data...
std::vector<nu_float4_structure> initial_velocity;                                                   // Backing up initial data...
std::vector<nu_float4_structure> initial_velocity_int;                                               // Backing up initial data...
std::vector<nu_float4_structure> initial_velocity_est;                                               // Backing up initial data...
std::vector<nu_float4_structure> initial_acceleration;                                               // Backing up initial data...
std::vector<nu_float4_structure> initial_spinor_pos;                                                 // Backing up initial data...
std::vector<nu_float4_structure> initial_frontier_pos;                                               // Backing up initial data...

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////// DATA INITIALIZATION /////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
void data_initialization ()
{
  // INDEXES:
  GLuint i;                                                                                          // Index [#].
  GLuint j;                                                                                          // Index [#].

  // MESH:
  spacetime->process (VOLUME, N, NU_MSH_HEX_8);                                                      // Processing mesh...
  position->data  = spacetime->node_coordinates;                                                     // Setting all node coordinates...
  neighbour->data = spacetime->neighbour;                                                            // Setting neighbour indices...
  central->data   = spacetime->neighbour_center;                                                     // Setting neighbour centers...
  offset->data    = spacetime->neighbour_offset;                                                     // Setting neighbour offsets...
  resting->data   = spacetime->neighbour_length;                                                     // Setting resting distances...
  nodes           = spacetime->node.size ();                                                         // Getting the number of nodes...
  elements        = spacetime->element.size ();                                                      // Getting the number of elements...
  groups          = spacetime->group.size ();                                                        // Getting the number of groups...
  neighbours      = spacetime->neighbour.size ();                                                    // Getting the number of neighbours...
  ds              = *std::min_element (std::begin (resting->data), std::end (resting->data));        // Getting cell size...

  dV              = (float)pow (ds, N);                                                              // Computing cell volume...
  dm              = rho*dV;                                                                          // Computing node mass...
  lambda          = (E*ni)/((1.0f + ni)*(ni - N*ni + 1.0f));                                         // Computing 1st Lamé parameter...
  mu              = E/(2.0f*(1.0f + ni));                                                            // Computing 2nd Lamé parameter (S-wave modulus)...
  M               = E*(1.0f - ni)/((1.0f + ni)*(ni - N*ni + 1.0f));                                  // Computing P-wave modulus...
  B               = lambda - mu;                                                                     // Computing dispersive pressure...
  Q               = B/(mu*(1.0f + 2.0f/N));                                                          // Computing dispersive to direct momentum flow ratio...
  C               = mu + mu*abs (Q);                                                                 // Computing interaction momentum carriers pressure...
  D               = Q/(1.0f + abs (Q));                                                              // Computing dispersion fraction...
  k               = 5.0f/(2.0f + 4.0f*sqrt (2.0f))*C*ds;                                             // Computing spring constant (valid only for N = 3)...
  K               = C*(1 + 2.0f/N)*(D - abs (D) + 1);                                                // Computing bulk modulus...
  v_p             = sqrt (abs (M/rho));                                                              // Computing speed of P-waves...
  v_s             = sqrt (abs (mu/rho));                                                             // Computing speed of S-waves...
  dt_CFL          = ds/(N*(v_p + v_s));                                                              // Computing Courant-Friedrichs-Lewy critical time step [s]...
  dt_SIM          = safety_CFL*dt_CFL;                                                               // Setting simulation time step [s]...

  // SETTING NEUTRINO ARRAYS (parameters):
  dispersion->data.push_back (D);                                                                    // Setting dispersion fraction...
  dt->data.push_back (dt_SIM);                                                                       // Setting time step...

  // SETTING NEUTRINO ARRAYS ("nodes" depending):
  for(i = 0; i < nodes; i++)
  {
    position->data[i].w = 1.0f;                                                                      // Setting freedom flag...
    velocity->data.push_back ({0.0f, 0.0f, 0.0f, beta});                                             // Setting velocity...
    velocity_int->data.push_back ({0.0f, 0.0f, 0.0f, 0.0f});                                         // Setting intermediate velocity...
    velocity_est->data.push_back ({0.0f, 0.0f, 0.0f, 0.0f});                                         // Setting estimated velocity...
    acceleration->data.push_back ({0.0f, 0.0f, 0.0f, dm});                                           // Setting acceleration...

    // Finding spinor:
    if(
       sqrt (
             pow (position->data[i].x, 2) +
             pow (position->data[i].y, 2) +
             pow (position->data[i].z, 2)
            ) < (sqrt (3.0f)*ds*R + 0.01f)
      )
    /*if(
       ((-0.01f - 3*ds) < position->data[i].x) &&
       (position->data[i].x < (0.01f + 3*ds)) &&
       ((-0.01f - 3*ds) < position->data[i].y) &&
       (position->data[i].y < (0.01f + 3*ds)) &&
       ((-0.01f - 3*ds) < position->data[i].z) &&
       (position->data[i].z < (0.01f + 3*ds))
       )*/
    {
      spinor->data.push_back (i);                                                                    // Setting spinor index...
      spinor_pos->data.push_back (position->data[i]);                                                // Setting initial spinor's position...
      position->data[i].w = 0.0f;                                                                    // Resetting freedom flag... (EZOR 25APR2021: temporary set to 1)
    }
  }

  spinor_num->data.push_back ((GLint)spinor->data.size ());

  // SETTING NEUTRINO ARRAYS ("neighbours" depending):
  for(i = 0; i < neighbours; i++)
  {
    // Building 3D isotropic 18-node cubic MSM:
    if(resting->data[i] < (ds + 0.01f))
    {
      stiffness->data.push_back (k);                                                                 // Setting 1st nearest neighbour link stiffness...
    }
    if((resting->data[i] > (ds + 0.01f)) &&
       (resting->data[i] < (sqrt (2.0f)*ds + 0.01f))
      )
    {
      stiffness->data.push_back (k);                                                                 // Setting 2nd nearest neighbour link stiffness...
    }
    if((resting->data[i] > (sqrt (2.0f)*ds + 0.01f)) &&
       (resting->data[i] < (sqrt (3.0f)*ds + 0.01f))
      )
    {
      stiffness->data.push_back (0.0f);                                                              // Setting 3rd nearest neighbour link stiffness...
    }

    // Showing only 1st neighbours:
    if(resting->data[i] < (ds + 0.01f))
    {
      color->data.push_back ({0.0f, 1.0f, 0.0f, 0.3f});                                              // Setting color...
    }
    else
    {
      color->data.push_back ({0.0f, 0.0f, 0.0f, 0.0f});                                              // Setting color...
    }
  }

  // SETTING MESH PHYSICAL CONSTRAINTS:
  //boundary.push_back (ABCD);                                                                        // Setting boundary surface...
  //boundary.push_back (EFGH);                                                                        // Setting boundary surface...
  boundary.push_back (ADHE);                                                                         // Setting boundary surface...
  boundary.push_back (BCGF);                                                                         // Setting boundary surface...
  boundary.push_back (ABFE);                                                                         // Setting boundary surface...
  boundary.push_back (DCGH);                                                                         // Setting boundary surface...

  for(i = 0; i < boundary.size (); i++)
  {
    spacetime->process (boundary[i], 2, NU_MSH_PNT);                                                 // Processing mesh...
    frontier->data.insert (
                           frontier->data.end (),
                           spacetime->node.begin (),
                           spacetime->node.end ()
                          );                                                                         // Getting nodes on the spacetime frontier...

    frontier_nodes += spacetime->node.size ();                                                       // Getting the number of nodes on the spacetime frontier...
  }

  std::cout << "frontier_nodes = " << frontier_nodes << std::endl;

  for(j = 0; j < frontier_nodes; j++)
  {
    position->data[frontier->data[j]].w = 0.0f;                                                      // Resetting freedom flag...
    frontier_pos->data.push_back (position->data[frontier->data[j]]);
  }

  frontier_num->data.push_back ((GLint)frontier_nodes);

  // SETTING INITIAL DATA BACKUP:
  initial_position     = position->data;                                                             // Setting backup data...
  initial_velocity_est = velocity_est->data;                                                         // Setting backup data...
  initial_velocity     = velocity->data;                                                             // Setting backup data...
  initial_velocity_int = velocity_int->data;                                                         // Setting backup data...
  initial_acceleration = acceleration->data;                                                         // Setting backup data...
  initial_spinor_pos   = spinor_pos->data;                                                           // Setting backup data...
  initial_frontier_pos = frontier_pos->data;                                                         // Setting backup data...
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////// MAIN /////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
int main ()
{
  // INDEXES:
  GLuint i;                                                                                          // Index [#].
  GLuint j;                                                                                          // Index [#].

  // MOUSE PARAMETERS:
  float  ms_orbit_rate  = 1.0f;                                                                      // Orbit rotation rate [rev/s].
  float  ms_pan_rate    = 5.0f;                                                                      // Pan translation rate [m/s].
  float  ms_decaytime   = 1.25f;                                                                     // Pan LP filter decay time [s].

  // GAMEPAD PARAMETERS:
  float  gmp_orbit_rate = 1.0f;                                                                      // Orbit angular rate coefficient [rev/s].
  float  gmp_pan_rate   = 1.0f;                                                                      // Pan translation rate [m/s].
  float  gmp_decaytime  = 1.25f;                                                                     // Low pass filter decay time [s].
  float  gmp_deadzone   = 0.30f;                                                                     // Gamepad joystick deadzone [0...1].

  data_initialization ();                                                                            // Initializing data...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////// OPENCL KERNELS INITIALIZATION /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  kernel_1->addsource (std::string (KERNEL_HOME) + std::string (UTILITIES));                         // Setting kernel source file...
  kernel_1->addsource (std::string (KERNEL_HOME) + std::string (KERNEL_1));                          // Setting kernel source file...
  kernel_1->build (nodes, 0, 0);                                                                     // Building kernel program...
  kernel_2->addsource (std::string (KERNEL_HOME) + std::string (UTILITIES));                         // Setting kernel source file...
  kernel_2->addsource (std::string (KERNEL_HOME) + std::string (KERNEL_2));                          // Setting kernel source file...
  kernel_2->build (nodes, 0, 0);                                                                     // Building kernel program...
  kernel_3->addsource (std::string (KERNEL_HOME) + std::string (UTILITIES));                         // Setting kernel source file...
  kernel_3->addsource (std::string (KERNEL_HOME) + std::string (KERNEL_3));                          // Setting kernel source file...
  kernel_3->build (nodes, 0, 0);                                                                     // Building kernel program...
  kernel_4->addsource (std::string (KERNEL_HOME) + std::string (UTILITIES));                         // Setting kernel source file...
  kernel_4->addsource (std::string (KERNEL_HOME) + std::string (KERNEL_4));                          // Setting kernel source file...
  kernel_4->build (nodes, 0, 0);                                                                     // Building kernel program...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////// OPENGL SHADERS INITIALIZATION /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  shader_1->addsource (std::string (SHADER_HOME) + std::string (SHADER_VERT), NU_VERTEX);            // Setting shader source file...
  shader_1->addsource (std::string (SHADER_HOME) + std::string (SHADER_GEOM), NU_GEOMETRY);          // Setting shader source file...
  shader_1->addsource (std::string (SHADER_HOME) + std::string (SHADER_FRAG), NU_FRAGMENT);          // Setting shader source file...
  shader_1->build (neighbours);                                                                      // Building shader program...

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////// SETTING OPENCL KERNEL ARGUMENTS /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  cl->write ();

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////// PRINTING SIMULATION PARAMETERS /////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "============================== LATTICE PARAMETERS ===============================";  // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Lattice spatial dimension:                  N          = " << N << " []";            // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Cell size:                                  ds         = " << ds << " [m]";          // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Cell volume:                                dV         = " << dV << " [m^N]";        // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Node mass:                                  dm         = " << dm << " [kg]";         // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Spring constant:                            k          = " << k << " [N/m]";         // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << std::endl;                                                                            // Printing message...

  std::cout << "============================= MECHANICAL PARAMETERS =============================";  // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Young's modulus:                            E          = " << E << " [Pa]";          // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Poisson's ratio:                            ni         = " << ni << " []";           // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Lame's 1st parameter:                       lambda     = " << lambda << " [Pa]";     // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Lame's 2nd parameter (S-wave modulus):      mu         = " << mu << " [Pa]";         // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "P-wave modulus:                             M          = " << M << " [Pa]";          // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Dispersive pressure:                        B          = " << B << " [Pa]";          // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Dispersive-to-direct momentum flow ratio:   Q          = " << Q << " []";            // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Interaction momentum carriers pressure:     C          = " << C << " [Pa]";          // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Dispersion fraction:                        D          = " << D << " []";            // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << std::endl;                                                                            // Printing message...

  std::cout << "============================== DYNAMICS PARAMETERS ==============================";  // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Mass density:                               rho        = " << rho << " [kg/m^N]";    // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Bulk modulus:                               K          = " << K << " [Pa]";          // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Speed of S-waves:                           v_s        = " << v_s << " [m/s]";       // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Speed of P-waves:                           v_p        = " << v_p << " [m/s]";       // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Courant-Friedrichs-Lewy critical time step: dt_CFL     = " << dt_CFL << " [s]";      // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Courant-Friedrichs-Lewy safety coefficient: safety_CFL = " << safety_CFL << " []";   // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << "Simulation time step:                       dt_sim     = " << dt_SIM << " [s]";      // Printing message...
  std::cout << std::endl;                                                                            // Printing message...
  std::cout << std::endl;                                                                            // Printing message...

  int pressure = 0;

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////// APPLICATION LOOP ////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  while(!gl->closed ())                                                                              // Opening window...
  {
    cl->get_tic ();                                                                                  // Getting "tic" [us]...
    cl->write (13);                                                                                  // Writing spinor's position...
    cl->write (16);                                                                                  // Writing frontier position...
    cl->acquire ();                                                                                  // Acquiring variables...
    cl->execute (kernel_1, NU_WAIT);                                                                 // Executing OpenCL kernel...
    cl->execute (kernel_2, NU_WAIT);                                                                 // Executing OpenCL kernel...
    cl->execute (kernel_3, NU_WAIT);                                                                 // Executing OpenCL kernel...
    cl->execute (kernel_4, NU_WAIT);                                                                 // Executing OpenCL kernel...
    cl->release ();                                                                                  // Releasing variables...

    gl->clear ();                                                                                    // Clearing gl...

    gl->poll_events ();                                                                              // Polling gl events...
    gl->mouse_navigation (ms_orbit_rate, ms_pan_rate, ms_decaytime);                                 // Mouse navigation...
    gl->gamepad_navigation (gmp_orbit_rate, gmp_pan_rate, gmp_decaytime, gmp_deadzone);              // Gamepad navigation...
    gl->plot (shader_1);                                                                             // Plotting shared arguments...

    ImGui_ImplOpenGL3_NewFrame ();
    ImGui_ImplGlfw_NewFrame ();
    ImGui::NewFrame ();

    /*
       float rho  = 1.0E+2f;                                                                            // Mass density [kg/m^3].
       float E    = 1.0E+1f;                                                                            // Young's modulus [Pa];
       float ni   = -0.99f;                                                                             // Poisson's ratio [];
       float beta = 2.0E+0f;                                                                            // Damping [kg*s*m].
       float R    = 1;                                                                                  // Particle's radius [#cells].
     */
    ImGui::Begin ("FREE LATTICE PARAMETERS", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth (200);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Mass density:    ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("rho = ");
    ImGui::SameLine ();
    ImGui::InputFloat (" [kg/m^3]", &rho);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Young's modulus:   ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("Y = ");
    ImGui::SameLine ();
    ImGui::InputFloat (" [Pa]", &E, 0.0f, 0.0f, "%e");

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Poisson's ratio:  ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("nu = ");
    ImGui::SameLine ();
    ImGui::InputFloat (" []", &ni, 0.0f, 0.0f, "%e");

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Damping:        ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("beta = ");
    ImGui::SameLine ();
    ImGui::InputFloat (" [kg*s*m]", &beta, 0.0f, 0.0f, "%e");

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Particle's radius: ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("R = ");
    ImGui::SameLine ();
    ImGui::InputInt (" [#cells]", &R);

    ImGui::End ();

    ImGui::Begin ("DERIVED LATTICE PARAMETERS", NULL, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth (400);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Cell size:                                  ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("ds         = %f [m]", ds);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Cell volume:                                ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("dV         = %f [m^3]", dV);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Node mass:                                  ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("dm         = %f [kg]", dm);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Spring constant:                            ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("k          = %f [N/m]", k);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Internal pressure:                          ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("P          = %d [N/m^2]", pressure);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Lame's 1st parameter:                       ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("lambda     = %f [Pa]", lambda);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("S-wave modulus (Lame's 2nd parameter):      ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("mu         = %f [Pa]", mu);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("P-wave modulus:                             ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("M          = %f [Pa]", M);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Dispersive pressure:                        ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("B          = %f [Pa]", B);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Dispersive-to-direct momentum flow ratio:   ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("Q          = %f []", Q);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Interaction momentum carriers pressure:     ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("C          = %f [Pa]", C);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Dispersion fraction:                        ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("D          = %f []", D);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Bulk modulus:                               ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("K          = %f [Pa]", K);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Speed of S-waves:                           ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("v_s        = %f [m/s]", v_s);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Speed of P-waves:                           ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("v_p        = %f [m/s]", v_p);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Courant-Friedrichs-Lewy critical time step: ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("dt_CFL     = %f [s]", dt_CFL);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Courant-Friedrichs-Lewy safety coefficient: ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("safety_CFL = %f []", safety_CFL);

    ImGui::PushStyleColor (ImGuiCol_Text, IM_COL32 (0,255,0,255));
    ImGui::Text ("Simulation time step:                       ");
    ImGui::PopStyleColor ();
    ImGui::SameLine ();
    ImGui::Text ("dt_sim     = %f [s]", dt_SIM);

    ImGui::End ();



    ImGui::Render ();
    ImGui_ImplOpenGL3_RenderDrawData (ImGui::GetDrawData ());

    gl->refresh ();                                                                                 // Refreshing gl...

    if(gl->button_DPAD_LEFT || gl->key_LEFT)
    {
      for(i = 0; i < (GLuint)spinor_num->data[0]; i++)
      {
        px                    = spinor_pos->data[i].x;
        py                    = spinor_pos->data[i].y;

        px_new                = +cos (ROT)*px - sin (ROT)*py;
        py_new                = +sin (ROT)*px + cos (ROT)*py;

        spinor_pos->data[i].x = px_new;
        spinor_pos->data[i].y = py_new;
      }
    }

    if(gl->button_DPAD_RIGHT || gl->key_RIGHT)
    {
      for(i = 0; i < (GLuint)spinor_num->data[0]; i++)
      {
        px                    = spinor_pos->data[i].x;
        py                    = spinor_pos->data[i].y;

        px_new                = +cos (ROT)*px + sin (ROT)*py;
        py_new                = -sin (ROT)*px + cos (ROT)*py;

        spinor_pos->data[i].x = px_new;
        spinor_pos->data[i].y = py_new;
      }
    }

    if(gl->button_DPAD_DOWN || gl->key_DOWN)
    {
      for(i = 0; i < (GLuint)spinor_num->data[0]; i++)
      {
        py                    = spinor_pos->data[i].y;
        pz                    = spinor_pos->data[i].z;

        py_new                = +cos (ROT)*py - sin (ROT)*pz;
        pz_new                = +sin (ROT)*py + cos (ROT)*pz;

        spinor_pos->data[i].y = py_new;
        spinor_pos->data[i].z = pz_new;
      }
    }

    if(gl->button_DPAD_UP || gl->key_UP)
    {
      for(i = 0; i < (GLuint)spinor_num->data[0]; i++)
      {
        py                    = spinor_pos->data[i].y;
        pz                    = spinor_pos->data[i].z;

        py_new                = +cos (ROT)*py + sin (ROT)*pz;
        pz_new                = -sin (ROT)*py + cos (ROT)*pz;

        spinor_pos->data[i].y = py_new;
        spinor_pos->data[i].z = pz_new;
      }
    }

    if(gl->button_LEFT_BUMPER || gl->key_O)
    {
      for(i = 0; i < (GLuint)spinor_num->data[0]; i++)
      {
        px                    = spinor_pos->data[i].x;
        py                    = spinor_pos->data[i].y;
        pz                    = spinor_pos->data[i].z;

        px_new                = px*0.999f;
        py_new                = py*0.999f;
        pz_new                = pz*0.999f;

        spinor_pos->data[i].x = px_new;
        spinor_pos->data[i].y = py_new;
        spinor_pos->data[i].z = pz_new;
      }
    }

    if(gl->button_RIGHT_BUMPER || gl->key_P)
    {
      for(i = 0; i < (GLuint)spinor_num->data[0]; i++)
      {
        px                    = spinor_pos->data[i].x;
        py                    = spinor_pos->data[i].y;
        pz                    = spinor_pos->data[i].z;

        px_new                = px/0.999f;
        py_new                = py/0.999f;
        pz_new                = pz/0.999f;

        spinor_pos->data[i].x = px_new;
        spinor_pos->data[i].y = py_new;
        spinor_pos->data[i].z = pz_new;
      }
    }

    if(gl->button_SQUARE || gl->key_Q)
    {
      for(i = 0; i < (GLuint)frontier_num->data[0]; i++)
      {
        px                      = frontier_pos->data[i].x;
        py                      = frontier_pos->data[i].y;
        pz                      = frontier_pos->data[i].z;

        px_new                  = px*0.9995f;
        py_new                  = py*0.9995f;
        pz_new                  = pz*0.9995f;

        frontier_pos->data[i].x = px_new;
        frontier_pos->data[i].y = py_new;
        frontier_pos->data[i].z = pz_new;

        pressure++;
      }
    }

    if(gl->button_CIRCLE || gl->key_W)
    {
      for(i = 0; i < (GLuint)frontier_num->data[0]; i++)
      {
        px                      = frontier_pos->data[i].x;
        py                      = frontier_pos->data[i].y;
        pz                      = frontier_pos->data[i].z;

        px_new                  = px/0.9995f;
        py_new                  = py/0.9995f;
        pz_new                  = pz/0.9995f;

        frontier_pos->data[i].x = px_new;
        frontier_pos->data[i].y = py_new;
        frontier_pos->data[i].z = pz_new;

        pressure--;
      }
    }

    if(gl->button_TRIANGLE || gl->key_R)
    {
      position->data     = initial_position;                                                        // Restoring backup...
      velocity->data     = initial_velocity;                                                        // Restoring backup...
      velocity_int->data = initial_velocity_int;                                                    // Restoring backup...
      velocity_est->data = initial_velocity_est;                                                    // Restoring backup...
      acceleration->data = initial_acceleration;                                                    // Restoring backup...
      spinor_pos->data   = initial_spinor_pos;                                                      // Restoring backup...
      frontier_pos->data = initial_frontier_pos;                                                    // Restoring backup...
      cl->write (0);                                                                                // Writing data...
      cl->write (1);                                                                                // Writing data...
      cl->write (2);                                                                                // Writing data...
      cl->write (3);                                                                                // Writing data...
      cl->write (4);                                                                                // Writing data...
      cl->write (13);                                                                               // Writing data...
      cl->write (16);                                                                               // Writing data...
    }

    if(gl->button_CROSS || gl->key_ESCAPE)
    {
      gl->close ();                                                                                 // Closing gl...
    }

    cl->get_toc ();                                                                                 // Getting "toc" [us]...
  }

  ImGui_ImplOpenGL3_Shutdown ();
  ImGui_ImplGlfw_Shutdown ();
  ImGui::DestroyContext ();

  ////////////////////////////////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////// CLEANUP ////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  delete cl;                                                                                        // Deleting OpenCL context...
  delete position;                                                                                  // Deleting position data...
  delete velocity;                                                                                  // Deleting velocity data...
  delete velocity_int;                                                                              // Deleting intermediate velocity data...
  delete velocity_est;                                                                              // Deleting estimated velocity data...
  delete acceleration;                                                                              // Deleting acceleration data...
  delete color;                                                                                     // Deleting color data...
  delete stiffness;                                                                                 // Deleting stiffness data...
  delete resting;                                                                                   // Deleting resting data...
  delete central;                                                                                   // Deleting central...
  delete neighbour;                                                                                 // Deleting neighbours...
  delete offset;                                                                                    // Deleting offset...
  delete spinor;                                                                                    // Deleting spinor...
  delete spinor_num;                                                                                // Deleting spinor_num...
  delete spinor_pos;                                                                                // Deleting spinor_pos...
  delete frontier;                                                                                  // Deleting frontier...
  delete frontier_num;                                                                              // Deleting frontier_num...
  delete frontier_pos;                                                                              // Deleting frontier_pos...
  delete dt;                                                                                        // Deleting time step data...
  delete kernel_1;                                                                                  // Deleting OpenCL kernel...
  delete kernel_2;                                                                                  // Deleting OpenCL kernel...
  delete kernel_3;                                                                                  // Deleting OpenCL kernel...
  delete kernel_4;                                                                                  // Deleting OpenCL kernel...
  delete shader_1;                                                                                  // Deleting OpenGL shader...
  delete spacetime;                                                                                 // Deleting spacetime mesh...
  delete cl;                                                                                        // Deleting OpenCL...
  delete gl;                                                                                        // Deleting OpenGL...

  return 0;
}
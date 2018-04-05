/**
* Authors: Samuel A. Cruz Alegría, Alessandra M. de Felice, Hrishikesh R. Gupta.
*
* This program simulates particle movement in 2D space.
*/

#include <ctime>
#include <fstream>
#include <iostream>
#include <cstdlib>

#include "particles.h"

using namespace std;

#define DEFAULT_WIDTH 800
#define DEFAULT_HEIGHT 800
#define DEBUGGING 1
#undef DEBUGGING

static const string PDPATH = "./particle_positions/";  // Path to write files to.
static int width;         // Width of box containing particles.
static int height;        // Height of box containing particles.
static int n;             // Number of particles.
static float fx;          // Horizontal component of the force field.
static float fy;          // Vertical component of the force field.
static float radius;      // Radius of the particles, in pixels.
static float delta;       // Time, in seconds, for inter-frame interval.
static int total_time_interval;    // Time, in seconds, for total time interval.
static float g;           // Gravitational factor (in y direction).
static float *pd;         // Particle details array.

void print_all_particle_details();
int write_all_particle_details_to_file(string filename);

/*
* Print expected usage of this program.
*/
void
print_usage()
{
  cerr << "Usage: [width=box_width] "
       << "[height=box_height] "
       << "[n=num_particles] "
       << "[fx=forcefield_x] "
       << "[fy=forcefield_y] "
       << "[trace=shading_factor_trace] "
       << "[radius=particle_radius] "
       << "[delta=inter_frame_interval_in_seconds] "
       << "[total_time_interval=total_time_in_seconds]\n";
}


// main() is where program execution begins.
int
main(int argc, char *argv[])
{
  // Do all necessary initializations.
  if (!init_params(argc, argv) || !init_particles()) {
    return -1;
  }

  // Calculate number of loop cycles to be performed given a total time interval
  // and time per frame.
  int nCycles = total_time_interval / delta;
  #ifdef DEBUGGING
  printf("nCycles=%d\n", nCycles);
  #endif
  for (int cycle = 0; cycle < nCycles; ++cycle) {
    float current_time_frame = delta * cycle;

    string filename ("positions_" + to_string(current_time_frame) + ".txt");

    update_particles();
    if (!write_all_particle_details_to_file(filename)) {
      cerr << "Could not write file: " << filename << "\n";
    }
  }

  free(pd);
  return 0;
}

/*
* Initialize all particle details.
* @return 1 on success, 0 on error.
*/
int
init_particles()
{
  // Allocate space for particle details.
  pd = (float *)malloc(sizeof(*pd) * n * 4);
  if (!pd) {
    fprintf(stderr, "Could not allocate space for particle details.\n");
    return 0;
  }

  /* The srand() function sets its argument seed as the seed for a new
   * sequence of pseudo-random numbers to be returned by rand().  These
   * sequences are repeatable by calling srand() with the same seed value.
   *
   *
   * time(0) explanation from: https://stackoverflow.com/questions/4736485/srandtime0-and-random-number-generation
   * time(0) gives the time in seconds since the Unix epoch, which is a
   * pretty good "unpredictable" seed (you're guaranteed your seed will be the
   * same only once, unless you start your program multiple times within the
   * same second).*/
  srand(time(0));

  // Go through all particles and initialize their details at random.
  for (int id = 0; id < n; ++id) {
    int px_i = id*4;      // x-position index.
    int py_i = id*4 + 1;  // y-position index.
    int vx_i = id*4 + 2;  // vx-component index.
    int vy_i = id*4 + 3;  // vy-component index.

    pd[px_i] = (double) (rand() % DEFAULT_WIDTH);   // Set x position.
    pd[py_i] = (double) (rand() % DEFAULT_HEIGHT);  // Set y position.
    pd[vx_i] = rand() / (float) RAND_MAX * fx;      // Set vx component.
    pd[vy_i] = rand() / (float) RAND_MAX * fy;      // Set vy component.

    // Correct starting x position.
    if (pd[px_i] - radius <= 0) {
      pd[px_i] = radius;
    } else if (pd[px_i] + radius >= DEFAULT_WIDTH) {
      pd[px_i] = DEFAULT_WIDTH - radius;
    }

    // Correct starting y position.
    if (pd[py_i] - radius <= 0) {
      pd[py_i] = radius;
    } else if (pd[py_i] + radius >= DEFAULT_HEIGHT) {
      pd[py_i] = DEFAULT_HEIGHT - radius;
    }
  }

  #ifdef DEBUGGING
  // Set position of first particle at bottom left corner.
  pd[0] = radius;
  pd[1] = DEFAULT_HEIGHT - radius;
  #endif

  return 1;
}

/*
* Update the particle details.
* @return 1 on success, 0 on error.
*/
int
update_particles()
{
  // Loop through all particles
  for (int id = 0; id < n; ++id) {
    int px_i = id*4;      // x-position index.
    int py_i = id*4 + 1;  // y-position index.
    int vx_i = id*4 + 2;  // vx-component index.
    int vy_i = id*4 + 3;  // vy-component index.

    float px = pd[px_i];  // x position.
    float py = pd[py_i];  // y position.
    float vx = pd[vx_i];  // vx component.
    float vy = pd[vy_i];  // vy component.

    // Set new x direction based on horizontal collision with wall.
    if (px + radius >= width || px - radius <= 0) {
      vx = vx * -1;
    }

    // Set new y direction based on vertical collision with wall.
    if (py - radius <= 0 || py + radius >= height) {
      vy = vy * -1;
    }

    // Update x and y positions.
    pd[px_i] = px + (vx*delta);
    pd[py_i] = py - (vy*delta) - (0.5 * g * delta * delta);
    // Update vx and vy components.
    pd[vx_i] = vx;  // vx only as acceleration in x-direction is 0.
    pd[vy_i] = vy + 0.5*(g)*delta;

    // Correct x position in case updated position goes past wall.
    if (pd[px_i] - radius <= 0) {
      pd[px_i] = radius;
    } else if (pd[px_i] + radius >= height) {
      pd[px_i] = width - radius;
    }

    // Correct y position in case updated position goes past wall.
    if (pd[py_i] - radius <= 0) {
      pd[py_i] = radius;
    } else if (pd[py_i] + radius >= height) {
      pd[py_i] = height - radius;
    }
  }

  return 1;
}

/* Print the details of all particles.*/
void
print_all_particle_details()
{
  for (int i = 0; i < n; ++i) {
    printf("particles[%d]: px=%f, py=%f, vx=%f, vy=%f\n",
            i, pd[i*4], pd[i*4 + 1], pd[i*4 + 2], pd[i*4 + 3]);
  }
}

/* Write the details of all particles to file with given filename.
* The path PDPATH is used to specify where to save the file.
* @return 1 on success, 0 on failure.*/
int
write_all_particle_details_to_file(string filename)
{
  ofstream myfile;

  myfile.open(PDPATH + filename, ios::out);
  if (myfile.is_open()) {
    for (int i = 0; i < n; ++i) {
      myfile << i << "\t" << pd[i*4] << "\t" << pd[i*4 + 1] << "\n";
    }

    myfile.close();
  } else {
    cerr << "Unable to open file: " << filename << "\n";
    return 0;
  }

  return 1;
}

/* Initialize default parameters.
* @return 1 on success, 0 on failure.*/
int
init_params(int argc, char *argv[])
{

  width = DEFAULT_WIDTH;    // Width of box containing particles.
  height = DEFAULT_HEIGHT;  // Height of box containing particles.
  n = 5;                    // Number of particles.
  fx = 50;                  // Horizontal component of the force field.
  fy = 50;                  // Vertical component of the force field.
  radius = 5;               // Radius of the particles, in pixels.
  delta = 1.0;              // Time, in seconds, for inter-frame interval.
  total_time_interval = 10;          // Time, in seconds, for total time interval.
  g = -9.8;                 // Gravitational factor (in y direction).

  // Read and process command-line arguments.
  for (int i = 1; i < argc; ++i) {
    if(!process_arg(argv[i])) {
      cerr << "Invalid argument: " << argv[i] << "\n";
      print_usage();
      return 0;
    }
  }

  #ifdef DEBUGGING
  printf("width=%d\n", width);
  printf("height=%d\n", height);
  printf("n=%d\n", n);
  printf("fx=%f\n", fx);
  printf("fy=%f\n", fy);
  printf("radius=%f\n", radius);
  printf("delta=%f\n", delta);
  printf("total_time_interval=%d\n", total_time_interval);
  printf("g: %f\n", g);
  #endif

  return 1;
}


/*
* Process the given command-line parameter.
* @param arg The command-line parameter.
* @return 1 on success, 0 on error.*/
int
process_arg(char *arg)
{
  if (strstr(arg, "width="))
  return sscanf(arg, "width=%d", &width) == 1;

  if (strstr(arg, "height="))
  return sscanf(arg, "height=%d", &height) == 1;

  else if (strstr(arg, "n="))
  return sscanf(arg, "n=%d", &n) == 1;

  else if (strstr(arg, "fx="))
  return sscanf(arg, "fx=%f", &fx) == 1;

  else if (strstr(arg, "fy="))
  return sscanf(arg, "fy=%f", &fy) == 1;

  else if (strstr(arg, "radius="))
  return sscanf(arg, "radius=%f", &radius) == 1;

  else if (strstr(arg, "delta="))
  return sscanf(arg, "delta=%f", &delta) == 1;

  else if (strstr(arg, "total_time_interval="))
  return sscanf(arg, "total_time_interval=%d", &total_time_interval) == 1;

  // Return 0 if the given command-line parameter was invalid.
  return 0;
}

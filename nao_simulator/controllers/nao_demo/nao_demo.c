/*
 * Copyright 1996-2020 Cyberbotics Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//---------------------------------------------------------------------------------
//  Description:  Example C controller program for Nao robot.
//                This demonstrates how to access sensors and actuators
//---------------------------------------------------------------------------------
/*
 * Modified By: Metehan PATACI, Vefak Murat AKMAN
 *
 *
 *
 */


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <in6addr.h>
#include <Ws2tcpip.h>
#include <winsock2.h>
#include <webots/accelerometer.h>
#include <webots/camera.h>
#include <webots/distance_sensor.h>
#include <webots/gps.h>
#include <webots/gyro.h>
#include <webots/inertial_unit.h>
#include <webots/keyboard.h>
#include <webots/led.h>
#include <webots/motor.h>
#include <webots/robot.h>
#include <webots/touch_sensor.h>
#include <webots/utils/motion.h>
#include "iphlpapi.h"


#ifdef _MSC_VER
#define snprintf sprintf_s
#endif

#define PHALANX_MAX 8

static int time_step = -1;

// simulated devices
static WbDeviceTag CameraTop, CameraBottom;  // cameras
static WbDeviceTag us[2];                    // ultra sound sensors
static WbDeviceTag accelerometer, gps, gyro, inertial_unit;
static WbDeviceTag fsr[2];                        // force sensitive resistors
static WbDeviceTag lfoot_lbumper, lfoot_rbumper;  // left foot bumpers
static WbDeviceTag rfoot_lbumper, rfoot_rbumper;  // right foot bumpers
static WbDeviceTag leds[7];                       // controllable led groupsstatic WbDeviceTag lphalanx[PHALANX_MAX];
static WbDeviceTag rphalanx[PHALANX_MAX];         // right hand motors
static WbDeviceTag lphalanx[PHALANX_MAX];         // left hand motors
static WbDeviceTag RShoulderPitch;
static WbDeviceTag LShoulderPitch;

// motion file handles

static WbMotionRef hand_wave, forwards, backwards, side_step_left, side_step_right, turn_left_60, turn_right_60,stand_up_from_front,turn_left_40,turn_right_40,turn_left_180,shoot_motion;
static WbMotionRef currently_playing = NULL;

static double maxPhalanxMotorPosition[PHALANX_MAX];
static double minPhalanxMotorPosition[PHALANX_MAX];

static void find_and_enable_devices() {

  printf("Test");
  // camera
  CameraTop = wb_robot_get_device("CameraTop");
  CameraBottom = wb_robot_get_device("CameraBottom");
  wb_camera_enable(CameraTop, 4 * time_step);
  wb_camera_enable(CameraBottom, 4 * time_step);

  // accelerometer
  accelerometer = wb_robot_get_device("accelerometer");
  wb_accelerometer_enable(accelerometer, time_step);

  // gyro
  gyro = wb_robot_get_device("gyro");
  wb_gyro_enable(gyro, time_step);

  // gps
  gps = wb_robot_get_device("gps");
  wb_gps_enable(gps, time_step);

  // inertial unit
  inertial_unit = wb_robot_get_device("inertial unit");
  wb_inertial_unit_enable(inertial_unit, time_step);

  // ultrasound sensors
  us[0] = wb_robot_get_device("Sonar/Left");
  us[1] = wb_robot_get_device("Sonar/Right");
  int i;
  for (i = 0; i < 2; i++)
    wb_distance_sensor_enable(us[i], time_step);

  // foot sensors
  fsr[0] = wb_robot_get_device("LFsr");
  fsr[1] = wb_robot_get_device("RFsr");
  wb_touch_sensor_enable(fsr[0], time_step);
  wb_touch_sensor_enable(fsr[1], time_step);

  // foot bumpers
  lfoot_lbumper = wb_robot_get_device("LFoot/Bumper/Left");
  lfoot_rbumper = wb_robot_get_device("LFoot/Bumper/Right");
  rfoot_lbumper = wb_robot_get_device("RFoot/Bumper/Left");
  rfoot_rbumper = wb_robot_get_device("RFoot/Bumper/Right");
  wb_touch_sensor_enable(lfoot_lbumper, time_step);
  wb_touch_sensor_enable(lfoot_rbumper, time_step);
  wb_touch_sensor_enable(rfoot_lbumper, time_step);
  wb_touch_sensor_enable(rfoot_rbumper, time_step);

  // There are 7 controlable LED groups in Webots
  leds[0] = wb_robot_get_device("ChestBoard/Led");
  leds[1] = wb_robot_get_device("RFoot/Led");
  leds[2] = wb_robot_get_device("LFoot/Led");
  leds[3] = wb_robot_get_device("Face/Led/Right");
  leds[4] = wb_robot_get_device("Face/Led/Left");
  leds[5] = wb_robot_get_device("Ears/Led/Right");
  leds[6] = wb_robot_get_device("Ears/Led/Left");

  // get phalanx motor tags
  // the real Nao has only 2 motors for RHand/LHand
  // but in Webots we must implement RHand/LHand with 2x8 motors
  for (i = 0; i < PHALANX_MAX; i++) {
    char name[32];
    sprintf(name, "LPhalanx%d", i + 1);
    lphalanx[i] = wb_robot_get_device(name);
    sprintf(name, "RPhalanx%d", i + 1);
    rphalanx[i] = wb_robot_get_device(name);

    // assume right and left hands have the same motor position bounds
    maxPhalanxMotorPosition[i] = wb_motor_get_max_position(rphalanx[i]);
    minPhalanxMotorPosition[i] = wb_motor_get_min_position(rphalanx[i]);
  }

  // shoulder pitch motors
  RShoulderPitch = wb_robot_get_device("RShoulderPitch");
  LShoulderPitch = wb_robot_get_device("LShoulderPitch");

  // keyboard
  wb_keyboard_enable(10 * time_step);
}

// load motion files
static void load_motion_files() {
  hand_wave = wbu_motion_new("../../motions/HandWave.motion");
  forwards = wbu_motion_new("../../motions/Forwards50.motion");
  backwards = wbu_motion_new("../../motions/Backwards.motion");
  side_step_left = wbu_motion_new("../../motions/SideStepLeft.motion");
  side_step_right = wbu_motion_new("../../motions/SideStepRight.motion");
  turn_left_60 = wbu_motion_new("../../motions/TurnLeft60.motion");
  turn_right_60 = wbu_motion_new("../../motions/TurnRight60.motion");
  stand_up_from_front = wbu_motion_new("../../motions/StandUpFromFront.motion");
  turn_left_40 = wbu_motion_new("../../motions/TurnLeft40.motion");
  turn_right_40 = wbu_motion_new("../../motions/TurnRight40.motion");
  turn_left_180 = wbu_motion_new("../../motions/TurnLeft180.motion");
  shoot_motion = wbu_motion_new("../../motions/Shoot.motion");
  
}

static void start_motion(WbMotionRef motion) {
  // interrupt current motion
  if (currently_playing)
    wbu_motion_stop(currently_playing);

  // start new motion
  wbu_motion_play(motion);
  currently_playing = motion;
}

static double clamp(double value, double min, double max) {
  if (min > max) {
    assert(0);
    return value;
  }
  return value < min ? min : value > max ? max : value;
}

// the accelerometer axes are oriented as on the real robot
// however the sign of the returned values may be opposite
static void print_acceleration() {
  const double *acc = wb_accelerometer_get_values(accelerometer);
  printf("----------accelerometer----------\n");
  printf("acceleration: [ x y z ] = [%f %f %f]\n", acc[0], acc[1], acc[2]);
}

// the gyro axes are oriented as on the real robot
// however the sign of the returned values may be opposite
static void print_gyro() {
  const double *vel = wb_gyro_get_values(gyro);
  printf("----------gyro----------\n");
  printf("angular velocity: [ x y ] = [%f %f]\n", vel[0], vel[1]);
}

static void print_gps() {
  const double *p = wb_gps_get_values(gps);
  printf("----------gps----------\n");
  printf("position: [ x y z] = [%f %f %f]\n", p[0], p[1], p[2]);
}

// the InertialUnit roll/pitch angles are equal to naoqi's AngleX/AngleY
static void print_inertial_unit() {
  const double *rpy = wb_inertial_unit_get_roll_pitch_yaw(inertial_unit);
  printf("----------inertial unit----------\n");
  printf("roll/pitch/yaw: = [%f %f %f]\n", rpy[0], rpy[1], rpy[2]);
}

static void print_foot_sensors() {
  const double *fsv[2] = {wb_touch_sensor_get_values(fsr[0]), wb_touch_sensor_get_values(fsr[1])};  // force sensor values

  double l[4], r[4];
  double newtonLeft = 0, newtonRight = 0;

  // The coefficients were calibrated against the real
  // robot so as to obtain realistic sensor values.
  l[0] = fsv[0][2] / 3.4 + 1.5 * fsv[0][0] + 1.15 * fsv[0][1];  // Left Foot Front Left
  l[1] = fsv[0][2] / 3.4 + 1.5 * fsv[0][0] - 1.15 * fsv[0][1];  // Left Foot Front Right
  l[2] = fsv[0][2] / 3.4 - 1.5 * fsv[0][0] - 1.15 * fsv[0][1];  // Left Foot Rear Right
  l[3] = fsv[0][2] / 3.4 - 1.5 * fsv[0][0] + 1.15 * fsv[0][1];  // Left Foot Rear Left

  r[0] = fsv[1][2] / 3.4 + 1.5 * fsv[1][0] + 1.15 * fsv[1][1];  // Right Foot Front Left
  r[1] = fsv[1][2] / 3.4 + 1.5 * fsv[1][0] - 1.15 * fsv[1][1];  // Right Foot Front Right
  r[2] = fsv[1][2] / 3.4 - 1.5 * fsv[1][0] - 1.15 * fsv[1][1];  // Right Foot Rear Right
  r[3] = fsv[1][2] / 3.4 - 1.5 * fsv[1][0] + 1.15 * fsv[1][1];  // Right Foot Rear Left

  int i;
  for (i = 0; i < 4; ++i) {
    l[i] = clamp(l[i], 0, 25);
    r[i] = clamp(r[i], 0, 25);
    newtonLeft += l[i];
    newtonRight += r[i];
  }

  printf("----------foot sensors----------\n");
  printf("   left       right\n");
  printf("+--------+ +--------+\n");
  printf("|%3.1f  %3.1f| |%3.1f  %3.1f|  front\n", l[0], l[1], r[0], r[1]);
  printf("|        | |        |\n");
  printf("|%3.1f  %3.1f| |%3.1f  %3.1f|  back\n", l[3], l[2], r[3], r[2]);
  printf("+--------+ +--------+\n");
  printf("total: %g Newtons, %g kilograms\n", newtonLeft + newtonRight, (newtonLeft + newtonRight) / 9.81);
}

static void print_foot_bumpers() {
  int ll = (int)wb_touch_sensor_get_value(lfoot_lbumper);
  int lr = (int)wb_touch_sensor_get_value(lfoot_rbumper);
  int rl = (int)wb_touch_sensor_get_value(rfoot_lbumper);
  int rr = (int)wb_touch_sensor_get_value(rfoot_rbumper);

  printf("----------foot bumpers----------\n");
  printf("   left       right\n");
  printf("+--------+ +--------+\n");
  printf("|%d      %d| |%d      %d|\n", ll, lr, rl, rr);
  printf("|        | |        |\n");
  printf("|        | |        |\n");
  printf("+--------+ +--------+\n");
}

static void print_ultrasound_sensors() {
  double dist[2];
  int i;
  for (i = 0; i < 2; i++)
    dist[i] = wb_distance_sensor_get_value(us[i]);

  printf("-----ultrasound sensors-----\n");
  printf("left: %f m, right %f m\n", dist[0], dist[1]);
}

static void print_camera_image(WbDeviceTag camera) {
  const int SCALED = 2;

  int width = wb_camera_get_width(camera);
  int height = wb_camera_get_height(camera);

  // read rgb pixel values from the camera
  const unsigned char *image = wb_camera_get_image(camera);

  printf("----------camera image (gray levels)---------\n");
  printf("original resolution: %d x %d, scaled to %d x %d\n", width, height, width / SCALED, height / SCALED);

  int y, x;
  char *line = malloc(width / SCALED + 1);
  line[width / SCALED] = 0;  // add line termination
  for (y = 0; y < height; y += SCALED) {
    int count = 0;
    for (x = 0; x < width; x += SCALED) {
      unsigned char gray = wb_camera_image_get_gray(image, width, x, y);
      line[count++] = '0' + gray * 9 / 255;
    }
    line[count++] = 0;
    printf("%s\n", line);
  }
  free(line);
}

static void set_all_leds_color(int rgb) {
  // these leds take RGB values
  int i;
  for (i = 0; i < 5; i++)
    wb_led_set(leds[i], rgb);

  // ear leds are single color (blue)
  // and take values between 0 - 255
  wb_led_set(leds[5], rgb & 0xff);
  wb_led_set(leds[6], rgb & 0xff);
}

static void set_hands_angle(double angle) {
  // we must activate the 8 phalanx motors
  int j;
  for (j = 0; j < PHALANX_MAX; j++) {
    double clampedAngle = angle;
    if (clampedAngle > maxPhalanxMotorPosition[j])
      clampedAngle = maxPhalanxMotorPosition[j];
    else if (maxPhalanxMotorPosition[j] < minPhalanxMotorPosition[j])
      clampedAngle = minPhalanxMotorPosition[j];

    if (rphalanx[j])
      wb_motor_set_position(rphalanx[j], clampedAngle);
    if (lphalanx[j])
      wb_motor_set_position(lphalanx[j], clampedAngle);
  }
}

static void print_help() {
  printf("----------nao_demo----------\n");
  printf("Select the robot and use the keyboard to control it:\n");
  printf("(The 3D window need to be focused)\n");
  printf("[Up][Down]: move a few steps forward/backwards\n");
  printf("[<-][->]: make a few side steps left/right\n");
  printf("[Shift] + [<-][->]: turn left/right\n");
  printf("[U]: print ultrasound sensors\n");
  printf("[A]: print accelerometer\n");
  printf("[G]: print gyro\n");
  printf("[S]: print gps\n");
  printf("[I]: print inertial unit (roll/pitch/yaw)\n");
  printf("[F]: print foot sensors\n");
  printf("[B]: print foot bumpers\n");
  printf("[Home][End]: print scaled top/bottom camera image\n");
  printf("[PageUp][PageDown]: open/close hands\n");
  printf("[7][8][9]: change all leds RGB color\n");
  printf("[0]: turn all leds off\n");
  printf("[H]: print this help message\n");
}

static void terminate() {
  // add you cleanup code here: write results, close files, free memory, etc.
  // ...

  wb_robot_cleanup();
}

static void simulation_step() {
  if (wb_robot_step(time_step) == -1)
    terminate();
}

void run_command(int key) {

printf("run_command %d",key);

  switch (key) {
    case 'Q':
     start_motion(stand_up_from_front);
     break;
    case 'W':
     start_motion(shoot_motion);
     break;
    case 'E':
     start_motion(turn_left_40);
     break;
    case 'R':
     start_motion(turn_right_40);
     break;
    case 'T':
     start_motion(turn_left_180);
     break;
  
    case WB_KEYBOARD_LEFT:
      start_motion(side_step_left);
      break;
    case WB_KEYBOARD_RIGHT:
      start_motion(side_step_right);
      break;
    case WB_KEYBOARD_UP:
      start_motion(forwards);
      break;
    case WB_KEYBOARD_DOWN:
      start_motion(backwards);
      break;
    case WB_KEYBOARD_LEFT | WB_KEYBOARD_SHIFT:
      start_motion(turn_left_60);
      break;
    case WB_KEYBOARD_RIGHT | WB_KEYBOARD_SHIFT:
      start_motion(turn_right_60);
      break;
    case 'A':
      print_acceleration();
      break;
    case 'G':
      print_gyro();
      break;
    case 'S':
      print_gps();
      break;
    case 'I':
      print_inertial_unit();
      break;
    case 'F':
      print_foot_sensors();
      break;
    case 'B':
      print_foot_bumpers();
      break;
    case 'U':
      print_ultrasound_sensors();
      break;
    case WB_KEYBOARD_HOME:
      print_camera_image(CameraTop);
      break;
    case WB_KEYBOARD_END:
      print_camera_image(CameraBottom);
      break;
    case WB_KEYBOARD_PAGEUP:
      set_hands_angle(0.96);
      break;
    case WB_KEYBOARD_PAGEDOWN:
      set_hands_angle(0.0);
      break;
    case '7':
      set_all_leds_color(0xff0000);  // red
      break;
    case '8':
      set_all_leds_color(0x00ff00);  // green
      break;
    case '9':
      set_all_leds_color(0x0000ff);  // blue
      break;
    case '0':
      set_all_leds_color(0x000000);  // off
      break;
    case 'H':
      print_help();
      break;
     case 33:
        wbu_motion_play(hand_wave);
        break;
     default:
      printf("Unknown Command\n");
      break;
  }
}


#define PORT     45100 
#define MAXLINE  1024
int robot_udp_socket;
struct sockaddr_in servaddr, cliaddr; 

#if 1
#define MAX_COMMAND_LEN 256
#define MAX_COMMAND_CNT 50

struct command_buffer_s
{
  unsigned char cb_command_list[MAX_COMMAND_CNT][MAX_COMMAND_LEN];
  int cb_command_cnt;
};

struct command_buffer_s robot_command_buffer;

/*
 *
 *
 */
void init_command_buffer(struct command_buffer_s * command_buffer_ptr)
{
memset(&robot_command_buffer,0,sizeof(struct command_buffer_s));
}


/*
 *
 *
 */
void insert_command_to_command_buffer(struct command_buffer_s * command_buffer_ptr,unsigned char * command_ptr)
{
if(command_ptr == NULL || command_ptr[0] == '\0')
  return;
  
strcpy(&command_buffer_ptr->cb_command_list[command_buffer_ptr->cb_command_cnt][0],command_ptr);
command_buffer_ptr->cb_command_cnt++;
}

/*
 *
 *
 */
unsigned char * get_command_from_command_buffer(struct command_buffer_s * command_buffer_ptr, int index)
{
return &command_buffer_ptr->cb_command_list[index];
}

/*
 *
 *
 */
void robot_tokenize_received_data(struct command_buffer_s * command_buffer_ptr,char * str_data_ptr)
{
   char str[80] = "This is - www.tutorialspoint.com - website";
   const char s[2] = ";";
   char *token;
   
   /* get the first token */
   token = strtok(str_data_ptr, s);
   
   /* walk through other tokens */
   while( token != NULL ) {
      printf( "Token %s\n", token );
    
      insert_command_to_command_buffer(command_buffer_ptr,token);
      token = strtok(NULL, s);
      
   }
}

/*
 *
 */
void robot_command_receiver_engine(unsigned char * command_ptr)
{
int robot_command = 0;

init_command_buffer(&robot_command_buffer);

robot_tokenize_received_data(&robot_command_buffer,command_ptr);

printf("Command Count %d\n",robot_command_buffer.cb_command_cnt );
printf("Command Value %c\n",robot_command_buffer.cb_command_list[0][0] );

if(robot_command_buffer.cb_command_cnt <= 0)
  return;

printf("Point\n");

printf("Atoi operation %c %c",robot_command_buffer.cb_command_list[0][0],robot_command_buffer.cb_command_list[0][1]);

robot_command = atoi(&robot_command_buffer.cb_command_list[0][0]);

printf("Executed Command %d",robot_command);
run_command(robot_command);


}
 

/*
 *
 *
 */
void create_udp_server()
{ 
    char buffer[MAXLINE]; 
    char *hello = "Hello from server"; 
      
    printf("Creating UDP server\n");  
      
    robot_udp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    // Creating socket file descriptor
    if ( robot_udp_socket == INVALID_SOCKET ) {
        //perror("socket creation failed %d",sockfd);
        printf("Error Code %d",robot_udp_socket);
        _sleep(2000);
        exit(EXIT_FAILURE);
    }
    
    printf("UDP server created\n");  
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Filling server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
      
    printf("Bind UDP server\n");  
    // Bind the socket with the server address 
    if ( bind(robot_udp_socket, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    int len, n; 
  
    len = sizeof(cliaddr);  //len is value/resuslt 
  
  
    printf("Udp waiting at recvFrom..\n");
    
    n= 1;
    while(n > 0)
    {
      n = recvfrom(robot_udp_socket, (char *)buffer, MAXLINE, 0, ( struct sockaddr *) &cliaddr,&len); 
      
      printf("RecvFrom Executed..\n");  
      
      if (n == SOCKET_ERROR)
      {
  	printf("Receive Error.. %d\n",WSAGetLastError());
  	Sleep(1000);
  	exit(0);
      }
      
      buffer[n] = '\0'; 
      
      printf("Client : %s\n", buffer);  
      
      robot_command_receiver_engine(buffer);
      
      //sendto(robot_udp_socket, (const char *)hello, strlen(hello),0, (const struct sockaddr *) &cliaddr, len); 
      //printf("Hello message sent.\n"); 
    }
    
    closesocket(robot_udp_socket);
      
    return 0; 
}

/*
 *
 *
 */
void robot_udp_sender(unsigned char * data_ptr,int data_size)
{
int len;
if(robot_udp_socket == 0)
  {
    printf("robot_udp_sender: socket send error\n");
  }
  
len = sizeof(cliaddr);
 
sendto(robot_udp_socket, (const char *)data_ptr, strlen(data_ptr),0, (const struct sockaddr *) &cliaddr, len); 
  
}

#endif

 
/*
 *
 *
 */
void udp_receiver_thread()
{
int i = 0;
printf("\n\n Task created ..\n\n");
while(true)
{
  _sleep(1000);
  printf("Task exec time ms %d\n",i++);
  create_udp_server();
  break;
}
}


/*
 *
 *
 */
void configure_wsa()
{

  WSADATA wsa_rec;
  int len, n,out_len;

    //Initialise winsock
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2,2),&wsa_rec) != 0)
    {

        exit(0);
    }

}


// main function
int main() {
  unsigned char param1 = 0;

  configure_wsa();
  
  // call this before any other call to a Webots function
  printf("\n>>> State 1\n\n");
  wb_robot_init();

  // simulation step in milliseconds
  time_step = wb_robot_get_basic_time_step();

  // initialize stuff
  find_and_enable_devices();
  load_motion_files();

  // print instructions
  print_help();
  wb_robot_task_new(udp_receiver_thread,(void *)&param1);

  // walk forwards
  wbu_motion_set_loop(hand_wave, true);
  wbu_motion_play(hand_wave);

  // until a key is pressed
  int key = -1;
  do {
    simulation_step();
    key = wb_keyboard_get_key();
  } while (key >= 0);

  // stop looping this motion
  wbu_motion_set_loop(hand_wave, false);
printf("Start read\n");


printf("Values %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",  WB_KEYBOARD_END,
  WB_KEYBOARD_HOME,
  WB_KEYBOARD_LEFT,
  WB_KEYBOARD_UP,
  WB_KEYBOARD_RIGHT,
  WB_KEYBOARD_DOWN,
  WB_KEYBOARD_PAGEUP,
  WB_KEYBOARD_PAGEDOWN,
  WB_KEYBOARD_NUMPAD_HOME,
  WB_KEYBOARD_NUMPAD_LEFT,
  WB_KEYBOARD_NUMPAD_UP,
  WB_KEYBOARD_NUMPAD_RIGHT,
  WB_KEYBOARD_NUMPAD_DOWN,
  WB_KEYBOARD_NUMPAD_END,
  WB_KEYBOARD_KEY,
  WB_KEYBOARD_SHIFT,
  WB_KEYBOARD_CONTROL,
  WB_KEYBOARD_ALT);


  // read keyboard and execute user commands
  while (1) {
    if (key >= 0){
      printf("Keyboard read\n");
      run_command(key);
      
      }

    simulation_step();
    key = wb_keyboard_get_key();
  }
 
  return 0;
}

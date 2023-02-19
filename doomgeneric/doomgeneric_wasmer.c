//doomgeneric for soso os

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>

#include <stdbool.h>
#include <assert.h>

const uint8_t KEY_PRESS = 1;
const uint8_t KEY_RELEASE = 3;

const uint8_t MOUSE_MOVE = 2;
const uint8_t MOUSE_PRESS_LEFT = 4;
const uint8_t MOUSE_PRESS_RIGHT = 5;
const uint8_t MOUSE_PRESS_MIDDLE = 7;

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char convertToDoomKey(unsigned int key){
  switch (key)
    {
    case 13:
      key = KEY_ENTER;
      break;
    case 27:
      key = KEY_ESCAPE;
      break;
    case 37:
      key = KEY_LEFTARROW;
      break;
    case 39:
      key = KEY_RIGHTARROW;
      break;
    case 38:
      key = KEY_UPARROW;
      break;
    case 40:
      key = KEY_DOWNARROW;
      break;
    case 17:
    case 18:
      key = KEY_FIRE;
      break;
    case 32:
      key = KEY_USE;
      break;
    case 16:
      key = KEY_RSHIFT;
      break;
    default:
      key = key;
      break;
    }

  return key;
}

static void addKeyToQueue(int pressed, unsigned int key){
  unsigned short keyData = (pressed << 8) | convertToDoomKey(key);

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

void DG_Init(){
  // Initialize framebuffer
  int fd = open("/_wasmer/dev/fb0/virtual_size", O_RDWR);
  assert(fd > 0);

  lseek(fd, 0, SEEK_SET);

  const size_t data_size = 64;
  const char data[data_size];
  snprintf(data, data_size, "%dx%d", DOOMGENERIC_RESX, DOOMGENERIC_RESY);
  write(fd, data, data_size);
}

void DG_DrawFrame()
{
  // Handle key presses
  int fd_input = open("/_wasmer/dev/fb0/input", O_RDWR);
  assert(fd_input > 0);

  const size_t input_buffer_size = 128;
  char input_buffer[input_buffer_size];
  ssize_t length = read(fd_input, input_buffer, input_buffer_size);

  ssize_t i = 0;
  while(i < length) {
    printf("Got some inputs %d %d\n", input_buffer[i], input_buffer[i+1]);

    char event = input_buffer[i];

    if(event == KEY_PRESS || event == KEY_RELEASE) {
      addKeyToQueue(event == KEY_PRESS, input_buffer[i+1]);
      i += 2;
    }
    else if(
      event == MOUSE_MOVE
      || event == MOUSE_PRESS_LEFT
      || event == MOUSE_PRESS_RIGHT
      || event == MOUSE_PRESS_MIDDLE
    ) {
      i += 9;
    }
  }

  // Write data to framebuffer
  int fd_fb = open("/_wasmer/dev/fb0/fb", O_RDWR);
  assert(fd_fb > 0);
  lseek(fd_fb, 0, SEEK_SET);
  write(fd_fb, DG_ScreenBuffer, DOOMGENERIC_RESX * DOOMGENERIC_RESX * sizeof(uint32_t));


  // Trigger redraw
  int fd_draw = open("/_wasmer/dev/fb0/draw", O_RDWR);
  assert(fd_draw > 0);
  lseek(fd_draw, 0, SEEK_SET);
  int fb = 0;
  write(fd_draw, &fb, 1);
}

void DG_SleepMs(uint32_t ms)
{
  struct timespec ts;
  ts.tv_sec = ms / 1000;
  ts.tv_nsec = (ms % 1000) * 1000000;
  nanosleep(&ts, NULL);
}

uint32_t DG_GetTicksMs()
{
  struct timeval  tp;
  struct timezone tzp;

  gettimeofday(&tp, &tzp);

  return (tp.tv_sec * 1000) + (tp.tv_usec / 1000); /* return milliseconds */
}

int DG_GetKey(int* pressed, unsigned char* doomKey)
{
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex){
    //key queue is empty
    return 0;
  }else{
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

  return 0;
}

void DG_SetWindowTitle(const char * title)
{
  printf("!!!!! DG_SetWindowTitle\n");
}

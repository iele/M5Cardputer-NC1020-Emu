#if defined(ARDUINO)
#include <M5Cardputer.h>
#include <M5GFX.h>
#include "SPI.h"
#include "SD.h"
#else
#include <M5Unified.h>
#endif
#include "nc1020.h"

// #define SD_SPI_SCK_PIN 36
// #define SD_SPI_MISO_PIN 35
// #define SD_SPI_MOSI_PIN 37
// #define SD_SPI_CS_PIN 4

#define SD_SPI_SCK_PIN 40
#define SD_SPI_MISO_PIN 39
#define SD_SPI_MOSI_PIN 14
#define SD_SPI_CS_PIN 12

#define FRAME_RATE 30
#define FRAME_INTERVAL (1000 / FRAME_RATE)

#define SCREEN_WIDTH 160
#define SCREEN_HEIGHT 80
#define LINE_SIZE 2

static uint8_t lcd_buf[SCREEN_WIDTH * SCREEN_HEIGHT / 8];
static uint8_t lcd_out[SCREEN_WIDTH * SCREEN_HEIGHT];

M5Canvas renderer;

#define KEY_SIZE 0x3f

// keyboard Mapping:
// BtnA: POWER 0x0f
// ROW1: es 1  2  3  4  5  6  7  8  9  0  f1 f3 f2
// ROW2: _  Q  W  E  R  T  Y  U  I  O  P  pu pd f4
// ROW3: sh cp A  S  D  F  G  H  J  K  L  ^  .  en
// ROW4: hp 0  ex Z  X  C  V  B  N  M  <  v  >  =
// when ex ->
// ROW1: _  yh mp js xc cy qt wl _  _  _  _  _  _

const char kb_map[4][14] = {
    {0x3B, 0x0b, 0x0c, 0x0d, 0x0a, 0x09, 0x08, 0x0e, 0x25, 0x26, 0x3c, 0x10, 0x12, 0x11},
    {0x00, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x18, 0x1c, 0x37, 0x1e, 0x13},
    {0x39, 0x3a, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x19, 0x1a, 0x3d, 0x1d},
    {0x38, 0x3c, 0x00, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x3f, 0x1b, 0x1f, 0x3e}};

bool kb_state[KEY_SIZE] = {0};

const char extend_kb_map[14] = {
    0x3B, 0x0b, 0x0c, 0x0d, 0x0a, 0x09, 0x08, 0x0e, 0x25, 0x26, 0x3c, 0x10, 0x12, 0x11};

void Render()
{
  if (lcd_buf == NULL)
  {
    return;
  }

  int cnt;
  for (int i = 0; i < 1600; i++)
    cnt += lcd_buf[i];

  // Serial.println("render copy");
  renderer.setBuffer(lcd_buf, SCREEN_WIDTH, SCREEN_HEIGHT, 1);
  renderer.display();

  // Serial.println("render push");
  renderer.pushRotateZoomWithAA(&M5.Display, M5.Display.width() / 2,
                                M5.Display.height() / 2, 0,
                                //                             1, 1);
                                (float)M5.Display.width() / (float)SCREEN_WIDTH, (float)M5.Display.height() / (float)SCREEN_HEIGHT);
  M5.Display.display();
}

void ProcessKeyboard()
{
  if (M5.BtnA.wasPressed())
  {
    wqx::SetKey(0x0f, true);
  }
  if (M5.BtnA.wasReleased())
  {
    wqx::SetKey(0x0f, false);
  }

#if defined(Arduino)
  std::vector<Point2D_t> keys = M5Cardputer.Keyboard.keyList();
  bool new_kb_state[KEY_SIZE] = {0};
  bool ex_mode = false;

  // find ex_key first
  for (auto &i : keys)
  {
    Serial.printf("%d %d\n", i.x, i.y);
    uint8_t key_code = kb_map[i.y][i.x];
    if (key_code == 0)
    {
      ex_mode = true;
      break;
    }
  }
  // map key_code
  for (auto &i : keys)
  {
    uint8_t key_code;
    // first line
    if (ex_mode && i.y == 0)
    {
      key_code = extend_kb_map[i.x];
    }
    else
    {
      key_code = kb_map[i.y][i.x];
    }
    if (key_code == 0)
    {
      continue;
    }
    new_kb_state[key_code] = true;
  }

  for (int i = 0; i < KEY_SIZE; i++)
  {
    if (new_kb_state[i] == true && kb_state[i] == false)
    {
      Serial.printf("key down %d\n", i);
      wqx::SetKey(i, true);
    }
    if (new_kb_state[i] == false && kb_state[i] == true)
    {
      Serial.printf("key up %d\n", i);
      wqx::SetKey(i, false);
    }
    kb_state[i] = new_kb_state[i];
  }
#endif
}

void RunGame()
{
  bool loop = true;

  while (loop)
  {
    M5.update();
    uint64_t tick = lgfx::v1::millis();
    // Serial.printf("run %ld\n", tick);
    wqx::RunTimeSlice(FRAME_INTERVAL, false);
    // Serial.printf("keyboard %ld\n", tick);
    ProcessKeyboard();

    // Serial.printf("lcd %ld\n", tick);
    if (!wqx::CopyLcdBuffer(lcd_buf))
    {
      M5.Display.drawString("Failed to copy buffer renderer.\n", 0, 0);
    }
    else
    {
      Render();
    }

    uint64_t diff = lgfx::v1::millis() - tick;
    uint64_t delay_mills = FRAME_INTERVAL < diff ? 0 : (FRAME_INTERVAL - diff);
    // Serial.printf("delay %ld\n", delay_mills);
    lgfx::v1::delay(delay_mills);
    diff = lgfx::v1::millis() - tick;

    M5.Display.setCursor(0, 0);
    M5.Display.printf("HZ: %d", 1000 / diff);
  }
}

void sd_begin()
{
  M5.Display.setTextSize(1);
  M5.Display.print("SDCard Checking...\n");

#if defined(Arduino)
  SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
  if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000))
  {
    M5.Display.print("SDCard Failed\n");
    while (1)
      ;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    M5.Display.print("SDCard Failed\n");
    return;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  M5.Display.printf("SD Size: %lluMB\n", cardSize);
#endif
}

void setup()
{
  M5.begin();
  M5.Display.setRotation(1);
  M5.Display.display();
  sd_begin();

  // todo testing
  renderer.setColorDepth(8);
  renderer.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  renderer.createPalette();
  renderer.setPaletteColor(1, TFT_BLACK);
  renderer.setPaletteColor(0, TFT_WHITE);
  renderer.setPivot(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

  wqx::WqxRom rom = {
#if defined(ARDUINO)
      .romPath = "/obj_lu.bin",
      .norFlashPath = "/nc1020.fls",
      .statesPath = "/nc1020.sts",
      .romTempPath = "/__tmp_obj_lu.bin",
      .norTempPath = "/__tmp_nc1020.fls",
#else
      .romPath = "./obj_lu.bin",
      .norFlashPath = "./nc1020.fls",
      .statesPath = "./nc1020.sts",
      .romTempPath = "./__tmp_obj_lu.bin",
      .norTempPath = "./__tmp_nc1020.fls",
#endif
  };

  M5.Display.print("Load ROM...\n");
  wqx::Initialize(rom);
  M5.Display.print("Load NOR...\n");
  wqx::LoadNC1020();
  M5.Display.print("\nLoad Completed\n");
  M5.Display.clearDisplay();
}

void loop()
{
  RunGame();
  wqx::SaveNC1020();
}

#if !defined(ARDUINO)
extern "C"
{
__attribute__((weak))
int user_func(bool* running)
{
  setup();
  do
  {
    loop();
  } while (*running);
  return 0;
}

int main(int, char**)
{
  // The second argument is effective for step execution with breakpoints.
  // You can specify the time in milliseconds to perform slow execution that ensures screen updates.
  return lgfx::Panel_sdl::main(user_func, 128);
}
}
#endif


#include <M5Cardputer.h>
#include <M5GFX.h>

#include "nc1020.h"

#include "SPI.h"
#include "SD.h"

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

M5Canvas renderer;

void setup()
{
  M5.begin();
  sd_begin();

  //todo testing
  renderer.setColorDepth(8);
  renderer.createSprite(SCREEN_WIDTH, SCREEN_HEIGHT);
  renderer.createPalette();
  renderer.setPaletteColor(0, TFT_BLACK);
  renderer.setPaletteColor(1, TFT_WHITE);

  wqx::WqxRom rom = {
      .romPath = "/obj_lu.bin",
      .norFlashPath = "/nc1020.fls",
      .statesPath = "/nc1020.sts",

      .romTempPath = "/__tmp_obj_lu.bin",
      .norTempPath = "/__tmp_nc1020.fls",
  };
  wqx::Initialize(rom);
  wqx::LoadNC1020();
}

void loop()
{
  RunGame();
  wqx::SaveNC1020();
}

void Render()
{
  renderer.clearDisplay();
  unsigned char *buffer = (unsigned char *)renderer.getBuffer();
  for (int i = 0; i < sizeof(lcd_buf); ++i)
  {
    for (int j = 0; j < 8; ++j)
    {
      bool pixel = (lcd_buf[i] & (1 << (7 - j))) != 0;
      memcpy(buffer, (const void *)(pixel ? 1 : 0), sizeof(unsigned char));
    }
  }
  renderer.pushRotateZoom(&M5.Display, 0, (float)M5.Display.width() / renderer.width(), (float)M5.Display.height() / renderer.height());
}

void RunGame()
{
  bool loop = true;

  while (loop)
  {
    M5.update();
    uint32_t tick = millis();

    wqx::RunTimeSlice(FRAME_INTERVAL, false);

    while (M5Cardputer.Keyboard.isChange())
    {
      if (M5Cardputer.Keyboard.isPressed())
      {
        //todo keyboard map
        Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();
        wqx::SetKey(status.hid_keys[0], true);
      }
    }

    if (!wqx::CopyLcdBuffer(lcd_buf))
    {
      std::cout << "Failed to copy buffer renderer." << std::endl;
    }
    Render();
    tick = millis() - tick;
    delay(FRAME_INTERVAL < tick ? 0 : FRAME_INTERVAL - tick);
  }
}

 void sd_begin()
  {
    SPI.begin(SD_SPI_SCK_PIN, SD_SPI_MISO_PIN, SD_SPI_MOSI_PIN, SD_SPI_CS_PIN);
    if (!SD.begin(SD_SPI_CS_PIN, SPI, 25000000))
    {
      M5.Display.print("SDCard Failed");
      while (1)
        ;
    }
     uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE) {
      M5.Display.print("SDCard Failed");
        return;
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    M5.Display.printf("SD Size: %lluMB\n", cardSize);
  }
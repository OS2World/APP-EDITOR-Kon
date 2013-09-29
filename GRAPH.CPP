#include "bowl.h"

int AGraphics::fontOpaque = 1;
int AGraphics::ACTIVE = 1;
int AGraphics::INACTIVESHADE = 2;
int AGraphics::INACTIVE = 4;
BOOL AGraphics::useBigFont = FALSE;
BOOL AGraphics::unicode = FALSE;   // WideCharToMultiByte for conversion!

ColorValue Color::BLACK = 0x000000;
ColorValue Color::RED = 0xFF0000;
ColorValue Color::DARKRED = 0x820000;
ColorValue Color::DARKGREEN = 0x008200;
ColorValue Color::GREEN = 0x00FF00;
ColorValue Color::BLUE = 0x0000FF;
ColorValue Color::DARKBLUE = 0x000082;
ColorValue Color::PINK = 0xFF00FF;
ColorValue Color::DARKPINK = 0x820082;
ColorValue Color::CYAN = 0x00FFFF;
ColorValue Color::DARKCYAN = 0x008282;
ColorValue Color::YELLOW = 0xFFFF00;
ColorValue Color::BROWN = 0x828200;
ColorValue Color::LIGHTGRAY = 0xCCCCCC;
ColorValue Color::GRAY = 0x828282;
ColorValue Color::WHITE= 0xFFFFFF;
ColorValue Color::DEFAULT = 0xFFFFFFFF;
ColorValue Color::BKGR = 0xCCCCCC;
ColorValue Color::LIGHT = 0xFFFFFF;
ColorValue Color::SHADOW = 0x828282;

int Font::BOLD = 1;
int Font::ITALIC = 2;
int Font::UNDERSCORE = 4;




#ifndef COLOUR_H
#define COLOUR_H

enum Colour
    {
    BLACK   = 30,
    RED     = 31,
    GREEN   = 32,
    YELLOW  = 33,
    BLUE    = 34,
    MAGENTA = 35,
    CYAN    = 36,
    RESET   = 0
    };

void ColouredfPrintf(Colour colour, FILE* fp, const char* format, ...);

void ColouredPrintf(Colour colour, const char* format, ...);
#endif // COLOUR_H

#ifndef COLOUR_H
#define COLOUR_H

enum Colour
    {
    BLACK   = 30, //!< Черный.
    RED     = 31, //!< Красный.
    GREEN   = 32, //!< Зеленый.
    YELLOW  = 33, //!< Желтый.
    BLUE    = 34, //!< Синий.
    MAGENTA = 35, //!< Пурпурный.
    CYAN    = 36, //!< Голубой.
    RESET   = 0   //!< Сброс.
    };

Error_t ColouredPrintf(Colour colour, const char* format, ...);
#endif // COLOUR_H

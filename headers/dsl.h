#define PUSH(arg)   StackPush(&spu.stk, arg)
#define POP()       StackPop(&spu.stk)
#define TOP()       StackTop(&spu.stk)
#define ZERO_DIV    printf("ERROR: dividing by zero");\
                    return CALCULATION_ERROR;
#define INT_ARG     *((int*) &spu.code[spu.ip])


DEF_CMD (push, 1,  1,   {
                        char arg_t = spu.code[spu.ip] >> ARG_TYPE_BORDER;
                        if (arg_t & DataType)
                            {
                            spu.ip++;
                            PUSH(INT_ARG);
                            spu.ip += sizeof(int) - 1;
                            }
                        else if (arg_t & RegisterType)
                            {
                            spu.ip++;
                            PUSH(spu.registers[INT_ARG]);
                            spu.ip += sizeof(int) - 1;
                            }
                        else if (arg_t & MemoryType)
                            {
                            spu.ip++;
                            PUSH(spu.memory[INT_ARG]);
                            spu.ip += sizeof(int) - 1;
                            }
                        })

DEF_CMD (pop,  2,  1,   {
                        char arg_t = spu.code[spu.ip] >> ARG_TYPE_BORDER;
                        if (arg_t & RegisterType)
                            {
                            spu.ip++;
                            spu.registers[INT_ARG] = POP();
                            spu.ip += sizeof(int) - 1;
                            }
                        else if (arg_t & MemoryType)
                            {
                            spu.ip++;
                            spu.memory[INT_ARG] = POP();
                            spu.ip += sizeof(int) - 1;
                            }
                        })

DEF_CMD (in,   3,  0,   {
                        float arg = 0;
                        printf("\nВведите данные: ");
                        scanf("%f", &arg);
                        PUSH((int) (arg * FIXED_POINT_MULTIPIER));
                        })

DEF_CMD (out,  4,  0,   {
                        printf("Нынешнее значение: %f\n", (float) TOP() / FIXED_POINT_MULTIPIER);
                        })

DEF_CMD (add,  5,  0,   {
                        PUSH(POP() + POP());
                        })

DEF_CMD (sub,  6,  0,   {
                        int a = POP();
                        int b = POP();
                        PUSH(b - a);
                        })

DEF_CMD (mul,  7,  0,   {
                        PUSH(POP() * POP() / FIXED_POINT_MULTIPIER);
                        })

DEF_CMD (div,  8,  0,   {
                        int a = POP();
                        int b = POP();
                        if (a != 0)
                            {
                            PUSH(b / a * FIXED_POINT_MULTIPIER);
                            }
                        else
                            {
                            ZERO_DIV;
                            }
                        })

DEF_CMD (mod,  9,  0,   {
                        int a = POP();
                        int b = POP();
                        if (a != 0)
                            {
                            PUSH(b % a);
                            }
                        else
                            {
                            ZERO_DIV;
                            }
                        })

DEF_CMD (abs,  10, 0,   {
                        PUSH(abs(POP()));
                        })

DEF_CMD (sin,  11, 0,   {
                        float a = (float) POP() / FIXED_POINT_MULTIPIER;
                        float b = sin(a) * FIXED_POINT_MULTIPIER;
                        PUSH((int) b);
                        })

DEF_CMD (cos,  12, 0,   {
                        float a = (float) POP() / FIXED_POINT_MULTIPIER;
                        float b = cos(a) * FIXED_POINT_MULTIPIER;
                        PUSH((int) b);
                        })

DEF_CMD (sqrt, 13, 0,   {
                        float a = (float) POP() / FIXED_POINT_MULTIPIER;
                        float b = sqrt(a) * FIXED_POINT_MULTIPIER;
                        PUSH((int) b);
                        })

DEF_CMD (neg, 14, 0,   {
                        PUSH(-POP());
                        })

DEF_CMD (mem, 25, 0,   {
                        MemDump(&spu);
                        })

DEF_CMD (hlt,  31, 0,   {
                        return EXIT;
                        })

MAKE_COND_JUMP (jmp,  16,  {
                            spu.ip++;
                            spu.ip = INT_ARG - 1;
                            })

MAKE_COND_JUMP (jae,   17,  {
                            spu.ip++;
                            int a = POP();
                            int b = POP();
                            if (b >= a) spu.ip = INT_ARG - 1;
                            else spu.ip += sizeof(int) - 1;
                            })

MAKE_COND_JUMP (ja,  18,    {
                            spu.ip++;
                            int a = POP();
                            int b = POP();
                            if (b > a) spu.ip = INT_ARG - 1;
                            else spu.ip += sizeof(int) - 1;
                            })

MAKE_COND_JUMP (jbe,   19,  {
                            spu.ip++;
                            int a = POP();
                            int b = POP();
                            if (b <= a) spu.ip = INT_ARG - 1;
                            else spu.ip += sizeof(int) - 1;
                            })

MAKE_COND_JUMP (jb,  20,    {
                            spu.ip++;
                            int a = POP();
                            int b = POP();
                            if (b < a) spu.ip = INT_ARG - 1;
                            else spu.ip += sizeof(int) - 1;
                            })

MAKE_COND_JUMP (je,   21,   {
                            spu.ip++;
                            int a = POP();
                            int b = POP();
                            if (a == b) spu.ip = INT_ARG - 1;
                            else spu.ip += sizeof(int) - 1;
                            })

MAKE_COND_JUMP (jne,  22,   {
                            spu.ip++;
                            int a = POP();
                            int b = POP();
                            if (a != b) spu.ip = INT_ARG - 1;
                            else spu.ip += sizeof(int) - 1;
                            })

MAKE_COND_JUMP (call, 23,   {
                            spu.ip++;
                            PUSH(spu.ip + sizeof(int) - 1);
                            spu.ip = INT_ARG - 1;
                            })

DEF_CMD (ret,  24,  0,  {
                        spu.ip = POP();
                        })

#define PUSH(arg)   StackPush(&spu.stk, arg)
#define POP         StackPop(&spu.stk)
#define ZERO_DIV    printf("ERROR: dividing by zero");\
                    exit(1);
#define CUR_VALUE   spu.stk.data[spu.stk.size - 1]

DEF_COM (push, 1,  1,   {char arg_t = spu.code[spu.ip] >> 5;
                        if (arg_t & (1 << 0))
                            {
                            spu.ip++;
                            int arg = *((int*) &spu.code[spu.ip]) * MUL_COEFF;
                            PUSH(arg);
                            spu.ip += sizeof(int) - 1;
                            }
                        else if (arg_t & (1 << 1))
                            {
                            spu.ip++;
                            PUSH(spu.registers[spu.code[spu.ip]]);
                            }
                        })

DEF_COM (pop,  2,  1,   {spu.ip++;
                        spu.registers[spu.code[spu.ip]] = POP;})

DEF_COM (in,   3,  0,   {printf("\nВведите данные: ");
                        float arg = 0;
                        scanf("%f", &arg);
                        PUSH((int) (arg * MUL_COEFF));
                        })

DEF_COM (out,  4,  0,   {printf("Нынешнее значение: %f\n", (float) CUR_VALUE / MUL_COEFF);
                        StackDump(&spu.stk, 0);
                        StackDump(&spu.return_codes, 0);
                        SpuDump(&spu);})

DEF_COM (add,  5,  0,   {PUSH(POP + POP);})

DEF_COM (sub,  6,  0,   {int a = POP;
                        int b = POP;
                        PUSH(b - a);})

DEF_COM (mul,  7,  0,   {PUSH(POP * POP / MUL_COEFF);})

DEF_COM (div,  8,  0,   {int a = POP;
                        int b = POP;
                        if (a != 0)
                            {
                            PUSH(b / a * MUL_COEFF);
                            }
                        else
                            {
                            ZERO_DIV;
                            }})

DEF_COM (mod,  9,  0,   {})

DEF_COM (abs,  10, 0,   {})

DEF_COM (sin,  11, 0,   {})

DEF_COM (cos,  12, 0,   {})

DEF_COM (sqrt, 13, 0,   {})

DEF_COM (hlt,  -1, 0,   {exit(1);})

MAKE_COND_JUMP (jmp,  16,  {spu.ip++;
                            spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (jae,   17,  {spu.ip++;
                            int a = POP;
                            int b = POP;
                            if (b >= a) spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (ja,  18,  {spu.ip++;
                            int a = POP;
                            int b = POP;
                            if (b > a) spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (jbe,   19,  {spu.ip++;
                            int a = POP;
                            int b = POP;
                            if (b <= a) spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (jb,  20,  {spu.ip++;
                            int a = POP;
                            int b = POP;
                            if (b < a) spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (je,   21,  {spu.ip++;
                            int a = POP;
                            int b = POP;
                            if (a == b) spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (jne,  22,  {spu.ip++;
                            int a = POP;
                            int b = POP;
                            if (a != b) spu.ip = spu.code[spu.ip] - 1;})

MAKE_COND_JUMP (call, 23,  {spu.ip++;
                            StackPush(&spu.return_codes, spu.ip);
                            spu.ip = spu.code[spu.ip] - 1;})

DEF_COM (ret,  24,  0,  {spu.ip = StackPop(&spu.return_codes);})

CFLAGS=-D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie -fPIE -Werror=vla -fsanitize=address,alignment,bool,bounds,enum,float-cast-overflow,float-divide-by-zero,integer-divide-by-zero,leak,nonnull-attribute,null,object-size,return,returns-nonnull-attribute,shift,signed-integer-overflow,undefined,unreachable,vla-bound,vptr

compiler: oneginlib.o
	g++ compiler.cpp ASM/oneginlib.o -o ASM/compiler # $(CFLAGS)

spu: stack.o colour.o logfiles.o
	g++ spu.cpp ASM/stack.o ASM/colour.o ASM/logfiles.o -o ASM/spu # $(CFLAGS)

disassembler:
	g++ disassembler.cpp -o ASM/disassembler # $(CFLAGS)

compiler.o: compiler.cpp
	g++ -c compiler.cpp -o ASM/compiler.o

disassembler.o: disassembler.cpp
	g++ -c disassembler.cpp -o ASM/disassembler.o

spu.o: spu.cpp
	g++ -c spu.cpp -o ASM/spu.o

hash.o: MyLib/hash.cpp
	g++ -c MyLib/hash.cpp -o ASM/hash.o

oneginlib.o: MyLib/oneginlib.cpp
	g++ -c MyLib/oneginlib.cpp -o ASM/oneginlib.o

stack.o: MyLib/stack.cpp
	g++ -c MyLib/stack.cpp -o ASM/stack.o

colour.o: MyLib/colour.cpp
	g++ -c MyLib/colour.cpp -o ASM/colour.o

logfiles.o: MyLib/logfiles.cpp
	g++ -c MyLib/logfiles.cpp -o ASM/logfiles.o

clean:
	rm -rf *.o

clean_o:
	rm -rf *.o

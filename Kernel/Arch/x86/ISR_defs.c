#include <stdint.h>
#include "Interrupts.h"
#include "GDT.h"
#include "IDT.h"

#include "ISR.h"

// Define the ISR function (for the C code), 
// as well as the x86_ISR_Initialize function
// The functions are implemented in ISR_defs.asm
// See ISR.asm as well

void __attribute__((cdecl)) x86_ISR_0();
void __attribute__((cdecl)) x86_ISR_1();
void __attribute__((cdecl)) x86_ISR_2();
void __attribute__((cdecl)) x86_ISR_3();
void __attribute__((cdecl)) x86_ISR_4();
void __attribute__((cdecl)) x86_ISR_5();
void __attribute__((cdecl)) x86_ISR_6();
void __attribute__((cdecl)) x86_ISR_7();
void __attribute__((cdecl)) x86_ISR_8();
void __attribute__((cdecl)) x86_ISR_9();
void __attribute__((cdecl)) x86_ISR_10();
void __attribute__((cdecl)) x86_ISR_11();
void __attribute__((cdecl)) x86_ISR_12();
void __attribute__((cdecl)) x86_ISR_13();
void __attribute__((cdecl)) x86_ISR_14();
void __attribute__((cdecl)) x86_ISR_15();
void __attribute__((cdecl)) x86_ISR_16();
void __attribute__((cdecl)) x86_ISR_17();
void __attribute__((cdecl)) x86_ISR_18();
void __attribute__((cdecl)) x86_ISR_19();
void __attribute__((cdecl)) x86_ISR_20();
void __attribute__((cdecl)) x86_ISR_21();
void __attribute__((cdecl)) x86_ISR_22();
void __attribute__((cdecl)) x86_ISR_23();
void __attribute__((cdecl)) x86_ISR_24();
void __attribute__((cdecl)) x86_ISR_25();
void __attribute__((cdecl)) x86_ISR_26();
void __attribute__((cdecl)) x86_ISR_27();
void __attribute__((cdecl)) x86_ISR_28();
void __attribute__((cdecl)) x86_ISR_29();
void __attribute__((cdecl)) x86_ISR_30();
void __attribute__((cdecl)) x86_ISR_31();
void __attribute__((cdecl)) x86_ISR_32();
void __attribute__((cdecl)) x86_ISR_33();
void __attribute__((cdecl)) x86_ISR_34();
void __attribute__((cdecl)) x86_ISR_35();
void __attribute__((cdecl)) x86_ISR_36();
void __attribute__((cdecl)) x86_ISR_37();
void __attribute__((cdecl)) x86_ISR_38();
void __attribute__((cdecl)) x86_ISR_39();
void __attribute__((cdecl)) x86_ISR_40();
void __attribute__((cdecl)) x86_ISR_41();
void __attribute__((cdecl)) x86_ISR_42();
void __attribute__((cdecl)) x86_ISR_43();
void __attribute__((cdecl)) x86_ISR_44();
void __attribute__((cdecl)) x86_ISR_45();
void __attribute__((cdecl)) x86_ISR_46();
void __attribute__((cdecl)) x86_ISR_47();
void __attribute__((cdecl)) x86_ISR_48();
void __attribute__((cdecl)) x86_ISR_49();
void __attribute__((cdecl)) x86_ISR_50();
void __attribute__((cdecl)) x86_ISR_51();
void __attribute__((cdecl)) x86_ISR_52();
void __attribute__((cdecl)) x86_ISR_53();
void __attribute__((cdecl)) x86_ISR_54();
void __attribute__((cdecl)) x86_ISR_55();
void __attribute__((cdecl)) x86_ISR_56();
void __attribute__((cdecl)) x86_ISR_57();
void __attribute__((cdecl)) x86_ISR_58();
void __attribute__((cdecl)) x86_ISR_59();
void __attribute__((cdecl)) x86_ISR_60();
void __attribute__((cdecl)) x86_ISR_61();
void __attribute__((cdecl)) x86_ISR_62();
void __attribute__((cdecl)) x86_ISR_63();
void __attribute__((cdecl)) x86_ISR_64();
void __attribute__((cdecl)) x86_ISR_65();
void __attribute__((cdecl)) x86_ISR_66();
void __attribute__((cdecl)) x86_ISR_67();
void __attribute__((cdecl)) x86_ISR_68();
void __attribute__((cdecl)) x86_ISR_69();
void __attribute__((cdecl)) x86_ISR_70();
void __attribute__((cdecl)) x86_ISR_71();
void __attribute__((cdecl)) x86_ISR_72();
void __attribute__((cdecl)) x86_ISR_73();
void __attribute__((cdecl)) x86_ISR_74();
void __attribute__((cdecl)) x86_ISR_75();
void __attribute__((cdecl)) x86_ISR_76();
void __attribute__((cdecl)) x86_ISR_77();
void __attribute__((cdecl)) x86_ISR_78();
void __attribute__((cdecl)) x86_ISR_79();
void __attribute__((cdecl)) x86_ISR_80();
void __attribute__((cdecl)) x86_ISR_81();
void __attribute__((cdecl)) x86_ISR_82();
void __attribute__((cdecl)) x86_ISR_83();
void __attribute__((cdecl)) x86_ISR_84();
void __attribute__((cdecl)) x86_ISR_85();
void __attribute__((cdecl)) x86_ISR_86();
void __attribute__((cdecl)) x86_ISR_87();
void __attribute__((cdecl)) x86_ISR_88();
void __attribute__((cdecl)) x86_ISR_89();
void __attribute__((cdecl)) x86_ISR_90();
void __attribute__((cdecl)) x86_ISR_91();
void __attribute__((cdecl)) x86_ISR_92();
void __attribute__((cdecl)) x86_ISR_93();
void __attribute__((cdecl)) x86_ISR_94();
void __attribute__((cdecl)) x86_ISR_95();
void __attribute__((cdecl)) x86_ISR_96();
void __attribute__((cdecl)) x86_ISR_97();
void __attribute__((cdecl)) x86_ISR_98();
void __attribute__((cdecl)) x86_ISR_99();
void __attribute__((cdecl)) x86_ISR_100();
void __attribute__((cdecl)) x86_ISR_101();
void __attribute__((cdecl)) x86_ISR_102();
void __attribute__((cdecl)) x86_ISR_103();
void __attribute__((cdecl)) x86_ISR_104();
void __attribute__((cdecl)) x86_ISR_105();
void __attribute__((cdecl)) x86_ISR_106();
void __attribute__((cdecl)) x86_ISR_107();
void __attribute__((cdecl)) x86_ISR_108();
void __attribute__((cdecl)) x86_ISR_109();
void __attribute__((cdecl)) x86_ISR_110();
void __attribute__((cdecl)) x86_ISR_111();
void __attribute__((cdecl)) x86_ISR_112();
void __attribute__((cdecl)) x86_ISR_113();
void __attribute__((cdecl)) x86_ISR_114();
void __attribute__((cdecl)) x86_ISR_115();
void __attribute__((cdecl)) x86_ISR_116();
void __attribute__((cdecl)) x86_ISR_117();
void __attribute__((cdecl)) x86_ISR_118();
void __attribute__((cdecl)) x86_ISR_119();
void __attribute__((cdecl)) x86_ISR_120();
void __attribute__((cdecl)) x86_ISR_121();
void __attribute__((cdecl)) x86_ISR_122();
void __attribute__((cdecl)) x86_ISR_123();
void __attribute__((cdecl)) x86_ISR_124();
void __attribute__((cdecl)) x86_ISR_125();
void __attribute__((cdecl)) x86_ISR_126();
void __attribute__((cdecl)) x86_ISR_127();
void __attribute__((cdecl)) x86_ISR_128();
void __attribute__((cdecl)) x86_ISR_129();
void __attribute__((cdecl)) x86_ISR_130();
void __attribute__((cdecl)) x86_ISR_131();
void __attribute__((cdecl)) x86_ISR_132();
void __attribute__((cdecl)) x86_ISR_133();
void __attribute__((cdecl)) x86_ISR_134();
void __attribute__((cdecl)) x86_ISR_135();
void __attribute__((cdecl)) x86_ISR_136();
void __attribute__((cdecl)) x86_ISR_137();
void __attribute__((cdecl)) x86_ISR_138();
void __attribute__((cdecl)) x86_ISR_139();
void __attribute__((cdecl)) x86_ISR_140();
void __attribute__((cdecl)) x86_ISR_141();
void __attribute__((cdecl)) x86_ISR_142();
void __attribute__((cdecl)) x86_ISR_143();
void __attribute__((cdecl)) x86_ISR_144();
void __attribute__((cdecl)) x86_ISR_145();
void __attribute__((cdecl)) x86_ISR_146();
void __attribute__((cdecl)) x86_ISR_147();
void __attribute__((cdecl)) x86_ISR_148();
void __attribute__((cdecl)) x86_ISR_149();
void __attribute__((cdecl)) x86_ISR_150();
void __attribute__((cdecl)) x86_ISR_151();
void __attribute__((cdecl)) x86_ISR_152();
void __attribute__((cdecl)) x86_ISR_153();
void __attribute__((cdecl)) x86_ISR_154();
void __attribute__((cdecl)) x86_ISR_155();
void __attribute__((cdecl)) x86_ISR_156();
void __attribute__((cdecl)) x86_ISR_157();
void __attribute__((cdecl)) x86_ISR_158();
void __attribute__((cdecl)) x86_ISR_159();
void __attribute__((cdecl)) x86_ISR_160();
void __attribute__((cdecl)) x86_ISR_161();
void __attribute__((cdecl)) x86_ISR_162();
void __attribute__((cdecl)) x86_ISR_163();
void __attribute__((cdecl)) x86_ISR_164();
void __attribute__((cdecl)) x86_ISR_165();
void __attribute__((cdecl)) x86_ISR_166();
void __attribute__((cdecl)) x86_ISR_167();
void __attribute__((cdecl)) x86_ISR_168();
void __attribute__((cdecl)) x86_ISR_169();
void __attribute__((cdecl)) x86_ISR_170();
void __attribute__((cdecl)) x86_ISR_171();
void __attribute__((cdecl)) x86_ISR_172();
void __attribute__((cdecl)) x86_ISR_173();
void __attribute__((cdecl)) x86_ISR_174();
void __attribute__((cdecl)) x86_ISR_175();
void __attribute__((cdecl)) x86_ISR_176();
void __attribute__((cdecl)) x86_ISR_177();
void __attribute__((cdecl)) x86_ISR_178();
void __attribute__((cdecl)) x86_ISR_179();
void __attribute__((cdecl)) x86_ISR_180();
void __attribute__((cdecl)) x86_ISR_181();
void __attribute__((cdecl)) x86_ISR_182();
void __attribute__((cdecl)) x86_ISR_183();
void __attribute__((cdecl)) x86_ISR_184();
void __attribute__((cdecl)) x86_ISR_185();
void __attribute__((cdecl)) x86_ISR_186();
void __attribute__((cdecl)) x86_ISR_187();
void __attribute__((cdecl)) x86_ISR_188();
void __attribute__((cdecl)) x86_ISR_189();
void __attribute__((cdecl)) x86_ISR_190();
void __attribute__((cdecl)) x86_ISR_191();
void __attribute__((cdecl)) x86_ISR_192();
void __attribute__((cdecl)) x86_ISR_193();
void __attribute__((cdecl)) x86_ISR_194();
void __attribute__((cdecl)) x86_ISR_195();
void __attribute__((cdecl)) x86_ISR_196();
void __attribute__((cdecl)) x86_ISR_197();
void __attribute__((cdecl)) x86_ISR_198();
void __attribute__((cdecl)) x86_ISR_199();
void __attribute__((cdecl)) x86_ISR_200();
void __attribute__((cdecl)) x86_ISR_201();
void __attribute__((cdecl)) x86_ISR_202();
void __attribute__((cdecl)) x86_ISR_203();
void __attribute__((cdecl)) x86_ISR_204();
void __attribute__((cdecl)) x86_ISR_205();
void __attribute__((cdecl)) x86_ISR_206();
void __attribute__((cdecl)) x86_ISR_207();
void __attribute__((cdecl)) x86_ISR_208();
void __attribute__((cdecl)) x86_ISR_209();
void __attribute__((cdecl)) x86_ISR_210();
void __attribute__((cdecl)) x86_ISR_211();
void __attribute__((cdecl)) x86_ISR_212();
void __attribute__((cdecl)) x86_ISR_213();
void __attribute__((cdecl)) x86_ISR_214();
void __attribute__((cdecl)) x86_ISR_215();
void __attribute__((cdecl)) x86_ISR_216();
void __attribute__((cdecl)) x86_ISR_217();
void __attribute__((cdecl)) x86_ISR_218();
void __attribute__((cdecl)) x86_ISR_219();
void __attribute__((cdecl)) x86_ISR_220();
void __attribute__((cdecl)) x86_ISR_221();
void __attribute__((cdecl)) x86_ISR_222();
void __attribute__((cdecl)) x86_ISR_223();
void __attribute__((cdecl)) x86_ISR_224();
void __attribute__((cdecl)) x86_ISR_225();
void __attribute__((cdecl)) x86_ISR_226();
void __attribute__((cdecl)) x86_ISR_227();
void __attribute__((cdecl)) x86_ISR_228();
void __attribute__((cdecl)) x86_ISR_229();
void __attribute__((cdecl)) x86_ISR_230();
void __attribute__((cdecl)) x86_ISR_231();
void __attribute__((cdecl)) x86_ISR_232();
void __attribute__((cdecl)) x86_ISR_233();
void __attribute__((cdecl)) x86_ISR_234();
void __attribute__((cdecl)) x86_ISR_235();
void __attribute__((cdecl)) x86_ISR_236();
void __attribute__((cdecl)) x86_ISR_237();
void __attribute__((cdecl)) x86_ISR_238();
void __attribute__((cdecl)) x86_ISR_239();
void __attribute__((cdecl)) x86_ISR_240();
void __attribute__((cdecl)) x86_ISR_241();
void __attribute__((cdecl)) x86_ISR_242();
void __attribute__((cdecl)) x86_ISR_243();
void __attribute__((cdecl)) x86_ISR_244();
void __attribute__((cdecl)) x86_ISR_245();
void __attribute__((cdecl)) x86_ISR_246();
void __attribute__((cdecl)) x86_ISR_247();
void __attribute__((cdecl)) x86_ISR_248();
void __attribute__((cdecl)) x86_ISR_249();
void __attribute__((cdecl)) x86_ISR_250();
void __attribute__((cdecl)) x86_ISR_251();
void __attribute__((cdecl)) x86_ISR_252();
void __attribute__((cdecl)) x86_ISR_253();
void __attribute__((cdecl)) x86_ISR_254();
void __attribute__((cdecl)) x86_ISR_255();

void x86_ISR_InitializeInterruptHandlers(){
	x86_IDT_SetInterruptHandler(  0, x86_ISR_0  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  1, x86_ISR_1  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  2, x86_ISR_2  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  3, x86_ISR_3  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  4, x86_ISR_4  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  5, x86_ISR_5  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  6, x86_ISR_6  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  7, x86_ISR_7  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  8, x86_ISR_8  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(  9, x86_ISR_9  , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 10, x86_ISR_10 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 11, x86_ISR_11 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 12, x86_ISR_12 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 13, x86_ISR_13 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 14, x86_ISR_14 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 15, x86_ISR_15 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 16, x86_ISR_16 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 17, x86_ISR_17 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 18, x86_ISR_18 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 19, x86_ISR_19 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 20, x86_ISR_20 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 21, x86_ISR_21 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 22, x86_ISR_22 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 23, x86_ISR_23 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 24, x86_ISR_24 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 25, x86_ISR_25 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 26, x86_ISR_26 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 27, x86_ISR_27 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 28, x86_ISR_28 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 29, x86_ISR_29 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 30, x86_ISR_30 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 31, x86_ISR_31 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 32, x86_ISR_32 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 33, x86_ISR_33 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 34, x86_ISR_34 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 35, x86_ISR_35 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 36, x86_ISR_36 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 37, x86_ISR_37 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 38, x86_ISR_38 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 39, x86_ISR_39 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 40, x86_ISR_40 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 41, x86_ISR_41 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 42, x86_ISR_42 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 43, x86_ISR_43 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 44, x86_ISR_44 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 45, x86_ISR_45 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 46, x86_ISR_46 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 47, x86_ISR_47 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 48, x86_ISR_48 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 49, x86_ISR_49 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 50, x86_ISR_50 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 51, x86_ISR_51 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 52, x86_ISR_52 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 53, x86_ISR_53 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 54, x86_ISR_54 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 55, x86_ISR_55 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 56, x86_ISR_56 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 57, x86_ISR_57 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 58, x86_ISR_58 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 59, x86_ISR_59 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 60, x86_ISR_60 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 61, x86_ISR_61 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 62, x86_ISR_62 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 63, x86_ISR_63 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 64, x86_ISR_64 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 65, x86_ISR_65 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 66, x86_ISR_66 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 67, x86_ISR_67 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 68, x86_ISR_68 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 69, x86_ISR_69 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 70, x86_ISR_70 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 71, x86_ISR_71 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 72, x86_ISR_72 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 73, x86_ISR_73 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 74, x86_ISR_74 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 75, x86_ISR_75 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 76, x86_ISR_76 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 77, x86_ISR_77 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 78, x86_ISR_78 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 79, x86_ISR_79 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 80, x86_ISR_80 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 81, x86_ISR_81 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 82, x86_ISR_82 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 83, x86_ISR_83 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 84, x86_ISR_84 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 85, x86_ISR_85 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 86, x86_ISR_86 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 87, x86_ISR_87 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 88, x86_ISR_88 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 89, x86_ISR_89 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 90, x86_ISR_90 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 91, x86_ISR_91 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 92, x86_ISR_92 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 93, x86_ISR_93 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 94, x86_ISR_94 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 95, x86_ISR_95 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 96, x86_ISR_96 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 97, x86_ISR_97 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 98, x86_ISR_98 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler( 99, x86_ISR_99 , GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(100, x86_ISR_100, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(101, x86_ISR_101, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(102, x86_ISR_102, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(103, x86_ISR_103, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(104, x86_ISR_104, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(105, x86_ISR_105, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(106, x86_ISR_106, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(107, x86_ISR_107, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(108, x86_ISR_108, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(109, x86_ISR_109, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(110, x86_ISR_110, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(111, x86_ISR_111, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(112, x86_ISR_112, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(113, x86_ISR_113, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(114, x86_ISR_114, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(115, x86_ISR_115, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(116, x86_ISR_116, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(117, x86_ISR_117, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(118, x86_ISR_118, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(119, x86_ISR_119, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(120, x86_ISR_120, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(121, x86_ISR_121, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(122, x86_ISR_122, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(123, x86_ISR_123, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(124, x86_ISR_124, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(125, x86_ISR_125, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(126, x86_ISR_126, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(127, x86_ISR_127, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(128, x86_ISR_128, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(129, x86_ISR_129, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(130, x86_ISR_130, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(131, x86_ISR_131, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(132, x86_ISR_132, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(133, x86_ISR_133, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(134, x86_ISR_134, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(135, x86_ISR_135, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(136, x86_ISR_136, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(137, x86_ISR_137, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(138, x86_ISR_138, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(139, x86_ISR_139, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(140, x86_ISR_140, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(141, x86_ISR_141, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(142, x86_ISR_142, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(143, x86_ISR_143, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(144, x86_ISR_144, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(145, x86_ISR_145, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(146, x86_ISR_146, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(147, x86_ISR_147, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(148, x86_ISR_148, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(149, x86_ISR_149, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(150, x86_ISR_150, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(151, x86_ISR_151, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(152, x86_ISR_152, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(153, x86_ISR_153, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(154, x86_ISR_154, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(155, x86_ISR_155, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(156, x86_ISR_156, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(157, x86_ISR_157, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(158, x86_ISR_158, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(159, x86_ISR_159, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(160, x86_ISR_160, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(161, x86_ISR_161, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(162, x86_ISR_162, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(163, x86_ISR_163, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(164, x86_ISR_164, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(165, x86_ISR_165, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(166, x86_ISR_166, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(167, x86_ISR_167, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(168, x86_ISR_168, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(169, x86_ISR_169, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(170, x86_ISR_170, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(171, x86_ISR_171, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(172, x86_ISR_172, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(173, x86_ISR_173, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(174, x86_ISR_174, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(175, x86_ISR_175, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(176, x86_ISR_176, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(177, x86_ISR_177, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(178, x86_ISR_178, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(179, x86_ISR_179, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(180, x86_ISR_180, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(181, x86_ISR_181, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(182, x86_ISR_182, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(183, x86_ISR_183, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(184, x86_ISR_184, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(185, x86_ISR_185, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(186, x86_ISR_186, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(187, x86_ISR_187, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(188, x86_ISR_188, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(189, x86_ISR_189, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(190, x86_ISR_190, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(191, x86_ISR_191, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(192, x86_ISR_192, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(193, x86_ISR_193, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(194, x86_ISR_194, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(195, x86_ISR_195, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(196, x86_ISR_196, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(197, x86_ISR_197, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(198, x86_ISR_198, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(199, x86_ISR_199, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(200, x86_ISR_200, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(201, x86_ISR_201, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(202, x86_ISR_202, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(203, x86_ISR_203, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(204, x86_ISR_204, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(205, x86_ISR_205, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(206, x86_ISR_206, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(207, x86_ISR_207, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(208, x86_ISR_208, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(209, x86_ISR_209, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(210, x86_ISR_210, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(211, x86_ISR_211, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(212, x86_ISR_212, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(213, x86_ISR_213, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(214, x86_ISR_214, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(215, x86_ISR_215, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(216, x86_ISR_216, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(217, x86_ISR_217, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(218, x86_ISR_218, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(219, x86_ISR_219, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(220, x86_ISR_220, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(221, x86_ISR_221, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(222, x86_ISR_222, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(223, x86_ISR_223, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(224, x86_ISR_224, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(225, x86_ISR_225, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(226, x86_ISR_226, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(227, x86_ISR_227, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(228, x86_ISR_228, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(229, x86_ISR_229, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(230, x86_ISR_230, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(231, x86_ISR_231, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(232, x86_ISR_232, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(233, x86_ISR_233, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(234, x86_ISR_234, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(235, x86_ISR_235, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(236, x86_ISR_236, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(237, x86_ISR_237, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(238, x86_ISR_238, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(239, x86_ISR_239, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(240, x86_ISR_240, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(241, x86_ISR_241, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(242, x86_ISR_242, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(243, x86_ISR_243, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(244, x86_ISR_244, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(245, x86_ISR_245, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(246, x86_ISR_246, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(247, x86_ISR_247, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(248, x86_ISR_248, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(249, x86_ISR_249, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(250, x86_ISR_250, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(251, x86_ISR_251, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(252, x86_ISR_252, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(253, x86_ISR_253, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(254, x86_ISR_254, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
	x86_IDT_SetInterruptHandler(255, x86_ISR_255, GDT_SEGMENT_KTEXT, IDT_ATTR_GATE_INTERRUPT32|IDT_ATTR_RING0);
}

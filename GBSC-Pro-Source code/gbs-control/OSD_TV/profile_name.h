/*
 * 自定义 OSD 配置文件的名称
 * 自定义 OSD 配置文件的名称
 *
 * Created in 0:49 16.02.2024
 * 作者：亚历山大-卡拉巴诺夫
 * https://www.youtube.com/channel/UCkNk5gQMIu8k9xW-uENsBzw
 *
 *
 *-----------------------------------------------------------------------------------
 *   =       -       .       /       :       '                                       *
 *   ↓       ↓       ↓       ↓       ↓       ↓                                       *
 * 0x3D    0x3E    0x2E    0x2F    0x3A    0x27                                      *     
 *                                                                                   *
 * A  B  C  D  E  F  G  H  I  J  K  L                                                *
 * M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z     输入字符表           *
 * a  b  c  d  e  f g  h  i  j  k  l            要输入的字符表     *
 * m  n  o  p  q  r  s  t  u  v  w  x  y  z                                          *
 * _0_  _1_  _2_  _3_  _4_  _5_  _6_  _7_  _8_  _9_                                  *
 *------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 *                                                                                                                                                                       *
 * x1 x2 x3 x4 x5 x6 x7 x8 x9     输入字符的序列号（输入文本的序列号）                                                            *
 *                                                                                                                                                                       *
 * 字母的最大位数 = 9 (x9)                                                                                                                            *
 * 字母位数上限 = 9 (x9)                                                                                                                         *
 *                                                                                                                                                                       *
 * 最多可输入的配置文件数 (20)                                                                                                                       *
 * 最多可输入的配置文件个数 (20)                                                                                                                          *
 *                                                                                                                                                                       *
 * 如何设置配置文件名称的范例                                                                                                                    *
 * 如何设置配置文件名称的好例子                                                                                                                  *
 *  Sega 2 = x5                                                                                                                                                          *
 * void name_1() {   x1 = S, x2 = e, x3 = g, x4 = a,  x6 = 2_  ; } // name 1 (Sega 2)   неверный ввод!!! incorrect input!!!                                              *
 *                                                                                                                                                                       *
 * Sega.2... = x9                                                                                                                                                        *
 * void name_1() {   x1 = S, x2 = e, x3 = g, x4 = a, x5 = 0x2E, x6 = _2_, x7 = 0x2E, x8 = 0x2E,  x9 = 0x2E   ; } // name 1 (Sega.2...)  правильный ввод! correct input!  *
 *                                                                                                                                                                       *
 *------------------------------------------------------------------------------------------------------------------------------------------------------------------------
 */





//***************************************************profile name*******************************************************************//

// void name_1() {   x1 = S, x2 = e, x3 = g, x4 = a, x5 = 0x2E, x6 = _2_, x7 = 0x2E, x8 = 0x2E,  x9 = 0x2E   ; } // name 1 (Sega.2...)
void name_1() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _1_   ; } // name 2 (profile-2)

void name_2() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _2_   ; } // name 2 (profile-2)

void name_3() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _3_   ; } // name 3 (profile-3)

void name_4() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _4_   ; } // name 4 (profile-4)

void name_5() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _5_   ; } // name 5 (profile-5)

void name_6() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _6_   ; } // name 6 (profile-6)

void name_7() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _7_   ; } // name 7 (profile-7)

void name_8() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _8_   ; } // name 8 (profile-8)

void name_9() {   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = 0x3E,  x9 = _9_   ; } // name 9 (profile-9)

void name_10(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _0_    ; } // name 10 (profile10)

void name_11(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _1_    ; } // name 11 (profile11)

void name_12(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _2_    ; } // name 12 (profile12)

void name_13(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _3_    ; } // name 13 (profile13)

void name_14(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _4_    ; } // name 14 (profile15)

void name_15(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _5_    ; } // name 15 (profile15)

void name_16(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _6_    ; } // name 16 (profile16)
 
void name_17(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _7_    ; } // name 17 (profile17)

void name_18(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _8_    ; } // name 18 (profile18)

void name_19(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _1_,  x9 = _9_    ; } // name 19 (profile19)

void name_20(){   x1 = p, x2 = r, x3 = o, x4 = f, x5 = i, x6 = l, x7 = e, x8 = _2_,  x9 = _0_    ; } // name 20 (profile20)

//*********************************************************************************************************************************//









// the sending line N1
void sending1(){name_1();}
void sending2(){name_2();}
void sending3(){name_3();}
void sending4(){name_4();} 
void sending5(){name_5();} 
void sending6(){name_6();} 
void sending7(){name_7();}
void sending8(){name_8();}
void sending9(){name_9();}
void sending10(){name_10();}
void sending11(){name_11();}
void sending12(){name_12();}
void sending13(){name_13();}
void sending14(){name_14();}
void sending15(){name_15();}
void sending16(){name_16();}
void sending17(){name_17();}
void sending18(){name_18();}
void sending19(){name_19();}
void sending20(){name_20();}
// the sending line N2
void sending1a(){name_1();}
void sending2a(){name_2();}
void sending3a(){name_3();} 
void sending4a(){name_4();} 
void sending5a(){name_5();} 
void sending6a(){name_6();} 
void sending7a(){name_7();} 
void sending8a(){name_8();}
void sending9a(){name_9();}
void sending10a(){name_10();}
void sending11a(){name_11();}
void sending12a(){name_12();}
void sending13a(){name_13();}
void sending14a(){name_14();}
void sending15a(){name_15();}
void sending16a(){name_16();}
void sending17a(){name_17();}
void sending18a(){name_18();}
void sending19a(){name_19();}
void sending20a(){name_20();}
// the sending line N3
void sending1b(){name_1();}
void sending2b(){name_2();}
void sending3b(){name_3();} 
void sending4b(){name_4();} 
void sending5b(){name_5();} 
void sending6b(){name_6();} 
void sending7b(){name_7();} 
void sending8b(){name_8();}
void sending9b(){name_9();}
void sending10b(){name_10();}
void sending11b(){name_11();}
void sending12b(){name_12();}
void sending13b(){name_13();}
void sending14b(){name_14();}
void sending15b(){name_15();}
void sending16b(){name_16();}
void sending17b(){name_17();}
void sending18b(){name_18();}
void sending19b(){name_19();}
void sending20b(){name_20();}
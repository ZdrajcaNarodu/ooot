glabel func_80A4BCA0
/* 00000 80A4BCA0 C4840024 */  lwc1    $f4, 0x0024($a0)           ## 00000024
/* 00004 80A4BCA4 8C82016C */  lw      $v0, 0x016C($a0)           ## 0000016C
/* 00008 80A4BCA8 3C0180A5 */  lui     $at, %hi(D_80A4DEC4)       ## $at = 80A50000
/* 0000C 80A4BCAC 4600218D */  trunc.w.s $f6, $f4                   
/* 00010 80A4BCB0 24420030 */  addiu   $v0, $v0, 0x0030           ## $v0 = 00000030
/* 00014 80A4BCB4 440F3000 */  mfc1    $t7, $f6                   
/* 00018 80A4BCB8 00000000 */  nop
/* 0001C 80A4BCBC A44F0000 */  sh      $t7, 0x0000($v0)           ## 00000030
/* 00020 80A4BCC0 8498001C */  lh      $t8, 0x001C($a0)           ## 0000001C
/* 00024 80A4BCC4 C48A0028 */  lwc1    $f10, 0x0028($a0)          ## 00000028
/* 00028 80A4BCC8 0018CA83 */  sra     $t9, $t8, 10               
/* 0002C 80A4BCCC 33280001 */  andi    $t0, $t9, 0x0001           ## $t0 = 00000000
/* 00030 80A4BCD0 00084880 */  sll     $t1, $t0,  2               
/* 00034 80A4BCD4 00290821 */  addu    $at, $at, $t1              
/* 00038 80A4BCD8 C428DEC4 */  lwc1    $f8, %lo(D_80A4DEC4)($at)  
/* 0003C 80A4BCDC 460A4400 */  add.s   $f16, $f8, $f10            
/* 00040 80A4BCE0 4600848D */  trunc.w.s $f18, $f16                 
/* 00044 80A4BCE4 440B9000 */  mfc1    $t3, $f18                  
/* 00048 80A4BCE8 00000000 */  nop
/* 0004C 80A4BCEC A44B0002 */  sh      $t3, 0x0002($v0)           ## 00000032
/* 00050 80A4BCF0 C484002C */  lwc1    $f4, 0x002C($a0)           ## 0000002C
/* 00054 80A4BCF4 4600218D */  trunc.w.s $f6, $f4                   
/* 00058 80A4BCF8 440D3000 */  mfc1    $t5, $f6                   
/* 0005C 80A4BCFC 03E00008 */  jr      $ra                        
/* 00060 80A4BD00 A44D0004 */  sh      $t5, 0x0004($v0)           ## 00000034


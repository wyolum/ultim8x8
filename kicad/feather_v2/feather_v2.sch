EESchema Schematic File Version 2
LIBS:power
LIBS:feather_v2
LIBS:ultim8x8_sym
LIBS:ultim_bus
LIBS:feather_v2-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "ULTiM8x8 Feather Board"
Date "2016-10-29"
Rev "2.0"
Comp "Maniacal Labs & WyoLum"
Comment1 "Power to panel provided by LiPo battery or USB from Feather (toggle with jumper)"
Comment2 "Used to mount an Adafruit Feather to the back of the Ultim8x8"
Comment3 "www.wyolum.com"
Comment4 "www.maniacallabs.com"
$EndDescr
$Comp
L MTG_1 M2
U 1 1 58151907
P 7600 5775
F 0 "M2" H 7700 5725 40  0000 L CNN
F 1 "Screw" H 7600 5830 30  0001 C CNN
F 2 "feather_v2:Screw_NPTH" H 7600 5775 60  0001 C CNN
F 3 "" H 7600 5775 60  0000 C CNN
F 4 "mfr_pn" H 7600 5775 60  0001 C CNN "manf#"
	1    7600 5775
	-1   0    0    1   
$EndComp
Wire Wire Line
	2400 1275 2750 1275
Wire Wire Line
	3925 1375 4800 1375
Wire Wire Line
	2375 1975 2750 1975
Wire Wire Line
	2375 2075 2750 2075
$Comp
L MTG_1 M1
U 1 1 58152AF0
P 7600 5450
F 0 "M1" H 7700 5400 40  0000 L CNN
F 1 "Screw" H 7600 5505 30  0001 C CNN
F 2 "feather_v2:Screw_NPTH" H 7600 5450 60  0001 C CNN
F 3 "" H 7600 5450 60  0000 C CNN
F 4 "mfr_pn" H 7600 5450 60  0001 C CNN "manf#"
	1    7600 5450
	-1   0    0    1   
$EndComp
NoConn ~ 2750 975 
NoConn ~ 2750 1075
NoConn ~ 2750 1175
NoConn ~ 2750 1375
NoConn ~ 2750 1475
NoConn ~ 2750 1575
NoConn ~ 2750 1675
NoConn ~ 2750 1775
NoConn ~ 2750 1875
NoConn ~ 2750 2175
NoConn ~ 2750 2275
NoConn ~ 2750 2375
NoConn ~ 2750 2475
NoConn ~ 3925 2475
NoConn ~ 3925 2375
NoConn ~ 3925 2275
NoConn ~ 3925 2175
NoConn ~ 3925 2075
NoConn ~ 3925 1975
NoConn ~ 3925 1875
NoConn ~ 3925 1775
NoConn ~ 3925 1675
NoConn ~ 3925 1475
$Comp
L JUMPER3 JP1
U 1 1 5817E55B
P 5125 1475
F 0 "JP1" H 5175 1375 50  0000 L CNN
F 1 "PWR_SEL" H 5125 1575 50  0000 C BNN
F 2 "feather_v2:switch_spdt" H 5125 1475 50  0001 C CNN
F 3 "" H 5125 1475 50  0000 C CNN
	1    5125 1475
	0    -1   1    0   
$EndComp
Wire Wire Line
	3925 1575 4800 1575
$Comp
L Feather_M0 P8
U 1 1 5867FBDF
P 3350 1725
F 0 "P8" H 2950 2575 50  0000 C CNN
F 1 "Feather_M0" V 3050 1725 50  0000 C CNN
F 2 "feather_v2:FEATHER_M0" H 2950 1725 50  0001 C CNN
F 3 "" H 2950 1725 50  0000 C CNN
F 4 "mfr_pn" H 3350 1725 60  0001 C CNN "manf#"
	1    3350 1725
	1    0    0    -1  
$EndComp
$Comp
L CONN4_S P6
U 1 1 5868039D
P 6925 1500
F 0 "P6" V 6875 1500 50  0000 C CNN
F 1 "Input" V 6975 1500 50  0000 C CNN
F 2 "feather_v2:header_2x2pos_th_male" H 6925 1500 60  0001 C CNN
F 3 "" H 6925 1500 60  0000 C CNN
	1    6925 1500
	-1   0    0    1   
$EndComp
$Comp
L CONN4_S P7
U 1 1 586803FA
P 8300 1500
F 0 "P7" V 8250 1500 50  0000 C CNN
F 1 "Output" V 8350 1500 50  0000 C CNN
F 2 "feather_v2:header_2x2pos_th_male" H 8300 1500 60  0001 C CNN
F 3 "" H 8300 1500 60  0000 C CNN
	1    8300 1500
	1    0    0    -1  
$EndComp
Wire Wire Line
	7275 1350 7450 1350
Wire Wire Line
	7450 1450 7275 1450
Wire Wire Line
	7275 1550 7450 1550
Wire Wire Line
	7450 1650 7275 1650
Text Label 5775 1475 2    50   ~ 10
VLED
Text Label 7450 1650 2    50   ~ 10
VLED
Text Label 2400 1275 0    50   ~ 10
GND
Text Label 2375 1975 0    50   ~ 10
SCK
Text Label 2375 2075 0    50   ~ 10
MOSI
Text Label 7450 1450 2    50   ~ 10
GND
Text Label 7450 1550 2    50   ~ 10
SCK
Text Label 7450 1350 2    50   ~ 10
MOSI
Wire Wire Line
	7950 1350 7775 1350
Wire Wire Line
	7775 1450 7950 1450
Wire Wire Line
	7950 1550 7775 1550
Wire Wire Line
	7775 1650 7950 1650
Text Label 7775 1350 0    50   ~ 10
GND
Text Label 7775 1550 0    50   ~ 10
VLED
NoConn ~ 7775 1450
NoConn ~ 7775 1650
Wire Wire Line
	4800 1375 4800 1125
Wire Wire Line
	4800 1125 5125 1125
Wire Wire Line
	5125 1125 5125 1225
Wire Wire Line
	4800 1575 4800 1850
Wire Wire Line
	4800 1850 5125 1850
Wire Wire Line
	5125 1850 5125 1725
Wire Wire Line
	5775 1475 5225 1475
Text Notes 7350 5250 0    50   ~ 0
Mounting Holes
Text Label 4375 1375 0    50   ~ 10
VBAT
Text Label 4350 1575 0    50   ~ 10
VUSB
Wire Wire Line
	4375 2850 4800 2850
$Comp
L JUMPER3 JP2
U 1 1 586BC487
P 5125 2950
F 0 "JP2" H 5175 2850 50  0000 L CNN
F 1 "PWR_SEL2" H 5125 3050 50  0000 C BNN
F 2 "feather_v2:switch_spdt" H 5125 2950 50  0001 C CNN
F 3 "" H 5125 2950 50  0000 C CNN
	1    5125 2950
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4350 3050 4800 3050
Text Label 5775 2950 2    50   ~ 10
VLED
Wire Wire Line
	4800 2850 4800 2600
Wire Wire Line
	4800 2600 5125 2600
Wire Wire Line
	5125 2600 5125 2700
Wire Wire Line
	4800 3050 4800 3325
Wire Wire Line
	4800 3325 5125 3325
Wire Wire Line
	5125 3325 5125 3200
Wire Wire Line
	5775 2950 5225 2950
Text Label 4375 2850 0    50   ~ 10
VBAT
Text Label 4350 3050 0    50   ~ 10
VUSB
$Comp
L LOGO_ML L2
U 1 1 58E27187
P 10050 6225
F 0 "L2" H 10050 5950 40  0000 C CNN
F 1 "LOGO_ML" H 10050 6408 60  0001 C CNN
F 2 "feather_v2:logo_MLlabs_small" H 10050 6225 60  0001 C CNN
F 3 "" H 10050 6225 60  0001 C CNN
F 4 "mfr_pn" H 10050 6225 60  0001 C CNN "manf#"
	1    10050 6225
	1    0    0    -1  
$EndComp
$Comp
L LOGO_WL L1
U 1 1 58E271AD
P 9625 6325
F 0 "L1" H 9625 6150 40  0000 C CNN
F 1 "LOGO_WL" H 9625 6461 60  0001 C CNN
F 2 "feather_v2:logo_wyo_butterfly_small" H 9625 6325 60  0001 C CNN
F 3 "" H 9625 6325 60  0001 C CNN
F 4 "mfr_pn" H 9625 6325 60  0001 C CNN "manf#"
	1    9625 6325
	1    0    0    -1  
$EndComp
$Comp
L OSHW L3
U 1 1 58E271D3
P 10700 6075
F 0 "L3" H 10700 5650 40  0000 C CNN
F 1 "OSHW" H 10700 6175 40  0001 C CNN
F 2 "feather_v2:OSHW_6mm" H 10700 6075 60  0001 C CNN
F 3 "" H 10700 6075 60  0000 C CNN
F 4 "mfr_pn" H 10700 6075 60  0001 C CNN "manf#"
	1    10700 6075
	1    0    0    -1  
$EndComp
$EndSCHEMATC

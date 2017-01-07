EESchema Schematic File Version 2
LIBS:power
LIBS:feather_v2
LIBS:ultim8x8_sym
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
L CONN_1 P3
U 1 1 58151907
P 1125 3150
F 0 "P3" H 1205 3150 40  0000 L CNN
F 1 "Screw" H 1125 3205 30  0001 C CNN
F 2 "feather_v2:Screw_NPTH" H 1125 3150 60  0001 C CNN
F 3 "" H 1125 3150 60  0000 C CNN
F 4 "mfr_pn" H 1125 3150 60  0001 C CNN "manf#"
	1    1125 3150
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
L LOGO_1 P1
U 1 1 581524CC
P 1000 3450
F 0 "P1" H 950 3550 40  0000 L CNN
F 1 "Maniacal Labs" H 1000 3505 30  0001 C CNN
F 2 "feather_v2:logo_MLlabs_small" H 1800 3450 60  0000 C CNN
F 3 "" H 1000 3450 60  0000 C CNN
	1    1000 3450
	1    0    0    -1  
$EndComp
$Comp
L LOGO_1 P10
U 1 1 58152670
P 1000 3675
F 0 "P10" H 950 3775 40  0000 L CNN
F 1 "WyoLum" H 1000 3730 30  0001 C CNN
F 2 "feather_v2:logo_wyo_butterfly_small" H 1950 3675 60  0000 C CNN
F 3 "" H 1000 3675 60  0000 C CNN
	1    1000 3675
	1    0    0    -1  
$EndComp
$Comp
L CONN_1 P2
U 1 1 58152AF0
P 1125 2950
F 0 "P2" H 1205 2950 40  0000 L CNN
F 1 "Screw" H 1125 3005 30  0001 C CNN
F 2 "feather_v2:Screw_NPTH" H 1125 2950 60  0001 C CNN
F 3 "" H 1125 2950 60  0000 C CNN
F 4 "mfr_pn" H 1125 2950 60  0001 C CNN "manf#"
	1    1125 2950
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
NoConn ~ 1275 3150
NoConn ~ 1275 2950
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
Text Notes 1350 3050 0    50   ~ 0
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
$EndSCHEMATC

#ifndef MATRIX_MAP_H
#define MATRIX_MAP_H

const int MatrixWidth = 56;
const int MatrixHeight = 24;
const int NUM_LEDS = MatrixWidth * MatrixHeight;

uint16_t MatrixMap[MatrixHeight][MatrixWidth] = {
{ 447, 432, 431, 416, 415, 400, 399, 384, 383, 368, 367, 352, 351, 336, 335, 320, 319, 304, 303, 288, 287, 272, 271, 256, 255, 240, 239, 224, 223, 208, 207, 192, 191, 176, 175, 160, 159, 144, 143, 128, 127, 112, 111,  96,  95,  80,  79,  64,  63,  48,  47,  32,  31,  16,  15,   0},
{ 446, 433, 430, 417, 414, 401, 398, 385, 382, 369, 366, 353, 350, 337, 334, 321, 318, 305, 302, 289, 286, 273, 270, 257, 254, 241, 238, 225, 222, 209, 206, 193, 190, 177, 174, 161, 158, 145, 142, 129, 126, 113, 110,  97,  94,  81,  78,  65,  62,  49,  46,  33,  30,  17,  14,   1},
{ 445, 434, 429, 418, 413, 402, 397, 386, 381, 370, 365, 354, 349, 338, 333, 322, 317, 306, 301, 290, 285, 274, 269, 258, 253, 242, 237, 226, 221, 210, 205, 194, 189, 178, 173, 162, 157, 146, 141, 130, 125, 114, 109,  98,  93,  82,  77,  66,  61,  50,  45,  34,  29,  18,  13,   2},
{ 444, 435, 428, 419, 412, 403, 396, 387, 380, 371, 364, 355, 348, 339, 332, 323, 316, 307, 300, 291, 284, 275, 268, 259, 252, 243, 236, 227, 220, 211, 204, 195, 188, 179, 172, 163, 156, 147, 140, 131, 124, 115, 108,  99,  92,  83,  76,  67,  60,  51,  44,  35,  28,  19,  12,   3},
{ 443, 436, 427, 420, 411, 404, 395, 388, 379, 372, 363, 356, 347, 340, 331, 324, 315, 308, 299, 292, 283, 276, 267, 260, 251, 244, 235, 228, 219, 212, 203, 196, 187, 180, 171, 164, 155, 148, 139, 132, 123, 116, 107, 100,  91,  84,  75,  68,  59,  52,  43,  36,  27,  20,  11,   4},
{ 442, 437, 426, 421, 410, 405, 394, 389, 378, 373, 362, 357, 346, 341, 330, 325, 314, 309, 298, 293, 282, 277, 266, 261, 250, 245, 234, 229, 218, 213, 202, 197, 186, 181, 170, 165, 154, 149, 138, 133, 122, 117, 106, 101,  90,  85,  74,  69,  58,  53,  42,  37,  26,  21,  10,   5},
{ 441, 438, 425, 422, 409, 406, 393, 390, 377, 374, 361, 358, 345, 342, 329, 326, 313, 310, 297, 294, 281, 278, 265, 262, 249, 246, 233, 230, 217, 214, 201, 198, 185, 182, 169, 166, 153, 150, 137, 134, 121, 118, 105, 102,  89,  86,  73,  70,  57,  54,  41,  38,  25,  22,   9,   6},
{ 440, 439, 424, 423, 408, 407, 392, 391, 376, 375, 360, 359, 344, 343, 328, 327, 312, 311, 296, 295, 280, 279, 264, 263, 248, 247, 232, 231, 216, 215, 200, 199, 184, 183, 168, 167, 152, 151, 136, 135, 120, 119, 104, 103,  88,  87,  72,  71,  56,  55,  40,  39,  24,  23,   8,   7},
{ 455, 456, 471, 472, 487, 488, 503, 504, 519, 520, 535, 536, 551, 552, 567, 568, 583, 584, 599, 600, 615, 616, 631, 632, 647, 648, 663, 664, 679, 680, 695, 696, 711, 712, 727, 728, 743, 744, 759, 760, 775, 776, 791, 792, 807, 808, 823, 824, 839, 840, 855, 856, 871, 872, 887, 888},
{ 454, 457, 470, 473, 486, 489, 502, 505, 518, 521, 534, 537, 550, 553, 566, 569, 582, 585, 598, 601, 614, 617, 630, 633, 646, 649, 662, 665, 678, 681, 694, 697, 710, 713, 726, 729, 742, 745, 758, 761, 774, 777, 790, 793, 806, 809, 822, 825, 838, 841, 854, 857, 870, 873, 886, 889},
{ 453, 458, 469, 474, 485, 490, 501, 506, 517, 522, 533, 538, 549, 554, 565, 570, 581, 586, 597, 602, 613, 618, 629, 634, 645, 650, 661, 666, 677, 682, 693, 698, 709, 714, 725, 730, 741, 746, 757, 762, 773, 778, 789, 794, 805, 810, 821, 826, 837, 842, 853, 858, 869, 874, 885, 890},
{ 452, 459, 468, 475, 484, 491, 500, 507, 516, 523, 532, 539, 548, 555, 564, 571, 580, 587, 596, 603, 612, 619, 628, 635, 644, 651, 660, 667, 676, 683, 692, 699, 708, 715, 724, 731, 740, 747, 756, 763, 772, 779, 788, 795, 804, 811, 820, 827, 836, 843, 852, 859, 868, 875, 884, 891},
{ 451, 460, 467, 476, 483, 492, 499, 508, 515, 524, 531, 540, 547, 556, 563, 572, 579, 588, 595, 604, 611, 620, 627, 636, 643, 652, 659, 668, 675, 684, 691, 700, 707, 716, 723, 732, 739, 748, 755, 764, 771, 780, 787, 796, 803, 812, 819, 828, 835, 844, 851, 860, 867, 876, 883, 892},
{ 450, 461, 466, 477, 482, 493, 498, 509, 514, 525, 530, 541, 546, 557, 562, 573, 578, 589, 594, 605, 610, 621, 626, 637, 642, 653, 658, 669, 674, 685, 690, 701, 706, 717, 722, 733, 738, 749, 754, 765, 770, 781, 786, 797, 802, 813, 818, 829, 834, 845, 850, 861, 866, 877, 882, 893},
{ 449, 462, 465, 478, 481, 494, 497, 510, 513, 526, 529, 542, 545, 558, 561, 574, 577, 590, 593, 606, 609, 622, 625, 638, 641, 654, 657, 670, 673, 686, 689, 702, 705, 718, 721, 734, 737, 750, 753, 766, 769, 782, 785, 798, 801, 814, 817, 830, 833, 846, 849, 862, 865, 878, 881, 894},
{ 448, 463, 464, 479, 480, 495, 496, 511, 512, 527, 528, 543, 544, 559, 560, 575, 576, 591, 592, 607, 608, 623, 624, 639, 640, 655, 656, 671, 672, 687, 688, 703, 704, 719, 720, 735, 736, 751, 752, 767, 768, 783, 784, 799, 800, 815, 816, 831, 832, 847, 848, 863, 864, 879, 880, 895},
{1343,1328,1327,1312,1311,1296,1295,1280,1279,1264,1263,1248,1247,1232,1231,1216,1215,1200,1199,1184,1183,1168,1167,1152,1151,1136,1135,1120,1119,1104,1103,1088,1087,1072,1071,1056,1055,1040,1039,1024,1023,1008,1007, 992, 991, 976, 975, 960, 959, 944, 943, 928, 927, 912, 911, 896},
{1342,1329,1326,1313,1310,1297,1294,1281,1278,1265,1262,1249,1246,1233,1230,1217,1214,1201,1198,1185,1182,1169,1166,1153,1150,1137,1134,1121,1118,1105,1102,1089,1086,1073,1070,1057,1054,1041,1038,1025,1022,1009,1006, 993, 990, 977, 974, 961, 958, 945, 942, 929, 926, 913, 910, 897},
{1341,1330,1325,1314,1309,1298,1293,1282,1277,1266,1261,1250,1245,1234,1229,1218,1213,1202,1197,1186,1181,1170,1165,1154,1149,1138,1133,1122,1117,1106,1101,1090,1085,1074,1069,1058,1053,1042,1037,1026,1021,1010,1005, 994, 989, 978, 973, 962, 957, 946, 941, 930, 925, 914, 909, 898},
{1340,1331,1324,1315,1308,1299,1292,1283,1276,1267,1260,1251,1244,1235,1228,1219,1212,1203,1196,1187,1180,1171,1164,1155,1148,1139,1132,1123,1116,1107,1100,1091,1084,1075,1068,1059,1052,1043,1036,1027,1020,1011,1004, 995, 988, 979, 972, 963, 956, 947, 940, 931, 924, 915, 908, 899},
{1339,1332,1323,1316,1307,1300,1291,1284,1275,1268,1259,1252,1243,1236,1227,1220,1211,1204,1195,1188,1179,1172,1163,1156,1147,1140,1131,1124,1115,1108,1099,1092,1083,1076,1067,1060,1051,1044,1035,1028,1019,1012,1003, 996, 987, 980, 971, 964, 955, 948, 939, 932, 923, 916, 907, 900},
{1338,1333,1322,1317,1306,1301,1290,1285,1274,1269,1258,1253,1242,1237,1226,1221,1210,1205,1194,1189,1178,1173,1162,1157,1146,1141,1130,1125,1114,1109,1098,1093,1082,1077,1066,1061,1050,1045,1034,1029,1018,1013,1002, 997, 986, 981, 970, 965, 954, 949, 938, 933, 922, 917, 906, 901},
{1337,1334,1321,1318,1305,1302,1289,1286,1273,1270,1257,1254,1241,1238,1225,1222,1209,1206,1193,1190,1177,1174,1161,1158,1145,1142,1129,1126,1113,1110,1097,1094,1081,1078,1065,1062,1049,1046,1033,1030,1017,1014,1001, 998, 985, 982, 969, 966, 953, 950, 937, 934, 921, 918, 905, 902},
{1336,1335,1320,1319,1304,1303,1288,1287,1272,1271,1256,1255,1240,1239,1224,1223,1208,1207,1192,1191,1176,1175,1160,1159,1144,1143,1128,1127,1112,1111,1096,1095,1080,1079,1064,1063,1048,1047,1032,1031,1016,1015,1000, 999, 984, 983, 968, 967, 952, 951, 936, 935, 920, 919, 904, 903},
};

#endif
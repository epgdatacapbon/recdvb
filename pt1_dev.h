/* -*- tab-width: 4; indent-tabs-mode: nil -*- */
#ifndef _PT1_DEV_H_
#define _PT1_DEV_H_


#ifndef FULLAUTO_SEARCH
#define FULLAUTO_SEARCH 1		// 0にする事で以下のDVBデバイス番号テーブルを利用したDVB自動選択に処理を変更できる
#endif

#if FULLAUTO_SEARCH == 0
// DVBデバイス番号テーブル
// 環境により各々違うと思われるので各自で適切に変更すること
//
// DVBデバイス番号を変更する場合は、/etc/modrpobe.d/options-dvb.confを以下の1行を参考に編集してください。
// options earth_pt1 adapter_nr=0,1,2,3			// 衛星0 地上0 衛星1 地上1の順に指定

// BS/CS
int bsdev[] = {
    0,
    2,
    4,
    6,
    8,
    10,
    12,
    14
};
// VHF/UHF/CATV
int isdb_t_dev[] = {
    1,
    3,
    5,
    7,
    9,
    11,
    13,
    15
};

#define NUM_BSDEV				(sizeof(bsdev)/sizeof(int))
#define NUM_ISDB_T_DEV			(sizeof(isdb_t_dev)/sizeof(int))
#endif


// 変換テーブル

ISDB_T_FREQ_CONV_TABLE    isdb_t_conv_table[] = {
    {   0, CHTYPE_SATELLITE, 0, 0x4010, "151"},  /* 151ch：BS朝日 */
    {   0, CHTYPE_SATELLITE, 1, 0x4011, "161"},  /* 161ch：BS-TBS */
    {   0, CHTYPE_SATELLITE, 2, 0x4012, "171"},  /* 171ch：BSテレ東 */
    {   1, CHTYPE_SATELLITE, 0, 0x4030, "191"},  /* 191ch：WOWOWプライム */
    {   1, CHTYPE_SATELLITE, 1, 0x4031, "103"},  /* 103ch：NHK BSプレミアム*/
    {   1, CHTYPE_SATELLITE, 2, 0x4632, "256"},  /* 256ch：ディズニーチャンネル */
    {   2, CHTYPE_SATELLITE, 0, 0x4450, "192"},  /* 192ch：WOWOWライブ */
    {   2, CHTYPE_SATELLITE, 1, 0x4451, "193"},  /* 193ch：WOWOWシネマ */
    {   4, CHTYPE_SATELLITE, 0, 0x4090, "211"},  /* 211ch：BS11 */
    {   4, CHTYPE_SATELLITE, 1, 0x4091, "200"},  /* 200ch：スターチャンネル1 */
    {   4, CHTYPE_SATELLITE, 2, 0x4092, "222"},  /* 222ch：TwellV */
    {   5, CHTYPE_SATELLITE, 0, 0x46b1, "241"},  /* 241ch：BSスカパー！ */
    {   5, CHTYPE_SATELLITE, 1, 0x46b2, "231"},  /* 231ch：BSキャンパスex */
    {   5, CHTYPE_SATELLITE, 1, 0x46b2, "232"},  /* 232ch：BSキャンパスon */
    {   5, CHTYPE_SATELLITE, 1, 0x46b2, "531"},  /* 531ch：放送大学ラジオ */
    {   6, CHTYPE_SATELLITE, 0, 0x40d0, "141"},  /* 141ch：BS日テレ */
    {   6, CHTYPE_SATELLITE, 1, 0x40d1, "181"},  /* 181ch：BSフジ */
    {   6, CHTYPE_SATELLITE, 2, 0x46d2, "236"},  /* 236ch：ANIMAX */
    {   7, CHTYPE_SATELLITE, 0, 0x40f1, "101"},  /* 101ch：NHK BS1 */
    {   7, CHTYPE_SATELLITE, 1, 0x40f2, "201"},  /* 201ch：スターチャンネル2 */
    {   7, CHTYPE_SATELLITE, 1, 0x40f2, "202"},  /* 202ch：スターチャンネル3 */
    {   9, CHTYPE_SATELLITE, 0, 0x4730, "245"},  /* 245ch：J SPORTS 4 */
    {   9, CHTYPE_SATELLITE, 1, 0x4731, "242"},  /* 242ch：J SPORTS 1 */
    {   9, CHTYPE_SATELLITE, 2, 0x4732, "243"},  /* 243ch：J SPORTS 2 */
    {   9, CHTYPE_SATELLITE, 3, 0x4751, "244"},  /* 244ch：J SPORTS 3 */
    {  10, CHTYPE_SATELLITE, 0, 0x4750, "252"},  /* 252ch：WOWOWプラス */
    {  10, CHTYPE_SATELLITE, 1, 0x4752, "234"},  /* 234ch：グリーンチャンネル */
    {  11, CHTYPE_SATELLITE, 0, 0x4770, "251"},  /* 251ch：BS釣りビジョン */
    {  11, CHTYPE_SATELLITE, 1, 0x4771, "255"},  /* 255ch：日本映画専門チャンネル */
    {  12, CHTYPE_SATELLITE, 0, 0x6020, "CS2"},  /* ND2 */
    {  13, CHTYPE_SATELLITE, 0, 0x7040, "CS4"},  /* ND4 */
    {  14, CHTYPE_SATELLITE, 0, 0x7060, "CS6"},  /* ND6 */
    {  15, CHTYPE_SATELLITE, 0, 0x6080, "CS8"},  /* ND8 */
    {  16, CHTYPE_SATELLITE, 0, 0x60a0, "CS10"}, /* ND10 */
    {  17, CHTYPE_SATELLITE, 0, 0x70c0, "CS12"}, /* ND12 */
    {  18, CHTYPE_SATELLITE, 0, 0x70e0, "CS14"}, /* ND14 */
    {  19, CHTYPE_SATELLITE, 0, 0x7100, "CS16"}, /* ND16 */
    {  20, CHTYPE_SATELLITE, 0, 0x7120, "CS18"}, /* ND18 */
    {  21, CHTYPE_SATELLITE, 0, 0x7140, "CS20"}, /* ND20 */
    {  22, CHTYPE_SATELLITE, 0, 0x7160, "CS22"}, /* ND22 */
    {  23, CHTYPE_SATELLITE, 0, 0x7180, "CS24"}, /* ND24 */
    {   0, CHTYPE_GROUND, 0, 0,   "1"}, {   1, CHTYPE_GROUND, 0, 0,   "2"},
    {   2, CHTYPE_GROUND, 0, 0,   "3"}, {   3, CHTYPE_GROUND, 0, 0, "C13"},
    {   4, CHTYPE_GROUND, 0, 0, "C14"}, {   5, CHTYPE_GROUND, 0, 0, "C15"},
    {   6, CHTYPE_GROUND, 0, 0, "C16"}, {   7, CHTYPE_GROUND, 0, 0, "C17"},
    {   8, CHTYPE_GROUND, 0, 0, "C18"}, {   9, CHTYPE_GROUND, 0, 0, "C19"},
    {  10, CHTYPE_GROUND, 0, 0, "C20"}, {  11, CHTYPE_GROUND, 0, 0, "C21"},
    {  12, CHTYPE_GROUND, 0, 0, "C22"}, {  13, CHTYPE_GROUND, 0, 0,   "4"},
    {  14, CHTYPE_GROUND, 0, 0,   "5"}, {  15, CHTYPE_GROUND, 0, 0,   "6"},
    {  16, CHTYPE_GROUND, 0, 0,   "7"}, {  17, CHTYPE_GROUND, 0, 0,   "8"},
    {  18, CHTYPE_GROUND, 0, 0,   "9"}, {  19, CHTYPE_GROUND, 0, 0,  "10"},
    {  20, CHTYPE_GROUND, 0, 0,  "11"}, {  21, CHTYPE_GROUND, 0, 0,  "12"},
    {  22, CHTYPE_GROUND, 0, 0, "C23"}, {  23, CHTYPE_GROUND, 0, 0, "C24"},
    {  24, CHTYPE_GROUND, 0, 0, "C25"}, {  25, CHTYPE_GROUND, 0, 0, "C26"},
    {  26, CHTYPE_GROUND, 0, 0, "C27"}, {  27, CHTYPE_GROUND, 0, 0, "C28"},
    {  28, CHTYPE_GROUND, 0, 0, "C29"}, {  29, CHTYPE_GROUND, 0, 0, "C30"},
    {  30, CHTYPE_GROUND, 0, 0, "C31"}, {  31, CHTYPE_GROUND, 0, 0, "C32"},
    {  32, CHTYPE_GROUND, 0, 0, "C33"}, {  33, CHTYPE_GROUND, 0, 0, "C34"},
    {  34, CHTYPE_GROUND, 0, 0, "C35"}, {  35, CHTYPE_GROUND, 0, 0, "C36"},
    {  36, CHTYPE_GROUND, 0, 0, "C37"}, {  37, CHTYPE_GROUND, 0, 0, "C38"},
    {  38, CHTYPE_GROUND, 0, 0, "C39"}, {  39, CHTYPE_GROUND, 0, 0, "C40"},
    {  40, CHTYPE_GROUND, 0, 0, "C41"}, {  41, CHTYPE_GROUND, 0, 0, "C42"},
    {  42, CHTYPE_GROUND, 0, 0, "C43"}, {  43, CHTYPE_GROUND, 0, 0, "C44"},
    {  44, CHTYPE_GROUND, 0, 0, "C45"}, {  45, CHTYPE_GROUND, 0, 0, "C46"},
    {  46, CHTYPE_GROUND, 0, 0, "C47"}, {  47, CHTYPE_GROUND, 0, 0, "C48"},
    {  48, CHTYPE_GROUND, 0, 0, "C49"}, {  49, CHTYPE_GROUND, 0, 0, "C50"},
    {  50, CHTYPE_GROUND, 0, 0, "C51"}, {  51, CHTYPE_GROUND, 0, 0, "C52"},
    {  52, CHTYPE_GROUND, 0, 0, "C53"}, {  53, CHTYPE_GROUND, 0, 0, "C54"},
    {  54, CHTYPE_GROUND, 0, 0, "C55"}, {  55, CHTYPE_GROUND, 0, 0, "C56"},
    {  56, CHTYPE_GROUND, 0, 0, "C57"}, {  57, CHTYPE_GROUND, 0, 0, "C58"},
    {  58, CHTYPE_GROUND, 0, 0, "C59"}, {  59, CHTYPE_GROUND, 0, 0, "C60"},
    {  60, CHTYPE_GROUND, 0, 0, "C61"}, {  61, CHTYPE_GROUND, 0, 0, "C62"},
    {  62, CHTYPE_GROUND, 0, 0, "C63"}, {  63, CHTYPE_GROUND, 0, 0,  "13"},
    {  64, CHTYPE_GROUND, 0, 0,  "14"}, {  65, CHTYPE_GROUND, 0, 0,  "15"},
    {  66, CHTYPE_GROUND, 0, 0,  "16"}, {  67, CHTYPE_GROUND, 0, 0,  "17"},
    {  68, CHTYPE_GROUND, 0, 0,  "18"}, {  69, CHTYPE_GROUND, 0, 0,  "19"},
    {  70, CHTYPE_GROUND, 0, 0,  "20"}, {  71, CHTYPE_GROUND, 0, 0,  "21"},
    {  72, CHTYPE_GROUND, 0, 0,  "22"}, {  73, CHTYPE_GROUND, 0, 0,  "23"},
    {  74, CHTYPE_GROUND, 0, 0,  "24"}, {  75, CHTYPE_GROUND, 0, 0,  "25"},
    {  76, CHTYPE_GROUND, 0, 0,  "26"}, {  77, CHTYPE_GROUND, 0, 0,  "27"},
    {  78, CHTYPE_GROUND, 0, 0,  "28"}, {  79, CHTYPE_GROUND, 0, 0,  "29"},
    {  80, CHTYPE_GROUND, 0, 0,  "30"}, {  81, CHTYPE_GROUND, 0, 0,  "31"},
    {  82, CHTYPE_GROUND, 0, 0,  "32"}, {  83, CHTYPE_GROUND, 0, 0,  "33"},
    {  84, CHTYPE_GROUND, 0, 0,  "34"}, {  85, CHTYPE_GROUND, 0, 0,  "35"},
    {  86, CHTYPE_GROUND, 0, 0,  "36"}, {  87, CHTYPE_GROUND, 0, 0,  "37"},
    {  88, CHTYPE_GROUND, 0, 0,  "38"}, {  89, CHTYPE_GROUND, 0, 0,  "39"},
    {  90, CHTYPE_GROUND, 0, 0,  "40"}, {  91, CHTYPE_GROUND, 0, 0,  "41"},
    {  92, CHTYPE_GROUND, 0, 0,  "42"}, {  93, CHTYPE_GROUND, 0, 0,  "43"},
    {  94, CHTYPE_GROUND, 0, 0,  "44"}, {  95, CHTYPE_GROUND, 0, 0,  "45"},
    {  96, CHTYPE_GROUND, 0, 0,  "46"}, {  97, CHTYPE_GROUND, 0, 0,  "47"},
    {  98, CHTYPE_GROUND, 0, 0,  "48"}, {  99, CHTYPE_GROUND, 0, 0,  "49"},
    { 100, CHTYPE_GROUND, 0, 0,  "50"}, { 101, CHTYPE_GROUND, 0, 0,  "51"},
    { 102, CHTYPE_GROUND, 0, 0,  "52"}, { 103, CHTYPE_GROUND, 0, 0,  "53"},
    { 104, CHTYPE_GROUND, 0, 0,  "54"}, { 105, CHTYPE_GROUND, 0, 0,  "55"},
    { 106, CHTYPE_GROUND, 0, 0,  "56"}, { 107, CHTYPE_GROUND, 0, 0,  "57"},
    { 108, CHTYPE_GROUND, 0, 0,  "58"}, { 109, CHTYPE_GROUND, 0, 0,  "59"},
    { 110, CHTYPE_GROUND, 0, 0,  "60"}, { 111, CHTYPE_GROUND, 0, 0,  "61"},
    { 112, CHTYPE_GROUND, 0, 0,  "62"},
    { 0, 0, 0, 0, NULL} /* terminate */
};
#endif

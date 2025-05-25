#pragma once
#include "node.hpp"
#include "constants.hpp"

// グローバル変数
extern node SN[N+1];  // BSとSN集合

extern int dst2[N+1][N+1];      // BS,SN間の距離２乗
extern double Esnd[N+1][N+1];   // 送信エネルギー
extern int dst2BSorder[N];      // BSからの距離順

extern int Round;  // ラウンド数
extern int FDN;    // 最初のノード枯渇のラウンド
extern int sFDN;   // その総和
extern int LDN;    // 最後のノード枯渇のラウンド
extern int sLDN;   // その総和
extern int Anum;   // 活ノード数

extern double UsedEn[Rmax];     // ラウンド毎の消費エネルギー
extern double sumUsedEn;        // 消費エネルギーの総和 
#include "../include/types.hpp"

// グローバル変数の定義
node SN[N + 1]; // BSとSN集合

int dst2[N + 1][N + 1];    // BS,SN間の距離２乗
double Esnd[N + 1][N + 1]; // 送信エネルギー
int dst2BSorder[N];        // BSからの距離順

int Round; // ラウンド数
int FDN;   // 最初のノード枯渇のラウンド
int sFDN;  // その総和
int LDN;   // 最後のノード枯渇のラウンド
int sLDN;  // その総和
int Anum;  // 活ノード数

double UsedEn[Rmax]; // ラウンド毎の消費エネルギー
double sumUsedEn;    // 消費エネルギーの総和

std::vector<std::vector<double>>
    En_R(M, std::vector<double>(N)); // ラウンド毎の消費エネルギー
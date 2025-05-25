#pragma once
#include <cmath>

#define Loopi0N for (int i = 0; i <= N; ++i)
#define Loopi1N for (int i = 1; i <= N; ++i)
#define Loopj1N for (int j = 1; j <= N; ++j)

/// ネットワークパラメータ
const int a = 50;    // 正方エリアの一辺{50,100} ●Conf.ver●
const int BSy = 150; // BSのy座標
const int N = 100;    // ノード総数
const double e0 = .5; // 初期エネルギー Table2 {0.2, 0.5, 1.0}
const int K = 2000;   // データ(msg)サイズ(bit)

const double p = 0.03;    // 〇CH割合(1.0は直接転送)
const double q = 1.0 / p; // p>0.5が問題

const double Eelec = 50 * (1e-9);       // 回路エネルギー[J/bit]●エリア次第
const double Eda = 5 * (1e-9);         // signalの融合コスト[J/msg]
const double Efs = 100 * (1e-12);      // 送信エネルギー（自由空間）[J/m^2]
const double Emp = 0.0013 * (1e-12);   // 送信エネルギー（多重経路）[J/m^4]
const double dc = pow(Efs / Emp, 0.5); // 送信エネルギーの境界距離[m]

const int Rmax = 3000; // magic number
const int S = 1;       // S個のサンプル，S=1デバック用
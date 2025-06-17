#include "../include/functions.hpp"
#include "../include/types.hpp"
#include <iostream>

using namespace std;

// 直接転送の消費エネルギー(ラウンド単位)
void CalcDtEn_R() {
  Loopi1N // 各SNからBSへ直接
  {
    if (SN[i].En <= 0)
      continue;                           // 枯渇skip
    SN[i].En -= K * (Eelec + Esnd[i][0]); // iの送信コスト
  }
}

// 直接転送の直接計算
void CalcDirectDtEn() {
  double Emax = Esnd[1][0], Emin = Esnd[1][0]; // 初期化
  Loopi1N {
    if (Emax < Esnd[i][0])
      Emax = Esnd[i][0]; // 最大値更新
    if (Emin > Esnd[i][0])
      Emin = Esnd[i][0]; // 最小値更新
  }

  cout << " FDN = " << (int)(e0 / K / (Eelec + Emax)) << ", ";
  sFDN += (int)(e0 / K / (Eelec + Emax)); // FDNへの加算

  cout << " LDN = " << (int)(e0 / K / (Eelec + Emin)) << endl;
  sLDN += (int)(e0 / K / (Eelec + Emin)); // LDNへの加算

  Loopi1N sumUsedEn += K * (Eelec + Esnd[i][0]); // ラウンド単位
}

void CalcEn() {
  SN[0].msg = 0; // リセット
  Loopi1N {
    if (SN[i].En > 0)
      SN[i].msg = 1;
    else
      SN[i].msg = 0; // 枯渇
  }

  // CM → CH
  Loopi1N {
    if (SN[i].CH)
      continue; // 非CHのみ
    if (SN[i].En <= 0)
      continue; // 枯渇skip

    int CH = SN[i].nh; // iのCH

    SN[i].En -= K * (Eelec + Esnd[i][CH]); // CHへの送信コスト
    En_R[Round-1][i] += K * (Eelec + Esnd[i][CH]); // ラウンド毎の消費エネルギー

    if (SN[i].En < 0)
      continue; // 枯渇送信取消

    SN[CH].En -= (Eelec * K); // CHの受信コスト
    En_R[Round-1][CH] += (Eelec * K); // ラウンド毎の消費エネルギー
    if (SN[CH].En < 0)
      continue; // 枯渇受信取消

    SN[CH].msg += SN[i].msg; // 上乗せ
  }

  // CH → BS
  Loopi1N {
    // CHのみ
    if (SN[i].CH) {
      SN[i].En -= Eda * SN[i].msg * K; // iの融合コスト
      En_R[Round][i] += Eda * SN[i].msg * K; // ラウンド毎の消費エネルギー
      if (SN[i].En < 0)
        continue; // 枯渇skip

      SN[i].En -= K * (Eelec + Esnd[i][0]); // BSへの送信コスト
      En_R[Round][i] += K * (Eelec + Esnd[i][0]); // ラウンド毎の消費エネルギー
      if (SN[i].En < 0)
        continue; // 枯渇skip

      SN[0].msg += SN[i].msg; // 上乗せ
    }
  }
}

// 活ノード総数とFDN・LDN算出
void SetAnum() {
  Anum = N; // 初期設定
  Loopi1N if (SN[i].En <= 0)-- Anum;

  if (Anum < N && FDN < 0)
    FDN = Round;
  if (Anum <= 0)
    LDN = Round;
}

// ラウンド毎の消費エネルギー
void SaveDtEn_R() {
  double tmp = 0.0;        // ローカル変数
  Loopi1N tmp += SN[i].En; // 現時点での残存エネルギー
  tmp = N * e0 - tmp;      // それまでの消費エネルギーの総和

  UsedEn[Round] += tmp - sumUsedEn; // ラウンド毎の記録
  sumUsedEn = tmp;                  // 消費エネルギーの総和の更新
}

// FDNまでのラウンド平均消費エネルギー
double En_FDN() {
  double tmp = 0.0;        // ローカル変数
  Loopi1N tmp += SN[i].En; // 現時点での残存エネルギー
  tmp = N * e0 - tmp;      // それまでの消費エネルギーの総和

  return tmp / Round; // サンプルとしての平均
}
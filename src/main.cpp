// LEACH00 Conf.ver Fig.5 追試済
// Lindsey02,02Cも同様に送信エネルギー２乗(自由空間)のみ
// LEACH00_6 では ●frame数に注意　論文の見直しを行う．
// p= 1.0設定で直接転送にも対応

#include "../include/functions.hpp"
#include "../include/types.hpp"
#include <iostream>
#include <random>
#include <string>

using namespace std;

random_device rnd;
mt19937 mt(rnd()); // メルセンヌツイスター

/**
 * @brief 設定パラメータの確認
 */
void ViewInits() {
  if (Efs > 10 * (1e-12)) {
    cout << "Conf.ver." << endl;
  } else {
    cout << "Journal ver." << endl;
  }

  cout << "----- 設定パラメータ -----" << endl;
  cout << "a = " << a << " : " << "Coner = (" << a << "," << a << ")" << endl;

  cout << "SN = " << N << ", ";
  cout << "BS.y = " << SN[0].y << ", ";
  cout << "K = " << K << endl;

  cout << "p = " << 100 * p << "[%]" << endl;
}

/**
 * @brief 初期設定
 */
void SetInits() {
  // サンプル単位
  Round = 0;
  FDN = -1;
  LDN = -1;
  Anum = N;

  // BSの初期化
  SN[0].En = 1e5;
  SN[0].msg = 0;

  // 続いてSNの初期化
  Loopi1N {
    SN[i].En = e0; // 初期エネルギー
    SN[i].nh = -1;
    SN[i].CH = false; // 非CH
    SN[i].nr = 0;     // CH候補
  }
}

/**
 * @brief CH割合が大きい場合のCH選択
 */
void RndMoreCHs() {
  Loopi1N {
    SN[i].CH = false; // 前提
    if (SN[i].En <= 0)
      continue; // 次

    int r = 1 + mt() % 100; //[1,100]

    if (100 * p >= r)
      SN[i].CH = true; // CH
  }
}

/**
 * @brief ランダムCH選択
 */
void Rnd_CHs() {
  if (p > 0.5) {
    // CH割合が大きい場合
    RndMoreCHs();
    return;
  }

  bool none = true; // CH決定済
  // それ以外，一つでも決定するまで
  while (none) {
    Loopi1N {
      if (SN[i].En <= 0) {
        SN[i].CH = false;
        continue;
      } // 次

      int r = 1 + mt() % 100; // 乱数[1,100]

      int rq = Round % ((int)q); //

      double Tn = 100.0 * p / (1.0 - p * rq); // ●CHになる閾値

      if (r <= (int)Tn && SN[i].nr >= 0) {
        // 非CH期間終了
        SN[i].CH = true;       // CH設定
        SN[i].nr = 1 - (int)q; // 非CH期間スタート
        none = false;          // 決定済CHアリ
      } else {
        // 非CHの場合
        SN[i].CH = false; // 非CH設定
        ++SN[i].nr;       // 非CH期間のインクリメント
      }
    }
  }
}

/**
 * @brief メイン関数
 */
int main() {
  ResetData(); // データリセット

  SN[0].x = a / 2;
  SN[0].y = BSy; // 正方形エリア
  ViewInits();   // 設定パラメータの確認

  cout << "シミュレーションを開始します" << endl;

  // S個サンプル
  for (int s = 0; s < S; s++) {
    SetInits(); // サンプル毎の初期化

    string D = "Samples/S" + to_string(a) + "_" + to_string(N); // 正方形エリア
    string DD = D + "/Data" + to_string(s) + ".txt";

    GetPosData(DD);   // 座標の読込
    SaveCoordinate(); // 座標の保存

    Calcdst2Esnd(); // SN間の距離２乗・送信コスト算出

    // 直接転送でのFDN・LDN直接算出
    if (p >= 1.0) {
      CalcDirectDtEn(); // 直接算出
      continue;
    }

    // LDN算出まで
    while (LDN < 0) {
      ++Round; // ラウンド数インクリメント

      if (p >= 1.0) {
        // 例外的・直接転送
        CalcDtEn_R(); // ラウンド毎のエネルギー算出
        SaveDtEn_R(); // とその記録
        SetAnum();    // 活ノード数算出
        continue;
      }

      Rnd_CHs(); // 乱数によるCH選択
      SetNh();   // Next hop設定

      CalcEn();  // 消費エネルギー算出
      SetAnum(); // 活ノード数算出
      SaveNh();  // Next hopの座標を保存
    }

    sumUsedEn += En_FDN(); // FDNまでのラウンド平均消費エネルギー

    sFDN += FDN; // 加算
    sLDN += LDN; // 加算
  }

  cout << "----- 結果 -----" << endl;
  cout << sumUsedEn / S << endl;
  SaveData((int)(100 * p),
           sumUsedEn / S); // ラウンド単位の消費エネのサンプル平均

  cout << "Average:" << endl;
  cout << " FDN = " << sFDN / S << endl; // FDN画面出力
  cout << " LDN = " << sLDN / S << endl; // LDN画面出力

  return 0;
}
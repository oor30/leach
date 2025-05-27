// LEACH00 Conf.ver Fig.5 追試済
// Lindsey02,02Cも同様に送信エネルギー２乗(自由空間)のみ
// LEACH00_6 では ●frame数に注意　論文の見直しを行う．
// p= 1.0設定で直接転送にも対応

#include "../include/functions.hpp"
#include "../include/types.hpp"
#include <algorithm>
#include <gurobi_c++.h> // Gurobi C++ API
#include <iostream>
#include <random>
#include <string>
#include <vector>

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
  int FDNs[S];
  int LDNs[S];
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

      if (Round == M) solve();

      Rnd_CHs(); // 乱数によるCH選択
      SetNh();   // Next hop設定

      CalcEn();  // 消費エネルギー算出
      SetAnum(); // 活ノード数算出
      SaveNh();  // Next hopの座標を保存
    }

    sumUsedEn += En_FDN(); // FDNまでのラウンド平均消費エネルギー

    sFDN += FDN;   // 加算
    FDNs[s] = FDN; // FDNの保存
    sLDN += LDN;   // 加算
    LDNs[s] = LDN; // LDNの保存
  }

  cout << "----- 結果 -----" << endl;
  cout << sumUsedEn / S << endl;
  SaveData((int)(100 * p),
           sumUsedEn / S); // ラウンド単位の消費エネのサンプル平均

  cout << "Average:" << endl;
  cout << " FDN = " << sFDN / S << endl; // FDN画面出力
  cout << " LDN = " << sLDN / S << endl; // LDN画面出力

  cout << "Min Max:" << endl;
  cout << " FDN = " << *min_element(begin(FDNs), end(FDNs)) << ", "
       << *max_element(begin(FDNs), end(FDNs)) << endl; // FDNの最小・最大
  cout << " LDN = " << *min_element(begin(LDNs), end(LDNs)) << ", "
       << *max_element(begin(LDNs), end(LDNs)) << endl; // LDNの最小・最大

  return 0;
}

void solve() {
  // ここに問題を解くためのコードを追加できます。
  // 例えば、特定のアルゴリズムを実行したり、データを処理したりすることができます。
  // 現在は空の関数として定義されています。
  // この関数は、メイン関数から呼び出されることを想定しています。
  // 制約式： XR*Eij <= ej
  // XRは R ラウンドで収集した全域木を用いる回数であり,整数変数である.
  // M は全域木を収集するラウンド数
  // Eijはラウンド i において j 番の SＮが消費したエネルギー
  // ejはＭラウンド終了時点での j 番の SＮの残余エネルギーを示す.
  try {
    cout << "solve関数が呼び出されました。" << endl;
    double ej[N];                            // 各SNの残存エネルギー
    for (int i = 0; i < N; ++i) {
      ej[i] = SN[i].En;
    }
    std::vector<std::vector<double>> Eij = En_R; // ラウンド毎の消費エネルギー

    GRBEnv env = GRBEnv();          // Gurobi環境の初期化
    GRBModel model = GRBModel(env); // モデルの作成
    model.set(GRB_StringAttr_ModelName, "EnergyOptimization"); // モデル名の設定
    GRBVar XR[N];                                              // 各SNの選択変数
    for (int i = 1; i <= N; ++i) {
      XR[i - 1] = model.addVar(0.0, M, 0.0, GRB_INTEGER,
                              "Xn_" + to_string(i)); // バイナリ変数
    }
    model.update(); // モデルの更新
    // 制約式の追加
    for (int j = 0; j < N; ++j) {
      GRBLinExpr expr = 0.0; // 線形式の初期化
      for (int i = 0; i < M; ++i) {
        expr += Eij[i][j] * XR[i]; // 各SNの消費エネルギーを加算
      }
      model.addConstr(expr <= ej[j], "EnergyConstraint_" + to_string(j + 1)); // 制約の追加
    }
    model.set(GRB_IntAttr_ModelSense, GRB_MAXIMIZE); // 最大化問題として設定
    model.optimize(); // 問題の最適化
    if (model.get(GRB_IntAttr_Status) == GRB_OPTIMAL) {
      cout << "最適解が見つかりました。" << endl;
      for (int i = 0; i < N; ++i) {
        cout << "SN " << i + 1 << ": " << XR[i].get(GRB_DoubleAttr_X) << endl; // 各SNの選択変数の値を出力
      }
    } else {
      cout << "最適解は見つかりませんでした。" << endl;
    }
    model.write("model.lp"); // モデルの内容をファイルに書き出し
    cout << "モデルの内容を model.lp に書き出しました。" << endl;
  } catch (GRBException& e) {
    cout << "Gurobiエラー: " << e.getMessage() << endl; // エラーメッセージの出力
  } catch (std::exception& e) {
    cout << "例外: " << e.what() << endl; // その他の例外の出力
  } catch (...) {
    cout << "不明なエラーが発生しました。" << endl; // 不明なエラーの出力
  }
}
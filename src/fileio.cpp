#include "../include/functions.hpp"
#include "../include/types.hpp"
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

/////////////////////////ファイル入力
void GetPosData(string s) {
  int xy = -1;     // ノードの位置情報
  ifstream fin(s); // 対象のデータ
  Loopi1N {
    fin >> xy; // 読込
    if (xy < 0)
      break;          // 番兵
    SN[i].x = xy / a; // x座標
    SN[i].y = xy % a; // y座標
  }
  fin.close(); // ファイルクローズ
  if (xy == -1) {
    cout << "No sample" << endl;
    exit(1);
  }
}

/////////////////////////////////ファイル出力
// データリセット
void ResetData() {
  ofstream out;
  out.open("out/csv/Result.csv"); // 結果のリセット
  out.close();
  out.open("out/csv/parent.csv"); // 親ノードのリセット
  Loopi1N {
    out << i; // ノード番号
    if (i < N)
      out << ","; // 最後の要素以外はカンマ区切り
    else
      out << endl; // 最後の要素は改行
  }
  out.close();

  for (int i = 0; i < Rmax; ++i)
    UsedEn[i] = 0.0; // 初期化
}

// データ保存 int型
void SaveData(int i) {
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加
  out << i << endl;
  out.close();
}

// double型
void SaveData(double d) {
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加
  out << d << endl;
  out.close();
}

// int, double型
void SaveData(int i, double d) {
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加
  out << i << "," << d << endl;
  out.close();
}

// ラウンド毎の消費エネルギーの記録
void SaveRoundData() {
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加

  for (int r = 1; r < Rmax; ++r) {
    cout << r << "," << UsedEn[r] << endl;
  }
}

// ノードの座標
void SaveCoordinate() {
  ofstream out;
  out.open("out/csv/coordinates.csv");

  out << "x,y" << endl; // ヘッダー行

  Loopi0N { out << SN[i].x << "," << SN[i].y << endl; }
  out.close();
}

// Next hopの座標
void SaveNh() {
  ofstream out;
  out.open("out/csv/parent.csv", ios::app); // 追加

  Loopi1N {
    out << SN[i].nh;
    if (i < N)
      out << ","; // 最後の要素以外はカンマ区切り
    else
      out << endl; // 最後の要素は改行
  }
  out.close();
}
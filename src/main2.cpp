// LEACH00 Conf.ver 送信エネルギー２乗(自由空間)のみ
// p= 1.0 設定で直接送信に対応

#include <fstream>
#include <iostream>
#include <random>

using namespace std;

random_device rnd;
mt19937 mt(rnd()); // メルセンヌツイスター

#define Loopi0N for (int i = 0; i <= N; ++i)
#define Loopi1N for (int i = 1; i <= N; ++i)
#define Loopj1N for (int j = 1; j <= N; ++j)

const int K = 2000; // データ(msg)サイズ(bit) ●Conf.ver●
const int a = 100;  // エリアの一辺{50,100} Tang12
const int N = 100;  // ノード総数

const int xm = a;      // BSのｘ座標の最小値
const int xM = 4 * a;  // 　同　最大値
const int Bx = a / 10; // 幅

const double p = 0.2; // CH割合 Fig.10-2
// const double p = 1.0; //●直接送信 Fig.10-1
double q = 1.0 / p; // int型にキャストして使用

const double e0 = .5; // 初期エネルギー Table2 {0.25, 0.5, 1.0}

/// //////////////////////

struct node {
  int x, y, nh, nr, msg;
  double En;
  bool CH;
};
//(int型)座標，Next hop，データ量，非CH期間
//(double型)残余エネ， (bool型)CHかどうか

node SN[N + 1]; // BSとSN集合

int dst2[N + 1][N + 1];    // BS,SN間の距離２乗
double Esnd[N + 1][N + 1]; // 送信エネルギー

int Round = -1; // ラウンド数: 0 スタート
int FDN = -1;   // 最初のノード枯渇のラウンド
int LDN = -1;   // 最後のノード枯渇のラウンド

const double Eelec = 50 * (1e-9); // 回路コスト

const double Efs = 100 * (1e-12); // 送信エネルギー ●Conf.ver●
const double Eda = 5 * (1e-9);    // signalの融合コスト[J/msg]

const int Rmax = 3000; // magic number
double UsedEn[Rmax];   // ラウンド毎の消費エネルギー

int Anum = N; // 活ノード数

double AveE = 0.0; // 消費エネルギーの(WSN全体ラウンド毎)

/////////////////////////ファイル入力

void SetGridModel() // 格子型
{
  Loopi1N {
    SN[i].x = (a / 10) * ((i - 1) / 10); // x座標
    SN[i].y = (a / 10) * ((i - 1) % 10); // y座標
  }
}

/////////////////////////////////ファイル出力
void ResetResults() // 結果のリセット
{
  ofstream out;
  out.open("out/csv/Results.csv");
  out.close();
}

void SaveResults(double d1, int i1, int i2, double d2) {
  ofstream out("out/csv/Results.csv", ios::app);

  out << d1 << ", " << i1 << ", " << i2 << ", " << d2 << endl;

  out.close();
}

// 初期データ
void ViewInits() // 設定パラメータの確認
{
  if (Efs > 10 * (1e-12))
    cout << "Conf.ver." << endl << endl;
  else
    cout << "Journal ver.";

  cout << "a = " << a << " : " << "Coner = (" << a << "," << a << ")" << endl;

  cout << "BS.x = " << SN[0].x << ", ";

  if (p >= 1.0)
    cout << "DT: p = 1.0" << endl << endl;
  else
    cout << "LEACH: p = " << 100 * p << "[%]" << endl;
}

void SetInits() // サンプル毎の初期設定
{
  Round = -1;
  FDN = LDN = -1;
  Anum = N;

  // BSの初期化(エネルギー，y座標)
  SN[0].En = 1e5;
  SN[0].msg = 0;
  SN[0].y = a / 2;

  // 続いてSNの初期化
  Loopi1N {
    SN[i].En = e0; // 初期エネルギー
    SN[i].nr = 0;  // CH候補
  }
}

void Calcdst2Esnd() // ノード間の距離２乗と送信エネルギー
{
  Loopi0N // BSとSN
  {
    dst2[i][i] = 0; // 対角成分
    Esnd[i][i] = 0.0;
    for (int j = i + 1; j <= N; ++j) // 上三角成分のみ
    {
      int x = SN[i].x - SN[j].x;               // ｘ座標の差分
      int y = SN[i].y - SN[j].y;               // ｙ座標の差分
      dst2[i][j] = dst2[j][i] = x * x + y * y; // ２乗

      Esnd[i][j] = Efs * dst2[i][j]; // ●自由空間のみ●
      Esnd[j][i] = Esnd[i][j];       // 対称分
    }
  }
}

void CalcDtEn() // Dtの直接計算 steady state skip
{
  double Emax = Esnd[1][0], Emin = Esnd[1][0]; // 初期化
  AveE = 0.0;
  Loopi1N {
    AveE += K * (Eelec + Esnd[i][0]); // 加算

    if (Emax < Esnd[i][0])
      Emax = Esnd[i][0]; // 最大値更新
    if (Emin > Esnd[i][0])
      Emin = Esnd[i][0]; // 最小値更新
  }

  FDN = (int)(e0 / K / (Eelec + Emax));
  LDN = (int)(e0 / K / (Eelec + Emin));

  SaveResults(SN[0].x, FDN, LDN, AveE); // 結果の記録
}

void CalcLEACH_En() // 各SNでの消費エネルギー
{
  SN[0].msg = 0; // リセット
  Loopi1N if (SN[i].En > 0) SN[i].msg = 1;
  else SN[i].msg = 0; // 枯渇

  Loopi1N // CM → CH
  {
    if (SN[i].CH)
      continue; // 非CHのみ
    if (SN[i].En <= 0)
      continue; // 枯渇skip

    int CH = SN[i].nh; // iのCH

    SN[i].En -= K * (Eelec + Esnd[i][CH]); // CHへの送信コスト
    if (SN[i].En < 0)
      continue; // 枯渇送信取消

    SN[CH].En -= (Eelec * K); // CHの受信コスト
    if (SN[CH].En < 0)
      continue; // 枯渇受信取消

    SN[CH].msg += SN[i].msg; // 上乗せ
  }

  Loopi1N // CH → BS
  {
    if (SN[i].CH) // CHのみ
    {
      SN[i].En -= Eda * SN[i].msg * K; // 融合コスト
      if (SN[i].En < 0)
        continue; // 枯渇skip

      SN[i].En -= K * (Eelec + Esnd[i][0]); // BSへの送信コスト
      if (SN[i].En < 0)
        continue; // 枯渇skip

      SN[0].msg += SN[i].msg; // 上乗せ
    }
  }
}

void SetAnum() // 活ノード総数とFDN・LDN算出
{
  Anum = N; // 初期設定
  Loopi1N if (SN[i].En <= 0)-- Anum;

  if (Anum < N && FDN < 0)
    FDN = Round;
  if (Anum <= 0)
    LDN = Round;
}

double GetAveE() // 平均消費エネルギー
{
  double sum = 0.0;        // ローカル変数
  Loopi1N sum += SN[i].En; // 現時点での残存エネルギー

  return (N * e0 - sum) / (1 + Round); // 初期値 Round = -1 による
}

/////////////////////////////////////Cluster形成

int ShortestPathRelay(int ii) // 最も近くのCH
{
  if (SN[ii].CH)
    return ii; // CHの場合

  int min = (int)1e6, jm = -1; // 最小値と該当CH
  Loopj1N                      // 呼出側　変数i使用
  {
    if (SN[j].En <= 0)
      continue; // 活ノードのみ
    if (!SN[j].CH)
      continue; // CHのみ
    if (ii == j)
      continue; // 自分以外
    if (min > dst2[ii][j]) {
      min = dst2[ii][j];
      jm = j;
    } // 最小値更新
  }
  return jm; // CH返し
}

void SetNh() // Next Hop設定
{
  Loopi1N {
    SN[i].nh = -1; // デフォルト
    if (SN[i].En > 0)
      SN[i].nh = ShortestPathRelay(i);
  }
}

void RndMoreCHs() // CH割合が大きい場合
{
  Loopi1N {
    SN[i].CH = false; // デフォルト
    if (SN[i].En <= 0)
      continue; // 枯渇skip

    int r = 1 + mt() % 100; // 乱数[1,100]

    if (100 * p >= r)
      SN[i].CH = true; // CHに
  }
}

void Rnd_CHs() /// ランダムCH選択
{
  if (p > 0.5) { // CH割合大(非CH期間設定なし)
    RndMoreCHs();
    return;
  }

  bool none = true; // CH決定済
  while (none)      // 一つでも決定するまで
  {
    Loopi1N {
      SN[i].CH = false; // 非CH前提
      if (SN[i].En <= 0)
        continue; // 枯渇skip

      int r = 1 + mt() % 100; // 乱数[1,100]

      int r_p = Round % ((int)q); // 閾値の分母

      double Tn = 100.0 * p / (1.0 - p * r_p); // CHになる閾値

      if (r <= (int)Tn && SN[i].nr >= 0) // 非CH期間終了
      {
        SN[i].CH = true;       // CH決定
        SN[i].nr = 1 - (int)q; // 非CH期間スタート
        none = false;          // whileループ抜け
      } else
        ++SN[i].nr; // 非CH期間のインクリメント
    }
  }
}

int main() {
  ResetResults(); // データリセット Figs.4,8-10コメントアウト

  SetGridModel(); // 格子モデル

  for (int bx = xm; bx <= xM; bx += Bx) // 様々な位置のBS
  {
    SN[0].x = bx; // BSのx座標
    SetInits();   // 初期化

    ViewInits(); // 設定データの確認

    Calcdst2Esnd(); // SN間の距離２乗・送信コスト算出

    if (p >= 1.0) // DTでのFDN・LDN直接算出 Fig.9,10
    {
      CalcDtEn(); // 直接算出
      continue;   // 次のBS
    }

    while (FDN < 0) // FDN算出まで Fig.4,8-10
    {
      ++Round; // ラウンド0からスタート

      Rnd_CHs(); // 乱数によるCH選択
      SetNh();   // Next hop設定

      CalcLEACH_En(); // 消費エネルギー算出
      SetAnum();      // 活ノード数算出
    }
    AveE = GetAveE();                         // ラウンド平均消費エネ
    SaveResults(p, SN[0].x, Round + 1, AveE); // 結果の記録
  }
  return 0;
}
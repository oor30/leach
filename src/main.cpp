// LEACH00 Conf.ver Fig.5 追試済
// Lindsey02,02Cも同様に送信エネルギー２乗(自由空間)のみ
// LEACH00_6 では ●frame数に注意　論文の見直しを行う．
// p= 1.0設定で直接転送にも対応

#include <fstream>
#include <iostream>
#include <random>
#include <string>

using namespace std;

random_device rnd;
mt19937 mt(rnd()); // メルセンヌツイスター

#define Loopi0N for (int i = 0; i <= N; ++i)
#define Loopi1N for (int i = 1; i <= N; ++i)
#define Loopj1N for (int j = 1; j <= N; ++j)

/// //////////////////////
int K = 2000;      // データ(msg)サイズ(bit) ●Conf.ver●
const int a = 100; // 正方エリアの一辺{50,100} ●Conf.ver●
const int N = 100; // ノード総数

const double p = 0.05; // 〇CH割合(1.0は直接転送)
double q = 1.0 / p;    // p>0.5が問題

const int BSy = -100; //

const double e0 = .5; // 初期エネルギー Table2 {0.2, 0.5, 1.0}

/// //////////////////////

struct node {
  int nr;
  bool CH;
  int x, y, nh, msg;
  double En;
};
//(int型)座標，Next hop，データ量，非CH期間
//(double型)残余エネ， (bool型)CHかどうか

node SN[N + 1]; // BSとSN集合

int dst2[N + 1][N + 1];    // BS,SN間の距離２乗
double Esnd[N + 1][N + 1]; // 送信エネルギー

int Round = 0; // ラウンド数
int FDN = -1;  // 最初のノード枯渇のラウンド
int sFDN = 0;  // その総和
int LDN = -1;  // 最後のノード枯渇のラウンド
int sLDN = 0;  // その総和
int Anum = N;  // 活ノード数

const double Eelec = a * (1e-9);  // 回路エネ[J/bit]●エリア次第
const double Efs = 100 * (1e-12); // 送信エネルギー ●Conf.ver●
const double Eda = 5 * (1e-9);    // signalの融合コスト[J/msg]

const int Rmax = 3000; // magic number
double UsedEn[Rmax];   // ラウンド毎の消費エネルギー

double sumUsedEn = 0.0; // 消費エネルギーの総和

const int S = 1; // S個のサンプル，S=1デバック用

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
void ResetData() // データリセット
{
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

void SaveData(int i) // データ保存 int型
{
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加
  out << i << endl;
  out.close();
}

void SaveData(double d) // double型
{
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加
  out << d << endl;
  out.close();
}

void SaveData(int i, double d) // int, double型
{
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加
  out << i << "," << d << endl;
  out.close();
}

void SaveRoundData() // ラウンド毎の消費エネルギーの記録
{
  ofstream out;
  out.open("out/csv/Result.csv", ios::app); // 追加

  for (int r = 1; r < Rmax; ++r) {
    cout << r << "," << UsedEn[r] << endl;
  }
}

void ViewInits() // 設定パラメータの確認
{
  if (Efs > 10 * (1e-12))
    cout << "Conf.ver." << endl << endl;
  else
    cout << "Journal ver.";

  cout << "a = " << a << " : " << "Coner = (" << a << "," << a << ")" << endl;

  cout << "＃SN = " << N << ", ";
  cout << "BS.y = " << SN[0].y << ", ";
  cout << "K = " << K << endl << endl;

  cout << "p = " << 100 * p << "[%]" << endl;
}

void SetInits() // 初期設定
{
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

void CalcDtEn_R() // 直接転送の消費エネルギー(ラウンド単位)
{
  Loopi1N // 各SNからBSへ直接
  {
    if (SN[i].En <= 0)
      continue;                           // 枯渇skip
    SN[i].En -= K * (Eelec + Esnd[i][0]); // iの送信コスト
  }
}

void CalcDirectDtEn() // 直接転送の直接計算
{
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

void CalcEn() // 各SNでの消費エネルギー
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
      SN[i].En -= Eda * SN[i].msg * K; // iの融合コスト
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

void SaveDtEn_R() // ラウンド毎の消費エネ
{
  double tmp = 0.0;        // ローカル変数
  Loopi1N tmp += SN[i].En; // 現時点での残存エネルギー
  tmp = N * e0 - tmp;      // それまでの消費エネルギーの総和

  UsedEn[Round] += tmp - sumUsedEn; // ラウンド毎の記録
  sumUsedEn = tmp;                  // 消費エネルギーの総和の更新
}

double En_FDN() // FDNまでのラウンド平均消費エネルギー
{
  double tmp = 0.0;        // ローカル変数
  Loopi1N tmp += SN[i].En; // 現時点での残存エネルギー
  tmp = N * e0 - tmp;      // それまでの消費エネルギーの総和

  return tmp / Round; // サンプルとしての平均
}

/////////////////////////////////////Cluster形成

int ShortestPathRelay(int ii) // 最も近くのCH
{
  if (SN[ii].CH)
    return ii; // CH自身

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
    if (SN[i].En <= 0) {
      SN[i].nh = -1;
      continue;
    }                                // 次
    SN[i].nh = ShortestPathRelay(i); // CH・非CHのいずれも
  }
}

void RndMoreCHs() // CH割合が大きい場合
{
  Loopi1N {
    SN[i].CH = false; // 前提
    if (SN[i].En <= 0)
      continue; // 次

    int r = 1 + mt() % 100; //[1,100]

    if (100 * p >= r)
      SN[i].CH = true; // CH
  }
}

void Rnd_CHs() /// ランダムCH選択
{
  if (p > 0.5) { // CH割合が大きい場合
    RndMoreCHs();
    return;
  }

  bool none = true; // CH決定済
  while (none)      // それ以外，一つでも決定するまで
  {
    Loopi1N {
      if (SN[i].En <= 0) {
        SN[i].CH = false;
        continue;
      } // 次

      int r = 1 + mt() % 100; // 乱数[1,100]

      int rq = Round % ((int)q); //

      double Tn = 100.0 * p / (1.0 - p * rq); // ●CHになる閾値

      if (r <= (int)Tn && SN[i].nr >= 0) // 非CH期間終了
      {
        SN[i].CH = true;       // CH設定
        SN[i].nr = 1 - (int)q; // 非CH期間スタート
        none = false;          // 決定済CHアリ
      } else                   // 非CHの場合
      {
        SN[i].CH = false; // 非CH設定
        ++SN[i].nr;       // 非CH期間のインクリメント
      }
    }
  }
}

void SaveCoordinate() // ノードの座標
{
  ofstream out;
  out.open("out/csv/coordinates.csv");

  out << "x,y" << endl; // ヘッダー行

  Loopi0N { out << SN[i].x << "," << SN[i].y << endl; }
  out.close();
}

void SaveNh() // Next hopの座標
{
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

int main() {
  ResetData(); // データリセット

  SN[0].x = a / 2;
  SN[0].y = BSy; // 正方形エリア
  ViewInits();   // 設定データの確認

  for (int s = 0; s < S; s++) // S個サンプル
  {
    SetInits(); // サンプル毎の初期化

    string D = "Samples/S" + to_string(a) + "_" + to_string(N); // 正方形エリア
    string DD = D + "/Data" + to_string(s) + ".txt";

    GetPosData(DD);   // 座標の読込
    SaveCoordinate(); // 座標の保存

    Calcdst2Esnd(); // SN間の距離２乗・送信コスト算出

    if (p >= 1.0) // 直接転送でのFDN・LDN直接算出
    {
      CalcDirectDtEn(); // 直接算出
      continue;
    }

    while (FDN < 0) // FDN算出まで
    //		while( LDN < 0 ) //LDN算出まで
    {
      ++Round; // ラウンド数インクリメント

      if (p >= 1.0) // 例外的・直接転送
      {
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

    sumUsedEn += En_FDN(); // FDNまでのラウンド平均消費エネ

    sFDN += Round; // 加算
    //		cout << "FDN = " << Round << endl;
    sLDN += Round; // 加算
    //		cout << "LDN = " << Round << endl;

  } // 次のs

  cout << sumUsedEn / S << endl;
  SaveData((int)(100 * p),
           sumUsedEn / S); // ラウンド単位の消費エネのサンプル平均

  cout << endl << "Average:";
  cout << endl << " FDN = " << sFDN / S; // FDN画面出力
  // cout << " LDN = " << sLDN / S;// LDN画面出力
  cout << endl;

  return 0;
}
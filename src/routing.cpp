#include "../include/types.hpp"

using namespace std;

// ノード間の距離２乗と送信エネルギー
void Calcdst2Esnd() {
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

// 最短経路のRelayを選択
int ShortestPathRelay(int ii) {
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

// Next Hop設定
void SetNh() {
  Loopi1N {
    if (SN[i].En <= 0) {
      SN[i].nh = -1;
      continue;
    } // 次
    SN[i].nh = ShortestPathRelay(i); // CH・非CHのいずれも
  }
}

// ブランチの適正数を計算
int Calchopt() {
  // BSまでの平均距離
  double d2bs = 0.0;
  Loopi1N {
    if (SN[i].En <= 0)
      continue; // 枯渇skip
    d2bs += sqrt(dst2[0][i]);
  }
  d2bs /= Anum;
  if (d2bs < dc) {
    // 自由空間
    d2bs = Efs * pow(d2bs, 2);
  } else {
    // 多重経路
    d2bs = Emp * pow(d2bs, 4);
  }

  // ブランチの適正数を計算
  double devidend = N * Efs * a * a;
  double divisor = 3 * (d2bs - Eelec - Eda - ((Efs * a * a) / 6));
  return round(pow(devidend / divisor, 1 / 3));
}
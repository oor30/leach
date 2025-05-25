#pragma once

/// ノード構造体
struct node {
  int nr;            // 非CH期間
  bool CH;           // CHかどうか
  int x, y, nh, msg; // 座標，Next hop，データ量，非CH期間
  double En;         // 残余エネルギー
};
#pragma once
#include <string>

// ファイル入出力関連
void GetPosData(std::string fileName);
void SaveCoordinate();
void SaveRoundData();
void SaveData(int round);
void SaveData(double value);
void SaveData(int round, double value);
void SaveNh();
void ResetData();

// エネルギー計算関連
void CalcEn();
void CalcDtEn_R();
double En_FDN();
void SetAnum();
void CalcDirectDtEn();
void SaveDtEn_R();

// ルーティング関連
void Calcdst2Esnd();
int Calchopt();
void FormRoutingTree(int hopt, double Eth);
void SetNh();
double CalcThreshold();
int ShortestPathRelay(int i);

// その他
void ViewInits();
void SetInits();
void solve();
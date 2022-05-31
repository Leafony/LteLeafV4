/*
 * typesV4.h
 * LTE-M Leaf V4 library
 *
 * 2021.12.28 kt-nakamura@kddi-tech.com
 */
#ifndef LPWA_TYPES_V4_H_
#define LPWA_TYPES_V4_H_

/**
 * モデムとネットワークの状態
 */
enum NetworkStatus {
  /** 内部エラー */
  LPWA_FAIL,
  /** 初期状態 */
  IDLE,
  /** LPWAモジュールの準備完了 */
  LPWA_READY,
  /** GPRS接続の準備完了 */
  GPRS_READY,
  /** LPWAモジュールの電源オフ */
  LPWA_OFF
};

#endif

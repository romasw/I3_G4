#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include <assert.h>
#include <complex.h>
#include <math.h>

/* 標本(整数)を複素数へ変換 */
void sample_to_complex(unsigned char * s, 
		       complex double * X, 
		       long n) {
  long i;
  for (i = 0; i < n; i++) X[i] = s[i];
}

/* 複素数を標本(整数)へ変換. 虚数部分は無視 */
void complex_to_sample(complex double * X, 
		       unsigned char * s, 
		       long n) {
  long i;
  for (i = 0; i < n; i++) {
    s[i] = creal(X[i]);
  }
}

/* 高速(逆)フーリエ変換;
   w は1のn乗根.
   フーリエ変換の場合   偏角 -2 pi / n
   逆フーリエ変換の場合 偏角  2 pi / n
   xが入力でyが出力.
   xも破壊される
 */
void fft_r(complex double * x, 
	   complex double * y, 
	   long n, 
	   complex double w) {
  if (n == 1) { y[0] = x[0]; }
  else {
    complex double W = 1.0; 
    long i;
    for (i = 0; i < n/2; i++) {
      y[i]     =     (x[i] + x[i+n/2]); /* 偶数行 */
      y[i+n/2] = W * (x[i] - x[i+n/2]); /* 奇数行 */
      W *= w;
    }
    fft_r(y,     x,     n/2, w * w);
    fft_r(y+n/2, x+n/2, n/2, w * w);
    for (i = 0; i < n/2; i++) {
      y[2*i]   = x[i];
      y[2*i+1] = x[i+n/2];
    }
  }
}

void fft(complex double * x, 
	 complex double * y, 
	 long n) {
  long i;
  double arg = 2.0 * M_PI / n;
  complex double w = cos(arg) - 1.0j * sin(arg);
  fft_r(x, y, n, w);
  for (i = 0; i < n; i++) y[i] /= n;
}

void ifft(complex double * y, 
	  complex double * x, 
	  long n) {
  double arg = 2.0 * M_PI / n;
  complex double w = cos(arg) + 1.0j * sin(arg);
  fft_r(y, x, n, w);
}

void band_shift(unsigned char *input_buf, unsigned char * output_buf, long n, int shift) { 
//FFT等のすべてを組み込んだ関数になっている。入力バッファと出力バッファ、音階シフトを引数としてとる。
//shiftは音を高くするか低くするかのフラグ。1なら上げる。0なら下げる
  int min = 200;
  int max = 3000;
  double fs = 44100;
  double freq_bin = fs / n;

  complex double * X = calloc(sizeof(complex double), n);
  complex double * Y = calloc(sizeof(complex double), n);
  //入力された音声を複素数に変換
  sample_to_complex(input_buf, X, n);
  /* FFT -> Y */
  fft(X, Y, n);

  // 周波数フィルタリング
  for (long i = 0; i < n; ++i) {
    double freq = i * freq_bin;
    if (freq < min || freq > max) {
      Y[i] = 0; // フィルタ範囲外の成分をゼロにする
    }
  }

  //ピッチシフト
  if (shift != 0) {
    //新しい周波数成分を保持するための配列
    complex double * shifted_Y = calloc(n, sizeof(complex double));

    if (shift > 0) {
      // ピッチを上げる (周波数を高くする)
      for (long i = 0; i < n; ++i) {
        long new_index = (i + shift) % n;
        if (new_index < n) {
          shifted_Y[new_index] = Y[i];
        }
      }
    } else {
      // ピッチを下げる (周波数を低くする)
      shift = -shift;
      for (long i = 0; i < n; ++i) {
        long new_index = (i - shift + n) % n;
        if (new_index >= 0) {
          shifted_Y[new_index] = Y[i];
        }
      }
    }

    // 元の配列に戻す
    for (long i = 0; i < n; ++i) {
      Y[i] = shifted_Y[i];
    }
    // 一時的な配列を解放
    free(shifted_Y);
  }
  ifft(Y, X, n);
  /* 標本の配列に変換 */
  complex_to_sample(X, output_buf, n);
  free(X);
  free(Y);
}
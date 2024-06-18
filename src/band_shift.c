#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

void sample_to_complex(unsigned char *s, complex double *X, long n) {
    long i;
    for (i = 0; i < n; i++) X[i] = s[i];
}

void complex_to_sample(complex double *X, unsigned char *s, long n) {
    long i;
    for (i = 0; i < n; i++) {
        s[i] = creal(X[i]);
    }
}

void fft_r(complex double *x, complex double *y, long n, complex double w) {
    if (n == 1) { 
        y[0] = x[0]; 
    } else {
        complex double W = 1.0; 
        long i;
        for (i = 0; i < n/2; i++) {
            y[i]     =     (x[i] + x[i+n/2]); /* 偶数行 */
            y[i+n/2] = W * (x[i] - x[i+n/2]); /* 奇数行 */
            W *= w;
        }
        fft_r(y, x, n/2, w * w);
        fft_r(y+n/2, x+n/2, n/2, w * w);
        for (i = 0; i < n/2; i++) {
            y[2*i]   = x[i];
            y[2*i+1] = x[i+n/2];
        }
    }
}

void fft(complex double *x, complex double *y, long n) {
    long i;
    double arg = 2.0 * M_PI / n;
    complex double w = cos(arg) - 1.0j * sin(arg);
    fft_r(x, y, n, w);
    for (i = 0; i < n; i++) y[i] /= n;
}

void ifft(complex double *y, complex double *x, long n) {
    double arg = 2.0 * M_PI / n;
    complex double w = cos(arg) + 1.0j * sin(arg);
    fft_r(y, x, n, w);
}

void band_shift(unsigned char *input_buf, unsigned char *output_buf, long n, int shift) {
    int min = 200;
    int max = 3000;
    double fs = 44100;
    double freq_bin = fs / n;

    // メモリ確保
    complex double *X = calloc(n, sizeof(complex double));
    complex double *Y = calloc(n, sizeof(complex double));
    if (X == NULL || Y == NULL) {
        perror("calloc failed");
        free(X);
        free(Y);
        exit(EXIT_FAILURE);
    }

    // 入力を複素数に変換
    sample_to_complex(input_buf, X, n);
    // FFT
    fft(X, Y, n);

    // 周波数フィルタリング
    for (long i = 0; i < n; ++i) {
        double freq = i * freq_bin;
        if (freq < min || freq > max) {
            Y[i] = 0; // フィルタ範囲外の成分をゼロにする
        }
    }

    // ピッチシフト
    if (shift != 0) {
        complex double *shifted_Y = calloc(n, sizeof(complex double));
        if (shifted_Y == NULL) {
            perror("calloc failed");
            free(X);
            free(Y);
            exit(EXIT_FAILURE);
        }

        if (shift > 0) {
            // ピッチを上げる
            for (long i = 0; i < n; ++i) {
                long new_index = (i + shift) % n;
                if (new_index < n) {
                    shifted_Y[new_index] = Y[i];
                }
            }
        } else {
            // ピッチを下げる
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

        free(shifted_Y);
    }

    // 逆FFT
    ifft(Y, X, n);
    // 標本の配列に変換
    complex_to_sample(X, output_buf, n);

    // メモリ解放
    free(X);
    free(Y);
}

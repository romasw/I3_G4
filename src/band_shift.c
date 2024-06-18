#include <stdio.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>
#include <limits.h>

void sample_to_complex(short *s, complex double *X, long n) {
    for (long i = 0; i < n; i++) X[i] = s[i];
}

void complex_to_sample(complex double *X, short *s, long n) {
    for (long i = 0; i < n; i++) {
        double value = creal(X[i]);
        if (value < SHRT_MIN) value = SHRT_MIN;
        if (value > SHRT_MAX) value = SHRT_MAX;
        s[i] = (short)value;
    }
}

void fft_r(complex double *x, complex double *y, long n, complex double w) {
    if (n == 1) {
        y[0] = x[0];
    } else {
        complex double W = 1.0;
        for (long i = 0; i < n / 2; i++) {
            y[i] = x[i] + x[i + n / 2];
            y[i + n / 2] = W * (x[i] - x[i + n / 2]);
            W *= w;
        }
        fft_r(y, x, n / 2, w * w);
        fft_r(y + n / 2, x + n / 2, n / 2, w * w);
        for (long i = 0; i < n / 2; i++) {
            y[2 * i] = x[i];
            y[2 * i + 1] = x[i + n / 2];
        }
    }
}

void fft(complex double *x, complex double *y, long n) {
    double arg = 2.0 * M_PI / n;
    complex double w = cos(arg) - I * sin(arg);
    fft_r(x, y, n, w);
}

void ifft(complex double *y, complex double *x, long n) {
    double arg = 2.0 * M_PI / n;
    complex double w = cos(arg) + I * sin(arg);
    fft_r(y, x, n, w);
    for (long i = 0; i < n; i++) x[i] /= n;  // IFFT後のスケーリング
}

void band_shift(short *input_buf, short *output_buf, long n, int shift) {
    int min = 30;   // フィルタリングの最小周波数
    int max = 1000; // フィルタリングの最大周波数
    double fs = 44100; // サンプリング周波数
    double freq_bin = fs / n;

    complex double *X = calloc(n, sizeof(complex double));
    complex double *Y = calloc(n, sizeof(complex double));
    if (X == NULL || Y == NULL) {
        perror("calloc failed");
        free(X);
        free(Y);
        exit(EXIT_FAILURE);
    }

    sample_to_complex(input_buf, X, n);
    fft(X, Y, n);

    // フィルタリング
    for (long i = 0; i < n; ++i) {
        double freq = i * freq_bin;
        if (freq < min || freq > max) {
            Y[i] = 0;
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

        // 時間領域でのピッチシフト
        if (shift > 0) {  // ピッチを上げる
            for (long i = 0; i < n; ++i) {
                long new_index = i - shift;
                if (new_index >= 0) {
                    shifted_Y[new_index] = Y[i];
                }
            }
        } else {  // ピッチを下げる
            shift = -shift;
            for (long i = 0; i < n; ++i) {
                long new_index = i + shift;
                if (new_index < n) {
                    shifted_Y[new_index] = Y[i];
                }
            }
        }

        for (long i = 0; i < n; ++i) {
            Y[i] = shifted_Y[i];
        }

        free(shifted_Y);
    }

    // 逆FFT
    ifft(Y, X, n);

    // スケーリングとクリッピング
    complex_to_sample(X, output_buf, n);

    free(X);
    free(Y);
}
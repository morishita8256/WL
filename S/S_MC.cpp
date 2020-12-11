#include <vector>
#include "S.hpp"

int main() {
  START(1);

  int samples = 10000;
  int interval = 10000;

  int bins;
  double Tc_min, Tc_max;
  double EPS, RW_step;
  double J_init;
  double Js_sum = 0.5;

  // Read file
  int dummy;
  FILE* fp = fopen("S_entropy.dat", "r");
  dummy = fscanf(fp, "%d", &bins);
  dummy = fscanf(fp, "%lf %lf", &Tc_min, &Tc_max);
  dummy = fscanf(fp, "%lf", &EPS);
  dummy = fscanf(fp, "%lf", &RW_step);
  dummy = fscanf(fp, "%lf", &J_init);

  double* S_s = alloc_dvector(bins);
  for (int i = 0; i < bins; i++)
    dummy = fscanf(fp, "%lf", &S_s[i]);
  fclose(fp);


  // Initialize
  int* hist = alloc_ivector(bins);
  for (int i = 0; i < bins; i++)
    hist[i] = 0;
  double* Tc_s = alloc_dvector(samples);
  double** Js_s = alloc_dmatrix(samples, 2);

  double bin_width = (Tc_max - Tc_min) / (double)bins;
  double cJs[2], nJs[2];
  cJs[0] = J_init;
  cJs[1] = Js_sum - cJs[0];
  double dJs;
  double cTc = Js_to_Tc(cJs, EPS), nTc;
  int cind = Tc_to_ind(cTc, Tc_min, bin_width, bins), nind;

  // Iteration
  for (int i = 0; i < samples; i++) {
    for (int _ = 0; _ < interval; _++) {
      dJs = (rand01() * 2 - 1) * RW_step;
      nJs[0] = cJs[0] + dJs;
      nJs[1] = Js_sum - nJs[0];
      nTc = Js_to_Tc(nJs, EPS);
      nind = Tc_to_ind(nTc, Tc_min, bin_width, bins);
      if (nind != -1 && rand01() <= exp1(S_s[cind], S_s[nind])) {
        cTc = nTc, cind = nind;
        for (int i = 0; i < 2; i++)
          cJs[i] = nJs[i];
      }
    }
    Tc_s[i] = cTc;
    for (int j = 0; j < 2; j++)
      Js_s[i][j] = cJs[j];
    hist[cind]++;

    if ((i + 1) % (samples / 100) == 0) {
      printf(((i + 1) % (samples / 10) == 0) ? "o" : ".");
      fflush(stdout);
    }
  }
  printf("\n");

  // Write to file
  fp = fopen("../data/S_Js.dat", "w");
  for (int i = 0; i < samples; i++) {
    fprintf(fp, "%.12f\t%.12f\n", Js_s[i][0], Js_s[i][1]);
  }

  fp = fopen("../data/S_Tc.dat", "w");
  for (int i = 0; i < samples; i++) {
    fprintf(fp, "%.12f\n", Tc_s[i]);
  }

  // Visualize
  double x, y, ymax = 0.0;
  for (int i = 0; i < bins; i++)
    ymax = max(ymax, (double)hist[i]);
  ymax *= 1.1;

  FILE* gp = popen("gnuplot -persist", "w");
  fprintf(gp, "set terminal pdfcairo color enhanced size 4in, 3in\n");
  fprintf(gp, "set output '../fig/S_hist.pdf'\n");
  fprintf(gp, "set style fill solid border lc rgb \"black\"\n");
  fprintf(gp, "set xrange [%f:%f]\n", Tc_min - 0.02, Tc_max + 0.02);
  fprintf(gp, "set yrange [0:%f]\n", ymax);
  fprintf(gp, "set xlabel \"T_c\"\n");
  fprintf(gp, "set ylabel \"Frequency\"\n");
  fprintf(gp, "set boxwidth %f\n", bin_width);
  fprintf(gp, "plot '-' with boxes notitle \"hist\"\n");
  for (int i = 0; i < bins; i++) {
    x = Tc_min + (i + 0.5) * bin_width;
    y = hist[i];
    fprintf(gp, "%.12f %.1f \n", x, y);
  }
  fprintf(gp, "e\n");
  pclose(gp);

  END();
}
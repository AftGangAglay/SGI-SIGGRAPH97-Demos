
#include <math.h>
#include <stdio.h>


#define PI 3.14159265


void
main(int argc, char** argv)
{
  int i, j, rows, columns;
  float x, y;

  if (argc < 3) {
    fprintf(stderr, "usage: sinc columns rows\n");
    exit(1);
  }

  sscanf(argv[1], "%d", &columns);
  sscanf(argv[2], "%d", &rows);

  printf("%d %d\n", columns, rows);

  for (j = 0; j < rows; j++) {
    for (i = 0; i < columns; i++) {
      x = PI * 8 * ((float)(i+1) / (float)columns) - PI * 4;
      y = PI * 8 * ((float)(j+1) / (float)rows) - PI * 4;
      if (x == 0.0)
	x = 1e-6;
      if (y == 0.0)
	y = 1e-6;
      printf("%2.2f ", sin(x) / x + sin(y) / y);
    }
    printf("\n");
  }
}

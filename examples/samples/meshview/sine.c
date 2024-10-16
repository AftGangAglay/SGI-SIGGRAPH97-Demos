
#include <math.h>
#include <stdio.h>


void
main(int argc, char** argv)
{
  int i, j, rows, columns;

  if (argc < 3) {
    fprintf(stderr, "usage: sine columns rows\n");
    exit(1);
  }

  sscanf(argv[1], "%d", &columns);
  sscanf(argv[2], "%d", &rows);

  printf("%d %d\n", columns, rows);

  for (j = 0; j < rows; j++) {
    for (i = 0; i < columns; i++) {
      printf("%2.2lf ", sin(6.28 * (float)i / (float)columns) +
	     sin(6.28 * (float)j / (float)rows));
    }
    printf("\n");
  }
}

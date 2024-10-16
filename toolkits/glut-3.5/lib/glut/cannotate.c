
/* This program isn't really a part of GLUT, but is here for
   historical reasons.  It will disappear next GLUT release. */

/* cannotate converts an "annotated C file" into either LaTeX
   or a compilable C file (with or without dead code).  I used
   this hack for the writing of my book. -mjk */

#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define TEX	1           /* generate annotated TeX forC code */
#define CCODE	2         /* generate shipping C code */
#define PCODE	3         /* generate publishable C code */

#define T_LINE	0
#define T_TEX	1
#define T_UNTEX	2
#define T_CODE	3
#define T_DEAD	4

FILE *ifile;
FILE *ofile;
char buffer[1024];
int mode = TEX;
int verbose = 1;
int tex, code;
int line;
int new_line_pending;
int indent = 0;
int just_entered_code;

int
line_type(char *buf)
{
  if (!strcmp("//tex\n", buf)) {
    return T_TEX;
  } else if (!strcmp("//untex\n", buf)) {
    return T_UNTEX;
  } else if (!strcmp("//code\n", buf)) {
    return T_CODE;
  } else if (!strcmp("//dead\n", buf)) {
    return T_DEAD;
  } else {
    return T_LINE;
  }
}

int
non_code(char *buf)
{
  if (!strcmp("#include <assert.h>\n", buf)) {
    return 1;
  }
  if (strstr(buf, "assert(")) {
    return 1;
  }
  if (strstr(buf, "ENDCENTRY")) {
    return 1;
  }
  if (strstr(buf, "CENTRY")) {
    return 1;
  }
  if (strstr(buf, "INDENT-ON")) {
    return 1;
  }
  if (strstr(buf, "INDENT-OFF")) {
    return 1;
  }
  return 0;
}

int
blank_line(char *buf)
{
  for (; *buf; buf++) {
    if (!isspace(*buf)) {
      return 0;
    }
  }
  return 1;
}

int
main(int argc, char **argv)
{
  char *iname, *suffix, *tail, *oname, obuf[512];
  char *gotsome;
  int type;
  int output, output_this;
  mode_t savemask;
  int i;

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      switch (argv[i][1]) {
      case 'i':
        indent = atoi(&argv[i][2]);
        break;
      case 't':
        mode = TEX;
        break;
      case 'c':
        mode = CCODE;
        break;
      case 'd':
        mode = PCODE;
        break;
      case 'v':
        verbose = 1;
        break;
      default:
        fprintf(stderr, "can: unrecognized option: %s\n", argv[i]);
        break;
      }
    } else {
      break;            /* exit loop; done with options */
    }
  }
  for (; i < argc; i++) {
    iname = argv[i];
    ifile = fopen(iname, "r");
    if (ifile == NULL) {
      fprintf(stderr, "can: could not open %s\n", iname);
      exit(1);
    }
    suffix = strrchr(iname, '.');
    if (suffix) {
      *suffix = '\0';
      suffix++;
      if (*suffix == 'a') {
        suffix++;
      } else {
        suffix = "c";
      }
    } else {
      suffix = "c";
    }
    switch (mode) {
    case TEX:
      sprintf(obuf, "%s.tex", iname);
      break;
    case CCODE:
      sprintf(obuf, "%s.%s", iname, suffix);
      break;
    case PCODE:
      sprintf(obuf, "%s_pub.%s", iname, suffix);
      break;
    }
    tail = strrchr(obuf, '/');
    if (tail) {
      oname = tail + 1;
    } else {
      oname = obuf;
    }
    /* Make sure the file is not writable after creation! */
    savemask = umask((mode_t) 0);
    umask((mode_t) savemask | S_IWUSR | S_IWGRP | S_IWOTH);
    (void) unlink(oname);  /* Remove existing file named oname. 

                            */
    ofile = fopen(oname, "w");
    umask((mode_t) savemask);
    if (ofile == NULL) {
      fprintf(stderr, "can: could not open %s\n", oname);
      exit(1);
    }
    line = 0;
    tex = 0;
    code = 0;
    output = 1;
    new_line_pending = 0;
    while (!feof(ifile)) {
      gotsome = fgets(buffer, sizeof(buffer), ifile);
      if (gotsome) {
        line++;
        type = line_type(buffer);
        switch (mode) {
        case TEX:
          switch (type) {
          case T_TEX:
            if (tex) {
              fprintf(stderr, "can: duplicate //tex seen: %d\n", line);
            }
            tex = 1;
            output_this = 0;
            output = 1;
            if (code) {
              new_line_pending = 0;
              fprintf(ofile, "\\end{verbatim}\n", buffer);
            }
            code = 0;
            break;
          case T_UNTEX:
            if (!tex) {
              fprintf(stderr, "can: unexpected //untex seen: %d\n", line);
            }
            if (code) {
              fprintf(stderr, "can: unexpected //untex seen: %d\n", line);
            }
            tex = 0;
            output_this = 0;
            output = 1;
            break;
          case T_CODE:
            if (tex) {
              tex = 0;
            }
            output_this = 0;
            output = 1;
            if (!code) {
              new_line_pending = 0;
              fprintf(ofile, "\\begin{verbatim}\n", buffer);
            }
            code = 1;
            just_entered_code = 1;
            break;
          case T_DEAD:
            if (tex) {
              tex = 0;
            }
            output_this = 0;
            output = 0;
            break;
          case T_LINE:
            if (code) {
              if (non_code(buffer)) {
                output_this = 0;
              } else {
                if (blank_line(buffer) && just_entered_code) {
                  output_this = 0;
                } else {
                  output_this = 1;
                  just_entered_code = 0;
                }
              }
            } else {
              output_this = 1;
            }
            break;
          }
          break;
        case CCODE:
          switch (type) {
          case T_TEX:
            if (tex) {
              fprintf(stderr, "can: duplicate //tex seen: %d\n", line);
            }
            tex = 1;
            output_this = 0;
            output = 0;
            break;
          case T_UNTEX:
            if (!tex) {
              fprintf(stderr, "can: unexpected //untex seen: %d\n", line);
            }
            tex = 0;
            output_this = 0;
            output = 1;
            break;
          case T_CODE:
            if (tex) {
              tex = 0;
            }
            output_this = 0;
            output = 1;
            break;
          case T_DEAD:
            if (tex) {
              tex = 0;
            }
            output_this = 0;
            output = 1;
            break;
          case T_LINE:
            output_this = 1;
            break;
          }
          break;
        case PCODE:
          switch (type) {
          case T_TEX:
            if (tex) {
              fprintf(stderr, "can: duplicate //tex seen: %d\n", line);
            }
            tex = 1;
            output_this = 0;
            output = 0;
            break;
          case T_UNTEX:
            if (!tex) {
              fprintf(stderr, "can: unexpected //untex seen: %d\n", line);
            }
            tex = 0;
            output_this = 0;
            output = 1;
            break;
          case T_CODE:
            if (tex) {
              tex = 0;
            }
            output_this = 0;
            output = 1;
            break;
          case T_DEAD:
            if (tex) {
              tex = 0;
            }
            output_this = 0;
            output = 0;
            break;
          case T_LINE:
            if (code && non_code(buffer)) {
              output_this = 0;
            } else {
              output_this = 1;
            }
            break;
          }
          break;
        }
        if (output && output_this) {
          if (blank_line(buffer)) {
            new_line_pending = 1;
          } else {
            if (new_line_pending) {
              fprintf(ofile, "\n");
              new_line_pending = 0;
            }
            if (indent && code) {
              int s;

              for (s = 0; s < indent; s++) {
                fputc(' ', ofile);
              }
            }
            fprintf(ofile, "%s", buffer);
          }
        }
      }
    }
    if (code && mode == TEX) {
      fprintf(ofile, "\\end{verbatim}\n");
    }
    fclose(ofile);
    fclose(ifile);
  }
  return 0;
}

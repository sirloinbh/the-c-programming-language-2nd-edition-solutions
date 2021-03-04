#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>

#define MAX_NR_OF_FIELDS 100
#define MAX_NR_OF_LINES 5000

#define LONG_MAX_NR_OF_DIGITS (size_t)(floor(log10(labs(LONG_MAX))) + 1)

int parse_arg_list(int argc, char *argv[]);

size_t str_nth_blank_pos(const char *s, size_t n);
char *substr(const char *s, size_t start, size_t end);
size_t read_lines(char *line_ptr[], const size_t max_nr_of_lines);
void write_lines(char *line_ptr[], const size_t nr_of_lines);

int numcmp(const char *s1, const char *s2);
int estrcmp(const char *s1, const char *s2);
int fieldscmp(const char *s1, const char *s2);
void swap(void *v[], size_t i, size_t j);
void quick_sort(void *v[], size_t start, size_t end, int (*comp)(void *, void *));

int order = 1;     // 1 ascendent, -1 descendent
int fold = 0;      // 0 case sensitive, 1 case insensitive
int directory = 0; // 0 normal, 1 directory
int (*comp)(const char *, const char *) = estrcmp;

int nr_of_fields = 0;
long fields_index[MAX_NR_OF_FIELDS];
int (*fields_comp[MAX_NR_OF_FIELDS])(const char *, const char *);
int fields_order[MAX_NR_OF_FIELDS];
int fields_fold[MAX_NR_OF_FIELDS];
int fields_directory[MAX_NR_OF_FIELDS];

int main(int argc, char *argv[])
{
  if (!parse_arg_list(argc, argv))
  {
    puts("ERROR: Invalid arguments.");
    return EXIT_FAILURE;
  }

  for (size_t i = 0; i < argc - 1; i++)
  {
    printf("%ld\t%d\t%d\t%d\n", fields_index[i], fields_order[i], fields_fold[i], fields_directory[i]);
  }

  size_t nr_of_lines;
  char *line_ptr[MAX_NR_OF_LINES];

  if ((nr_of_lines = read_lines(line_ptr, MAX_NR_OF_LINES)) != -1)
  {
    quick_sort((void **)line_ptr, 0, nr_of_lines - 1, (int (*)(void *, void *))comp);
    write_lines(line_ptr, nr_of_lines);
  }
  else
  {
    puts("ERROR: Input too large.");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int parse_arg_list(int argc, char *argv[])
{
  for (int i = 1; i < argc; ++i)
  {
    size_t arg_len = strlen(argv[i]);
    if (arg_len > 1 && argv[i][0] == '-')
    {
      for (size_t j = 1; j < arg_len; ++j)
      {
        if (isdigit(argv[i][j]) && !fields_index[i - 1])
        {
          size_t k = 0;
          char temp[LONG_MAX_NR_OF_DIGITS];
          while (isdigit(argv[i][j]) && j < arg_len && k < LONG_MAX_NR_OF_DIGITS)
          {
            temp[k++] = argv[i][j++];
          }
          temp[k] = '\0';
          fields_index[i - 1] = atol(temp);

          --j;
          ++nr_of_fields;
        }
        else
        {
          switch (argv[i][j])
          {
          case 'n':
            comp = numcmp;
            break;

          case 'f':
            fold = 1;
            break;

          case 'd':
            directory = 1;
            break;

          case 'r':
            order = -1;
            break;

          default:
            return 0;
            break;
          }
        }
      }

      if (nr_of_fields || argc > 2)
      {
        if (!fields_index[i - 1])
        {
          return 0;
        }

        fields_comp[i - 1] = comp;
        fields_order[i - 1] = order;
        fields_fold[i - 1] = fold;
        fields_directory[i - 1] = directory;

        comp = estrcmp;
        order = 1;
        fold = 0;
        directory = 0;
      }
    }
    else
    {
      return 0;
    }
  }

  if (nr_of_fields && nr_of_fields == argc - 1)
  {
    comp = fieldscmp;
  }
  else if (argc > 2)
  {
    return 0;
  }

  return 1;
}

size_t str_nth_blank_pos(const char *s, size_t n)
{
  size_t pos = 0;
  while (n && *s != '\0')
  {
    if (*s == ' ')
    {
      do
      {
        ++pos;
        ++s;
      } while (*s == ' ');

      --n;
    }
    else
    {
      ++pos;
      ++s;
    }
  }

  return pos;
}

char *substr(const char *s, size_t start, size_t end)
{
  if (start > end && end < strlen(s))
  {
    return NULL;
  }

  const size_t len = end - start;
  char *dest = (char *)malloc(sizeof(char) * (len + 1));
  strncpy(dest, s + start, end);
  dest[len + 1] = '\0';

  return dest;
}

size_t read_lines(char *line_ptr[], const size_t max_nr_of_lines)
{
  size_t line_length;
  size_t nr_of_lines = 0;
  size_t bufsize = 0;

  char *current_line = NULL;
  char *current_line_copy = NULL;

  while ((line_length = getline(&current_line, &bufsize, stdin)) != -1)
  {
    if (nr_of_lines >= max_nr_of_lines || (current_line_copy = (char *)malloc(line_length * sizeof(char))) == NULL)
    {
      return -1;
    }
    else
    {
      current_line[line_length - 1] = '\0';
      strcpy(current_line_copy, current_line);
      line_ptr[nr_of_lines++] = current_line_copy;
    }
  }

  free(current_line);

  return nr_of_lines;
}

void write_lines(char *line_ptr[], const size_t nr_of_lines)
{
  for (size_t i = 0; i < nr_of_lines; ++i)
  {
    puts(line_ptr[i]);
    free(line_ptr[i]);
  }
}

int numcmp(const char *s1, const char *s2)
{
  double nr1 = atof(s1);
  double nr2 = atof(s2);

  if (nr1 < nr2)
  {
    return order * -1;
  }
  else if (nr1 > nr2)
  {
    return order * 1;
  }

  return 0;
}

int estrcmp(const char *s1, const char *s2)
{
  while (*s1 != '\0' && *s2 != '\0')
  {
    if (directory)
    {
      while (*s1 != '\0' && !isalnum(*s1) && !isspace(*s1))
      {
        ++s1;
      }
      while (*s2 != '\0' && !isalnum(*s2) && !isspace(*s2))
      {
        ++s2;
      }
    }

    int result = fold ? tolower(*s1) - tolower(*s2) : *s1 - *s2;
    if (result == 0)
    {
      ++s1;
      ++s2;
    }
    else
    {
      return order * result;
    }
  }

  return 0;
}

int fieldscmp(const char *s1, const char *s2)
{
  int i = 0;
  while (i < nr_of_fields)
  {
    size_t start_s1 = str_nth_blank_pos(s1, fields_index[i] - 1);
    size_t end_s1 = str_nth_blank_pos(s1, fields_index[i]);
    char *field_s1 = substr(s1, start_s1, end_s1);

    size_t start_s2 = str_nth_blank_pos(s2, fields_index[i] - 1);
    size_t end_s2 = str_nth_blank_pos(s2, fields_index[i]);
    char *field_s2 = substr(s2, start_s2, end_s2);

    comp = fields_comp[i];
    order = fields_order[i];
    fold = fields_fold[i];
    directory = fields_directory[i];

    int comp_result = comp(field_s1, field_s2);

    puts(field_s1);
    puts(field_s2);

    free(field_s1);
    free(field_s2);

    if (comp_result == 0)
    {
      ++i;
    }
    else
    {
      return comp_result;
    }
  }

  return 0;
}

void swap(void *v[], size_t i, size_t j)
{
  void *temp;
  temp = v[i];
  v[i] = v[j];
  v[j] = temp;
}

void quick_sort(void *v[], size_t start, size_t end, int (*comp)(void *, void *))
{
  if ((long)start >= (long)end)
  {
    return;
  }

  swap(v, start, (start + end) / 2);

  size_t last = start;
  for (size_t i = start + 1; i <= end; ++i)
  {
    if ((*comp)(v[i], v[start]) < 0)
    {
      swap(v, ++last, i);
    }
  }

  swap(v, start, last);
  quick_sort(v, start, last - 1, comp);
  quick_sort(v, last + 1, end, comp);
}

// NOTE: run: ./sort -df < file_in.txt
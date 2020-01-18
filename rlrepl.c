#include <janet.h>
#include <readline.h>
#include <history.h>

static char *longest_common_prefix(char **strs, int n) {
  int shortest_len = -1;
  char *shortest_str = NULL;

  for (int i = 0; i < n; i++) {
    char *s = strs[i];
    int len = strlen(s);
    if (shortest_len == -1 || len < shortest_len) {
      shortest_len = len;
      shortest_str = s;
    }
  }

  if (shortest_len < 0)
    abort();

  int longest_prefix = shortest_len;

  for (int i = 0; i < n; i++) {
    char *s = strs[i];
    for (int j = 0; j < longest_prefix; j++) {
      if (s[j] != shortest_str[j]) {
        longest_prefix = j;
        break;
      }
    }
  }

  char *pfx = strndup(shortest_str, longest_prefix);
  if (!pfx)
    abort();

  return pfx;
}

static JanetFunction *completion_janet_function = NULL;
static char **rlrepl_readline_attempted_completion(const char *text, int start,
                                                   int end) {
  if (!completion_janet_function)
    return NULL;

  Janet line =
      janet_wrap_string(janet_string((const uint8_t *)rl_line_buffer, end));
  JanetFiber *fiber = NULL;
  Janet completions = janet_wrap_nil();
  const int nargs = 3;
  Janet *args = janet_tuple_begin(nargs);
  args[0] = line;
  args[1] = janet_wrap_integer(start);
  args[2] = janet_wrap_integer(end);
  janet_tuple_end(args);

  janet_gcroot(janet_wrap_tuple(args));
  janet_gcroot(janet_wrap_function(completion_janet_function));

  int nrlcompletions = 0;
  char **rlcompletions = NULL;

  JanetSignal status =
      janet_pcall(completion_janet_function, nargs, args, &completions, &fiber);
  if (status == JANET_SIGNAL_OK) {
    if (janet_type(completions) == JANET_ARRAY) {
      JanetArray *ca = janet_unwrap_array(completions);
      for (int i = 0; i < ca->count; i++) {
        Janet j = ca->data[i];

        if (janet_type(j) != JANET_STRING)
          continue;

        const uint8_t *jstr = janet_unwrap_string(j);
        const char *cstr = (const char *)jstr;
        size_t cstrlen = strlen(cstr);

        if (cstrlen != (size_t)janet_string_length(jstr))
          continue;

        if (!rlcompletions) {
          // We need at least space for substitution + matches + null
          // libeditline assumes at least this many NULLs.
          rlcompletions = calloc(ca->count + 2, sizeof(char *));
          nrlcompletions = 0;
        }

        if (!rlcompletions)
          abort();
        char *completion = strdup(cstr);
        if (!completion)
          abort();
        rlcompletions[nrlcompletions + 1] = completion;
        nrlcompletions += 1;
      }
    }
  }

  janet_gcunroot(janet_wrap_tuple(args));
  janet_gcunroot(janet_wrap_function(completion_janet_function));

  if (rlcompletions) {
    char *pfx = longest_common_prefix(rlcompletions + 1, nrlcompletions);
    rlcompletions[0] = pfx;
  }

  rl_attempted_completion_over = 1;
  rl_completion_append_character = 0;
  return rlcompletions;
}

static Janet rlrepl_readline(int32_t argc, Janet *argv) {
  static int recursion = 0;
  if (recursion)
    janet_panic("readline cannot be called from readline!");
  recursion = 1;

  janet_fixarity(argc, 2);

  Janet ret = janet_wrap_nil();

  rl_attempted_completion_function = rlrepl_readline_attempted_completion;

  const char *prompt = janet_getcstring(argv, 0);
  completion_janet_function = janet_getfunction(argv, 1);
  char *ln = readline(prompt);
  if (ln) {
    if (*ln)
      add_history(ln);
    ret = janet_cstringv(ln);
    free(ln);
  }

  recursion = 0;
  return ret;
}

static Janet rlrepl_save_history(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 1);
  if (write_history(janet_getcstring(argv, 0)) != 0)
    janet_panic("rlrepl_save_history: error");
  return janet_wrap_nil();
}

static Janet rlrepl_load_history(int32_t argc, Janet *argv) {
  janet_fixarity(argc, 1);
  if (read_history(janet_getcstring(argv, 0)) != 0)
    janet_panic("rlrepl_load_history: error");
  return janet_wrap_nil();
}

static const JanetReg cfuns[] = {{"readline", rlrepl_readline, NULL},
                                 {"load-history", rlrepl_load_history, NULL},
                                 {"save-history", rlrepl_save_history, NULL},
                                 {NULL, NULL, NULL}};

JANET_MODULE_ENTRY(JanetTable *env) { janet_cfuns(env, "_rlrepl", cfuns); }
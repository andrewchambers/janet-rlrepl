(import _rlrepl)

(var *get-completions*
  (fn get-completions
    [env line start end]
    (var to-expand (string (string/slice line start end)))
    (def completions 
      (->>
         (all-bindings env)
         (map string)
         (filter (fn [s] (string/has-prefix? to-expand s)))))
    completions))

(var *get-prompt*
  (fn get-prompt
    [p]
    (def [lineno] (parser/where p))
    (string "janet:" lineno ":" (parser/state p :delimiters) "> ")))

(var *history-file* nil)

(defn rlrepl
  [&opt replenv]
  (default replenv (fiber/getenv (fiber/current)))

  (when *history-file*
    (when (os/stat *history-file*)
      (_rlrepl/load-history *history-file*)))
  
  (defn getline [prompt buf]
    (defn get-completions
      [line start end]
      (try
        (*get-completions* replenv line start end)
        ([err f]
          (eprint "error in tab completion function:")
          (debug/stacktrace f err)
          @[])))
    (when-let [ln (_rlrepl/readline prompt get-completions)]
      (buffer/push-string buf ln "\n")
      buf))

  (defn getchunk [buf p]
    (def prompt (*get-prompt* p))
    (getline prompt buf))

  (repl getchunk nil replenv)
  (when *history-file*
    (_rlrepl/save-history *history-file*)))

(def save-history _rlrepl/save-history)
(def load-history _rlrepl/load-history)

(declare-project
  :name "rlrepl"
  :description "A Janet repl using GNU readline"
  :author "Andrew Chambers"
  :url "https://github.com/andrewchambers/janet-rlrepl"
  :repo "git+https://github.com/andrewchambers/janet-rlrepl.git")

(declare-source
  :name "rlrepl"
  :source ["rlrepl.janet"])

(defn exec-get-string-and-exit-code [args]
  (def env (os/environ))
  (def streams (os/pipe))
  (put env :out (streams 1))
  (def proc (os/spawn args :pe env))
  (ev/close (streams 1))
  (def text (string/trim (ev/read (streams 0) :all)))
  (def exit-code (os/proc-wait proc))
  {:exit-code exit-code :text text})

(defn pkg-config [& what]
  (def result (exec-get-string-and-exit-code ["pkg-config" ;what]))
  (unless (zero? (result :exit-code))
    (error "pkg-config failed!"))
  (string/split " " (result :text)))

(declare-native
  :name "_rlrepl"
  :cflags (pkg-config "readline" "--cflags")
  :lflags (pkg-config "readline" "--libs")
  :source ["rlrepl.c"])

(declare-source
  :name "rlrepl"
  :source "rlrepl.janet")

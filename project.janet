
(declare-project
  :name "rlrepl"
  :author "Andrew Chambers"
  :url "https://github.com/andrewchambers/janet-rlrepl"
  :repo "git+https://github.com/andrewchambers/janet-rlrepl.git")

(declare-source
  :name "rlrepl"
  :source ["rlrepl.janet"])

(defn exec_get_string_and_exit_code [args]
  (def env (os/environ))
  (def streams (os/pipe))
  (put env :out (streams 1))
  (def exit_code (os/execute args :pe env))
  (ev/close (streams 1))
  {:exit_code exit_code :text (string/trim (ev/read (streams 0) :all))})

(defn pkg-config [& what]
  (def result (exec_get_string_and_exit_code ["pkg-config" ;what]))
  (unless (zero? (result :exit_code))
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

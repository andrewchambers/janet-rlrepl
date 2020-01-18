
(declare-project
  :name "rlrepl"
  :author "Andrew Chambers"
  :url "https://github.com/andrewchambers/janet-rlrepl"
  :repo "git+https://github.com/andrewchambers/janet-rlrepl.git")

(declare-source
  :name "rlrepl"
  :source ["rlrepl.janet"])

(defn pkg-config [what]
  (def f (file/popen (string "pkg-config " what)))
  (def v (->>
           (file/read f :all)
           (string/trim)
           (string/split " ")))
  (unless (zero? (file/close f))
    (error "pkg-config failed!"))
  v)

(declare-native
  :name "_rlrepl"
  :cflags (pkg-config "readline --cflags")
  :lflags (pkg-config "readline --libs")
  :source ["rlrepl.c"])

(declare-source
  :name "rlrepl"
  :source "rlrepl.janet")
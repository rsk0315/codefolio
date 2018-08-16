(provide 'markdown)

(require 'compile)

;; See:
;; ~/share/emacs/25.3/lisp/font-lock.el.gz
;; ~/share/emacs/25.3/lisp/textmodes/tex-mode.el.gz
;; https://www.gnu.org/software/emacs/manual/html_node/elisp/Search_002dbased-Fontification.html#Search_002dbased-Fontification

;; Not working properly:
;; backticks within markers (*like `this`*)
;; improper nesting (*like _this*_)

(defface markdown-math
  '((t :inherit font-lock-string-face))
  "Face used to highlight TeX math expressions."
  :group 'markdown)
(defvar markdown-math-face 'markdown-math)

(defface markdown-code
  '((t :inherit font-lock-string-face))
  "Face used to highlight codes."
  :group 'markdown)
(defvar markdown-code-face 'markdown-code)

(defface markdown-atcmd
  '((t :inherit font-lock-variable-face :weight bold))
  "Face used to highlight @ commands."
  :group 'markdown)
(defvar markdown-atcmd-face 'markdown-atcmd)

;; Sample
;; (defface markdown-foo
;;   '((t :inherit font-lock-keyword-face :weight bold))
;;   "Face used to highlight foo."
;;   :group 'markdown)
;; (defvar markdown-foo-face 'markdown-foo)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

(defun re-repeat (char count)
  (concat
   (if (> count 0) (regexp-quote char) "")
   (if (> count 1) (format "\\{%d\\}" count))))

(defun re-repeat-at-most (char count)
  (concat
   (if (> count 0) (regexp-quote char) "")
   (if (= count 1) "?" "")
   (if (> count 1) (format "\\{0,%d\\}" count))))

(defun re-char-inside-paren (char)
  (concat "[^\n\\" char "]"))

(defconst re-unescaped
  ;; not escaped by a backslash
  "\\(?:^\\|[^\\]\\)\\(?:\\\\.\\)*")

;; (setq font-lock-multiline t)  ;; ???

(defun re-paren (char count)
  (concat
   re-unescaped
   "\\("
   (re-repeat char count)
   "\\(?:"
   (re-repeat-at-most char (1- count))
   (re-char-inside-paren char)
   "\\|\\\\.\\)+"
   (re-repeat char count)
   "\\)"))

(defun markdown-font-lock-append-prop (prop)
  ;; (princ "<")
  ;; TODO: in processing *, if an unescaped _ is found before an
  ;;     : unescaped *, then return font-lock-warning-face.
  ;;     : Take care of nil-comparison, point, and order of process,
  ;;     : # of *, ...
  (unless (memq (car (get-text-property (match-beginning 1) 'face))
                `(font-lock-comment-face
                  ,markdown-code-face
                  ,markdown-math-face
                  ,markdown-atcmd-face))
    ;; (princ (match-beginning 1))
    ;; (princ " ")
    ;; (princ (car (get-text-property (match-beginning 1) 'face)))
    ;; (princ ">")
    prop))

(add-hook
 'markdown-mode-hook
 (lambda ()
   (font-lock-add-keywords
    nil
    ;; math: anywhere but in verbatim
    ;; verbatim: anywhere but in math
    ;; emphasis: neither in math nor in verbatim
    `(
      ;; math
      (,(re-paren "$" 1) 1 (markdown-font-lock-append-prop markdown-math-face) append)

      ;; `verbatim` styles
      (,(re-paren "`" 1) 1 (markdown-font-lock-append-prop markdown-code-face) append)
      (,(re-paren "`" 2) 1 (markdown-font-lock-append-prop markdown-code-face) append)
      (,(re-paren "`" 3) 1 (markdown-font-lock-append-prop markdown-code-face) append)
      (,(re-paren "`" 4) 1 (markdown-font-lock-append-prop markdown-code-face) append)
      (,(re-paren "`" 5) 1 (markdown-font-lock-append-prop markdown-code-face) append)

      ;; @ command
      (,(concat
         re-unescaped
         "\\(@"
         "\\(?:\\[\\([_0-9A-Za-z:#-]+\\)\\]\\)"  ;; label
         "\\(\\(?:\\\\.\\|[^\\@$\n]\\)*\\)"
         "@\\)")
       (1 . ((markdown-font-lock-append-prop 'font-lock-variable-name-face) append))
       (2 . ((markdown-font-lock-append-prop 'font-lock-builtin-face) prepend))
       (3 . ((markdown-font-lock-append-prop markdown-atcmd-face) prepend)))

      ;; *bold* styles
      (,(re-paren "*" 1) 1 (markdown-font-lock-append-prop 'bold) append)
      (,(re-paren "*" 2) 1 (markdown-font-lock-append-prop 'bold) append)
      (,(re-paren "*" 3) 1 (markdown-font-lock-append-prop 'bold) append)
      (,(re-paren "*" 4) 1 (markdown-font-lock-append-prop 'bold) append)
      (,(re-paren "*" 5) 1 (markdown-font-lock-append-prop 'bold) append)

      ;; _italic_ styles
      (,(re-paren "_" 1) 1 (markdown-font-lock-append-prop 'italic) append)
      (,(re-paren "_" 2) 1 (markdown-font-lock-append-prop 'italic) append)
      (,(re-paren "_" 3) 1 (markdown-font-lock-append-prop 'italic) append)
      (,(re-paren "_" 4) 1 (markdown-font-lock-append-prop 'italic) append)
      (,(re-paren "_" 5) 1 (markdown-font-lock-append-prop 'italic) append)

      ;; # directives
      ;; sections 
      ("^\\(#+\\) " 1 '(font-lock-comment-face 'bold) append)
      ("^#+ \\(.+\\)" 1 '(font-lock-keyword-face 'bold) append)

      ;; footnotes
      ;; ("^\\(#\\[\\(\\^\\).+\\)"
      ;;  (1 . (font-lock-comment-face append))
      ;;  (2 . (font-lock-warning-face t)))
      ("^\\(#\\[\\(\\^\\)\\([^\]]+\\)\\].+\\)"
       (1 . (font-lock-comment-face prepend))
       (2 . (font-lock-variable-name-face prepend))
       (3 . (font-lock-keyword-face t)))
      (,(concat
         ;; not (escaped or preceded by #)
         "\\(?:^\\|[^\\#]\\)\\(?:\\\\.\\)*"
         "\\(\\[\\(\\^\\)\\([_0-9A-Za-z-]+\\)\\]\\)")
       (1 . (font-lock-builtin-face append))
       (2 . (font-lock-warning-face prepend))
       (3 . ('bold append)))

      ;; XXX multi-line highlighting is removed when edited
      ;;     re-highlighted when editing a part before it

      ;; pre-formatted
      ("^\\(#`.*\\)\\(?:\n\\|.\\)*?\n\\1" 0 font-lock-string-face t)

      ;; comment
      ("^#%.*" 0 font-lock-comment-face t)
      ("^\\(#\\){\\(?:\n\\|.\\)*?\n\\1}" 0 font-lock-comment-face t)

      ;; ;; test
      ;; (test-font-lock-match (1 (test-font-lock (match-beginning 0)) append))
      ))) t)

(defvar markdown-mode-syntax-table
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?$ "$$" table)
    (modify-syntax-entry ?\\ "/" table)
    (modify-syntax-entry ?` "$`" table)
    table))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-derived-mode markdown-mode prog-mode
  "Markdown"
  "Major mode for editing rsk0315-flavored markdown language."
  (set-syntax-table markdown-mode-syntax-table)
  )

(add-to-list 'auto-mode-alist '("\\.md\\'" . markdown-mode))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
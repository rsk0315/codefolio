(provide 'markdown)

(require 'compile)

;; See:
;; ~/share/emacs/25.3/lisp/font-lock.el.gz
;; ~/share/emacs/25.3/lisp/textmodes/tex-mode.el.gz
;; https://www.gnu.org/software/emacs/manual/html_node/elisp/Search_002dbased-Fontification.html#Search_002dbased-Fontification

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
  (concat "[^\n$\\" char "]"))

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

(add-hook
 'markdown-mode-hook
 (lambda ()
   (font-lock-add-keywords
    nil
    `(
      ;; *bold* styles
      (,(concat (re-paren "*" 1)) 1 'bold append)
      (,(concat (re-paren "*" 2)) 1 'bold append)
      (,(concat (re-paren "*" 3)) 1 'bold append)
      (,(concat (re-paren "*" 4)) 1 'bold append)
      (,(concat (re-paren "*" 5)) 1 'bold append)

      ;; _italic_ styles
      (,(concat (re-paren "_" 1)) 1 'italic append)
      (,(concat (re-paren "_" 2)) 1 'italic append)
      (,(concat (re-paren "_" 3)) 1 'italic append)
      (,(concat (re-paren "_" 4)) 1 'italic append)
      (,(concat (re-paren "_" 5)) 1 'italic append)

      ;; `verbatim` styles
      (,(concat (re-paren "`" 1)) 1 font-lock-string-face append)
      (,(concat (re-paren "`" 2)) 1 font-lock-string-face append)
      (,(concat (re-paren "`" 3)) 1 font-lock-string-face append)
      (,(concat (re-paren "`" 4)) 1 font-lock-string-face append)
      (,(concat (re-paren "`" 5)) 1 font-lock-string-face append)

      ;; # directives
      ;; sections 
      ("^\\(#+\\) " 1 '(font-lock-comment-face 'bold) append)
      ("^#+ \\(.+\\)" 1 '(font-lock-keyword-face 'bold) append)

      ;; footnotes
      ("^\\(#\\[.+\\)" 1 font-lock-comment-face append)
      ("^#\\[\\^\\([^\]]+\\)\\]" 1 font-lock-keyword-face t)
      (,(concat re-unescaped "\\[\\^\\([_0-9A-Za-z-]+\\)\\]") 1 'bold append)

      ;; XXX multi-line highlighting is removed when edited
      ;;     re-highlighted when editing a part before it

      ;; pre-formatted
      ("^\\(#`.*\\)\\(?:\n\\|.\\)*?\n\\1" 0 font-lock-string-face t)

      ;; comment
      ("^#%.*" 0 font-lock-comment-face t)
      ("^\\(#\\){\\(?:\n\\|.\\)*?\n\\1}" 0 font-lock-comment-face t)

      ;; @ command
      (,(concat
         re-unescaped
         "\\(" "@" "\\(?:\\[\\([_0-9A-Za-z:#-]+\\)\\]\\)?" "\\)")
       (1 . (font-lock-variable-name-face append))
       (2 . (font-lock-builtin-face prepend)))
      ))) t)

(defvar markdown-mode-syntax-table
  (let ((table (make-syntax-table)))
    (modify-syntax-entry ?$ "\"" table)
    (modify-syntax-entry ?\\ "\\" table)
    table))

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-derived-mode markdown-mode prog-mode
  "Markdown"
  "Major mode for editing rsk0315-flavored markdown language."
  (set-syntax-table markdown-mode-syntax-table)
  )

(add-to-list 'auto-mode-alist '("\\.md\\'" . markdown-mode))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

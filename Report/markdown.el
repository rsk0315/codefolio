(provide 'markdown)

(require 'compile)

;; See:
;; ~/share/emacs/25.3/lisp/font-lock.el.gz
;; ~/share/emacs/25.3/lisp/textmodes/tex-mode.el.gz
;; https://www.gnu.org/software/emacs/manual/html_node/elisp/Search_002dbased-Fontification.html#Search_002dbased-Fontification

;; Not working properly:
;; backticks within markers (*like `this`*)
;; improper nesting (*like _this*_)

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
  (concat "[^\n$`\\" char "]"))

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

;;;
;; (defcustom test-font-script-display '(-0.2 0.2)
;;   :type '(list (float :tag "Subscript")
;;                (float :tag "Superscript"))
;;   :version "23.1")

;; (defun test-font-lock (pos)
;;   (unless (or (memq (get-text-property pos 'face)
;;                     '(font-lock-constant-face font-lock-builtin-face
;;                       font-lock-comment-face font-lock-string-face))
;;               ;; Check for backslash quoting
;;               (let ((odd nil)
;;                     (pos pos))
;;                 (while (eq (char-before pos) ?\\)
;;                   (setq pos (1- pos) odd (not odd)))
;;                 odd))
;;     (if (eq (char-after pos) ?_)
;;         `(face font-lock-keyword-face display (raise ,(car test-font-script-display)))
;;       `(face font-lock-warning-face display (raise ,(cadr test-font-script-display))))))

;; (defun test-font-lock-match (limit)
;;   (when (re-search-forward "\\(ab\\(c.+\\)d\\(e\\)\\)" limit t)
;;     (when (match-end 3)
;;       (let ((beg (match-beginning 3))
;;             (end (save-restriction
;;                    (narrow-to-region (point-min) limit)
;;                    (condition-case nil (scan-lists (point) 1 1) (error nil)))))
;;         (store-match-data (if end
;;                               (list (match-beginning 0) end beg end)
;;                             (list beg beg beg beg)))))
;;     t))
;;;

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

      ;; # directives
      ;; sections 
      ("^\\(#+\\) " 1 '(font-lock-comment-face 'bold) append)
      ("^#+ \\(.+\\)" 1 '(font-lock-keyword-face 'bold) append)

      ;; footnotes
      ;; ("^\\(#\\[\\(\\^\\).+\\)"
      ;;  (1 . (font-lock-comment-face append))
      ;;  (2 . (font-lock-warning-face t)))
      ("^\\(#\\[\\(\\^\\)\\([^\]]+\\)\\].+\\)"
       (1 . (font-lock-comment-face append))
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

      ;; @ command
      (,(concat
         re-unescaped
         "\\(@"
         "\\(?:\\[\\([_0-9A-Za-z:#-]+\\)\\]\\)"  ;; label
         "\\(\\(?:\\\\.\\|[^\\@$\n]\\)*\\)"
         "@\\)")
       (1 . (font-lock-variable-name-face append))
       (2 . (font-lock-builtin-face prepend))
       (3 . ('bold append)))

      ;; math
      (,(concat (re-paren "$" 1)) 1 'font-lock-string-face t)

      ;; `verbatim` styles
      (,(concat (re-paren "`" 1)) 1 font-lock-string-face t)
      (,(concat (re-paren "`" 2)) 1 font-lock-string-face t)
      (,(concat (re-paren "`" 3)) 1 font-lock-string-face t)
      (,(concat (re-paren "`" 4)) 1 font-lock-string-face t)
      (,(concat (re-paren "`" 5)) 1 font-lock-string-face t)

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
;; ;; Use string syntax but math face for $...$.
;; (defun md-font-lock-syntactic-face-function (state)
;;   (let ((char (nth 3 state)))
;;     (cond
;;      ((not char)
;;       (if (eq 2 (nth 7 state)) font-lock-string-face font-lock-comment-face))
;;      ((eq char ?$) font-lock-string-face)
;;      ;; A \verb element.
;;      (t font-lock-string-face))))

;; (defun md-font-lock-unfontify-region (beg end)
;;   (font-lock-default-unfontify-region beg end)
;;   (while (< beg end)
;;     (let ((next (next-single-property-change beg 'display nil end))
;;           (prop (get-text-property beg 'display)))
;;       (if (and (eq (car-safe prop) 'raise)
;;                (member (car-safe (cdr prop)) test-font-script-display)
;;                (null (cddr prop)))
;;           (put-text-property beg next 'display nil))
;;       (setq beg next))))

;; (setq-local font-lock-defaults
;;             '(nil
;;               nil nil nil nil
;;               ;; Who ever uses that anyway ???
;;               (font-lock-mark-block-function . mark-paragraph)
;;               (font-lock-syntactic-face-function
;;                . md-font-lock-syntactic-face-function)
;;               (font-lock-unfontify-region-function
;;                . md-font-lock-unfontify-region)))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
(define-derived-mode markdown-mode prog-mode
  "Markdown"
  "Major mode for editing rsk0315-flavored markdown language."
  (set-syntax-table markdown-mode-syntax-table)
  )

(add-to-list 'auto-mode-alist '("\\.md\\'" . markdown-mode))
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

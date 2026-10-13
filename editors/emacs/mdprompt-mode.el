;;; mdprompt-mode.el --- mdprompt highlighting -*- lexical-binding: t; -*-
;; Requires markdown-mode; native parsing defines the authoritative boundary.
(require 'markdown-mode)
(defface mdprompt-instruction-face '((t (:inherit font-lock-warning-face)))
  "AI request text." :group 'markdown)
(defun mdprompt--escaped-p (pos)
  "Whether the character at POS is preceded by an odd backslash run."
  (save-excursion
    (goto-char pos)
    (let ((count 0))
      (while (eq (char-before) ?\\) (backward-char) (setq count (1+ count)))
      (= (% count 2) 1))))
(defun mdprompt--protected-p (pos)
  (save-match-data
    (or (markdown-code-block-at-pos pos)
        (markdown-inline-code-at-pos-p pos)
        (save-excursion
          (goto-char (point-min))
          (let ((found nil)
                (regex (concat markdown-regex-math-inline-double "\\|"
                               markdown-regex-math-inline-single)))
            (while (and (not found) (re-search-forward regex nil t))
              (when (and (<= (match-beginning 0) pos) (< pos (match-end 0)))
                (setq found t)))
            found)))))
(defun mdprompt--match-instruction (limit)
  (catch 'found
    (while (re-search-forward "\\^%" limit t)
      (let* ((start (match-beginning 0))
             (block (save-excursion
                      (goto-char start)
                      (and (looking-at "\\^%%[ \t]*$")
                           (looking-back "^ \\{0,3\\}" (line-beginning-position)))))
             (reserved (and (eq (char-after) ?%)
                            (not (eq (char-after (1+ (point))) ?^)))))
        (unless (or (mdprompt--escaped-p start) (mdprompt--protected-p start)
                    (and reserved (not block)))
          (let ((end nil)
                (bound (if block (point-max) (line-end-position))))
            (if block
                (when (re-search-forward "^ \\{0,3\\}%%\\^[ \t]*$" nil t)
                  (setq end (match-end 0)))
              (while (and (not end) (search-forward "%^" bound t))
                (unless (mdprompt--escaped-p (- (point) 2)) (setq end (point)))))
            (setq end (or end bound))
            (set-match-data (list start end))
            (goto-char end)
            (throw 'found t)))))))
;;;###autoload
(define-derived-mode mdprompt-mode gfm-mode "mdprompt"
  "GFM with TeX and explicit AI requests."
  (setq-local markdown-enable-math t)
  (font-lock-add-keywords nil '((mdprompt--match-instruction 0 'mdprompt-instruction-face prepend)) t)
  (setq-local font-lock-multiline t))
;;;###autoload
(add-to-list 'auto-mode-alist '("\\.\\(?:mdp\\|mdprompt\\)\\'" . mdprompt-mode))
(provide 'mdprompt-mode)
;;; mdprompt-mode.el ends here

#! /usr/bin/env python3

"""A tool for converting markdown files to other format. In particular,
destination format is LaTeX and HTML."""

import os
import re
import sys

# Added features:
# @...@ for typewriter fonts are deprecated.
# We defined @[cmd]...@ to typeset e.g. \textsc{...},
# instead of raw-style: &\textsc{...}&.
# @[tt]...@, @[sc]...@, @[color:red]...@ are introduced.

# ---- Exceptions ------------
class MarkdownDiagnostic(SyntaxError):
    """Diagnostic raised or printed for invalid syntax."""
    CLASSNAME = 'MarkdownDiagnostic'
    BOLD_SEQ = '\x1b[1m'
    RESET_SEQ = '\x1b[0m'
    INDENT_WIDTH = 4

    def __init__(self, msg, diagno=None):
        self.msg = msg
        if diagno is not None:
            # passed in 0-indexed (for convenience),
            # but store in 1-indexed since SyntaxError does so
            self.diagno = (
                self.filename, self.lineno, self.offset, self.length, self.text
            ) = diagno
            if isinstance(self.lineno, int):
                self.lineno += 1
            if isinstance(self.offset, int):
                self.offset += 1
        else:
            self.diagno = None
            (
                self.filename, self.lineno, self.offset, self.length, self.text
            ) = (None for i in range(5))

        if self.msg and self.msg[0].isupper():
            print(
                'Warning: this should start with a lowercase (maybe)',
                file=sys.stderr
            )

        self.diag_type = None
        self.color_seq = None

    def __repr__(self):
        if self.diagno is None:
            return '{}({!r},)'.format(self.CLASSNAME, self.msg)
        else:
            return '{}({!r}, {!r})'.format(
                self.CLASSNAME, self.msg, self.diagno
            )

    def __str__(self):
        res = self.msg
        if isinstance(self.lineno, int):
            if isinstance(self.filename, str):
                res += ' ({}, line {})'.format(self.filename, self.lineno)
            else:
                res += ' (line {})'.format(self.lineno)
        else:
            if isinstance(self.filename, str):
                res += ' ({})'.format(self.filename)

        return res

    def diagnose(self):
        """Return the diagnostic message.

        The message contains the filename and where the diagnostics are
        issued, with colored underlines."""
        if self.lineno is None:
            res = self.BOLD_SEQ + self.filename + ':'
        elif self.offset is None:
            res = self.BOLD_SEQ + '{}:{}:'.format(
                self.filename, self.lineno
            )
        else:
            res = self.BOLD_SEQ + '{}:{}:{}:'.format(
                self.filename, self.lineno, self.offset
            )
        res += (
            self.RESET_SEQ + ' '
            + self.color_seq + self.diag_type + ': '
            + self.RESET_SEQ + self.msg
        )

        if self.text is None:
            return res

        res += '\n' + ' '*self.INDENT_WIDTH
        if self.offset is None or self.length is None:
            res += self.text
            return res

        offset = self.offset-1
        length = self.length
        res += (
            self.text[:offset]
            + self.color_seq + self.text[offset:offset+length]
            + self.RESET_SEQ + self.text[offset+length:]
        )

        # NOTE: keep tabs for alignment
        # XXX but we do not know about tab stops
        eoft = len(('    '+self.text[:offset]).expandtabs())
        wlen = len(('    '+self.text[:offset+length]).expandtabs())-eoft
        res += (
            '\n' + ' '*self.INDENT_WIDTH
            + re.sub(r'\S', ' ', self.text[:offset])
            + self.color_seq + '^' + '~'*(wlen-1)
            + self.RESET_SEQ
        )
        return res


class MarkdownSyntaxError(MarkdownDiagnostic):
    """Error raised for syntax erros."""
    CLASSNAME = 'MarkdownSyntaxError'

    def __init__(self, msg, diagno=None, note=None):
        MarkdownDiagnostic.__init__(self, msg, diagno)
        self.note = note
        self.diag_type = 'error'
        self.color_seq = '\x1b[1;31m'


class MarkdownSyntaxWarning(MarkdownDiagnostic):
    """Error raised for syntax which is a probably mistake."""
    CLASSNAME = 'MarkdownSyntaxWarning'

    def __init__(self, msg, diagno=None):
        MarkdownDiagnostic.__init__(self, msg, diagno)
        self.diag_type = 'warning'
        self.color_seq = '\x1b[1;35m'


class MarkdownSyntaxNote(MarkdownDiagnostic):
    """Note for syntax errors and warnings."""
    CLASSNAME = 'MarkdownSyntaxNote'

    def __init__(self, msg, diagno=None):
        MarkdownDiagnostic.__init__(self, msg, diagno)
        self.diag_type = 'note'
        self.color_seq = '\x1b[1;36m'


# ---- Markdown elements ------------
class MarkdownElement(object):
    """Superclass for elements on markdown text."""
    def __init__(self, lines_withno, footnotes, filename, **kwargs):
        self._lines_withno = lines_withno
        self._footnotes = footnotes
        self._filename = filename
        self._kwargs = kwargs

        self._parsed = []
        self._parse()

    def __len__(self):
        if len(self._parsed) == 1 and not self._parsed[0][1]:
            return 0

        return len(self._parsed)

    def _parse(self):
        """Parse given line(s) and store them in a form that can be easy
        to translate several formats: LaTeX, HTML, etc."""
        # implemented in subclasses
        raise NotImplementedError

    def to_latex(self):
        """Translate the parsed data to LaTeX."""
        # implemented in subclasses
        raise NotImplementedError

    def to_html(self):
        """Translate the parsed date to HTML."""
        # implemented in subclasses
        raise NotImplementedError

    def translate(self, fmt='LaTeX'):
        """Translate the parsed data to given format."""
        if fmt in ('latex', 'LaTeX'):
            return self.to_latex()

        raise KeyError(fmt)


class Control(MarkdownElement):
    def _parse(self):
        self._parsed = self._lines_withno[0][1]

    def to_latex(self):
        if self._parsed == '\f':  # ^L
            return '\\newpage\n'
        elif self._parsed == '\f\f':
            return '\\clearpage\n'
        else:
            raise ValueError(self._parsed)

    def to_html(self):
        # In future, a separating line is added
        return '\n'


class SpecialText(MarkdownElement):
    PAGE_RE = re.compile(
        r"""
        (?P<PAGES>\b[Pp]p?\.\ ?(?P<PP_BEGIN>\d+)(?P<PP_DASH>-*)(?P<PP_END>\d*))
        """, flags=re.VERBOSE
    )
    FIGURE_RE = re.compile(
        r"""(?P<FIGURE>\b[Ff]ig\.\ ?(?P<FIG_NO>\d+))""",
        flags=re.VERBOSE
    )
    ABBREV_RE = re.compile(
        r"""\b(?P<ABBREV>e\.g\.|cf\.|i\.e\.)\ """,
        flags=re.VERBOSE
    )
    ELLIPSIS_RE = re.compile(r'(?P<ELLIPSIS>\.\.\.)', flags=re.VERBOSE)
    # We may fail: "*a ,*b", "a\,b"
    COMMA_RE = re.compile(
        r"""
        (?P<COMMA>\s*(?P<DOT>[,;:.])\s*)
        """, flags=re.VERBOSE
    )

    RE_S = (PAGE_RE, FIGURE_RE, ABBREV_RE, ELLIPSIS_RE, COMMA_RE)
    RE = '|'.join(prog.pattern for prog in RE_S)
    TYPES = (PAGE, FIGURE, ABBREV, ELLIPSIS, COMMA) = range(len(RE_S))

    def _parse(self):
        lineno, line = self._lines_withno[0]
        offset = self._kwargs['offset']
        for prog, type_ in zip(self.RE_S, self.TYPES):
            m = prog.match(line, pos=offset)
            if m is not None:
                self._type = type_
                self._m = m
                self._nc = line[m.end():m.end()+1]
                return

    def to_latex(self):
        m = self._m
        if self._type == self.PAGE:
            if m.group('PP_END'):
                return '{}p.~{}--{}'.format(
                    m.group()[:1], m.group('PP_BEGIN'), m.group('PP_END')
                )
            elif m.group('PP_DASH'):
                return '{}p.~{}---'.format(m.group()[:1], m.group('PP_BEGIN'))
            else:
                return '{}.~{}'.format(m.group()[:1], m.group('PP_BEGIN'))

        elif self._type == self.FIGURE:
            return '{}.~{}'.format(m.group()[:3], m.group('FIG_NO'))

        elif self._type == self.ABBREV:
            return '{}\\ '.format(m.group('ABBREV'))

        elif self._type == self.ELLIPSIS:
            # surrounding spaces needed?
            return '{\\ldots}'

        elif self._type == self.COMMA:
            res = m.group('DOT')
            nc = self._nc
            if nc and re.match('\w', nc):
                res += ' '
            return res

    def to_html(self):
        m = self._m
        if self._type == self.PAGE:
            if m.group('PP_END'):
                return '{}p. {}--{}'.format(
                    m.group()[:1], m.group('PP_BEGIN'), m.group('PP_END')
                )
            elif m.group('PP_DASH'):
                return '{}p. {}---'.format(m.group()[:1], m.group('PP_BEGIN'))
            else:
                return '{}. {}'.format(m.group()[:1], m.group('PP_BEGIN'))

        elif self._type == self.FIGURE:
            return '{}. {}'.format(m.group()[:3], m.group('FIG_NO'))

        elif self._type == self.ABBREV:
            return '{} '.format(m.group('ABBREV'))

        elif self._type == self.ELLIPSIS:
            # surrounding spaces needed?
            return '...'

        elif self._type == self.COMMA:
            res = m.group('DOT')
            nc = self._nc
            if nc and re.match('\w', nc):
                res += ' '
            return res


class Text(MarkdownElement):
    (
        TEXT_TYPE, SECTION_TYPE, TABLE_TYPE, ITEMENUM_TYPE, FOOTNOTE_TYPE,
        TITLE_TYPE, AUTHOR_TYPE
    ) = range(7)
    (
        NORMAL, ESCAPED, SEPARATOR, NEED_ESCAPE, QUOTES, FOOTNOTE,
        BOLD, ITALIC, AT_CMD, VERBATIM, LATEX, MATH, OTHER
        # XXX small-caps
    ) = [(1 << i) for i in range(13)]
    OPEN, CLOSE = range(2)
    RE = re.compile(
        r"""
        \\(?P<ESCAPED>.)
        | (?P<NEED_ESCAPE>[#%{}^~<>])
        | (?P<QUOTES>[\"\'])
        | \[\^(?P<FOOTNOTE>[\w-]+)\]  # NOTE: "[^ ... $" is invalid
        | (?P<BOLD>\*+)
        | (?P<ITALIC>_+)
        | (?P<VERBATIM>`+)
        | (?P<AT_CMD>@\[)
        | (?P<AT_CLOSE>@)
        | (?P<LATEX>&+)
        | (?P<MATH>\$+)
        | (?P<SPECIAL>"""+SpecialText.RE+')', flags=re.VERBOSE
    )
    AT_CMD_RE = re.compile(r"""\[(?P<NAME>[\w:#-]+)\]""")


    def __init__(
            self, lines_withno, footnotes, filename, sep=None, offset=0,
            type_=TEXT_TYPE
    ):
        MarkdownElement.__init__(
            self, lines_withno, footnotes, filename, sep=sep, offset=offset,
            type_=type_
        )
        self.cmd = None

    @classmethod
    def re_escape(cls, ch):
        """Escape a given character in order to use it in a regular
        expression."""
        return '\\'+ch if ch in '+*?|^$[](){}\\' else ch

    @classmethod
    def escape_char(cls, ch):
        """Escape a given character in order to use it in LaTeX text."""
        # ??? Which is better: '\\Foo{}' or '{\\Foo}'
        return {
            '#': r'\#',
            '$': r'\$',
            '%': r'\%',
            '&': r'\&',
            '~': r'\char`\~{}',
            '^': r'\char`\^{}',
            '_': r'\_',
            '{': r'\{',
            '}': r'\}',
            '\\': r'\char`\\{}',
            '-': '{-}',
            '<': '{<}',
            '>': '{>}',
            ',': '{,}',
            "'": r'\textquotesingle{}',
            '`': r'\char0{}',
            ' ': '{ }',
        }.get(ch, ch)

    @classmethod
    def escape(cls, src, cmd=[]):
        """Escape a given string in order to use it in LaTeX text."""

        # complexity: at least linear in size of cmd :(
        tt_in_cmd = ('tt' in cmd)

        ESC_RE = re.compile(
            (
                r"(?P<NEED_ESCAPE>" + (
                    r"[#$%^{}_~'`]" if tt_in_cmd else
                    r"[#$%^{}_~]"
                ) + ")"
            ) + r"""
            | \\(?P<ESCAPED>.)
            """, flags=re.VERBOSE
        )
        dst = ''
        m = ESC_RE.search(src)
        pos = 0
        while m is not None:
            spc = m.group()
            start, end = m.span()
            len_ = end-start
            dst += src[pos:start]
            pos = end

            if m.start('ESCAPED') > -1:
                if tt_in_cmd and spc[-1] == ' ':
                    dst += r'\char32{}'
                else:
                    dst += cls.escape_char(spc[-1])
            elif m.start('NEED_ESCAPE') > -1:
                dst += cls.escape_char(spc[-1])

            m = ESC_RE.search(src, pos=pos)

        dst += src[pos:]
        return dst

    @classmethod
    def verbatim(cls, src):
        """Simulate the verb command in LaTeX.  We do not want to use it
        because it is fragile and it requires us to search for a
        character which is not in given string.  If we have to escape
        all of the characters we can use, then how should we do?"""
        VERB_RE = re.compile(
            r"""
            (?P<NEED_ESCAPE>[#%$&^~{}_\\]+|\ \ +)
            | (?P<STOPS>[!?.:;]\ )
            | (?P<QUOTES>[\'\`])
            | (?P<LIGATURES>--|,,|<<|>>)
            """, flags=re.VERBOSE
        )
        dst = ''
        pos = 0
        m = VERB_RE.search(src)
        while m is not None:
            dst += src[pos:m.start()]
            ch = m.group()
            if m.start('NEED_ESCAPE') > -1:
                dst += ''.join(cls.escape_char(c) for c in ch)
            elif m.start('QUOTES') > -1:
                if ch == "'":
                    dst += r'\textquotesingle{}'
                elif ch == '`':
                    dst += r'\char0{}'
            elif m.start('LIGATURES') > -1:
                dst += ch[0]+'{'+ch[1]+'}'
            elif m.start('STOPS') > -1:
                dst += ch[0]+'\\ '

            pos = m.end()
            m = VERB_RE.search(src, pos=pos)

        dst += src[pos:]
        return dst

    def _parse(self):
        sep = self._kwargs['sep']
        spc_re = re.compile(
            r'(?P<SEPARATOR>'+self.re_escape(sep)+')|'+self.RE.pattern,
            flags=re.VERBOSE
        ) if sep is not None else self.RE

        lineno, line = self._lines_withno[0]
        self._parsed = []
        self._seplist = []
        offset = self._kwargs['offset']
        quoted = [False, False]  # single, double
        emphs = []  # [(emph_type, offset), ...]
        markers = {self.BOLD: 0, self.ITALIC: 0}
        spc_m = spc_re.search(line, pos=offset)
        at_flags = []
        at_offset = []

        while spc_m is not None:
            start, end = spc_m.span()
            len_ = end-start
            self._parsed.append((self.NORMAL, line[offset:start]))
            offset = end
            if sep is not None and spc_m.start('SEPARATOR') > -1:
                self._parsed.append((self.SEPARATOR, sep))
                self._seplist.append(start)

            elif spc_m.start('ESCAPED') > -1:
                self._parsed.append((self.ESCAPED, spc_m.group('ESCAPED')))

            elif spc_m.start('NEED_ESCAPE') > -1:
                self._parsed.append(
                    (self.NEED_ESCAPE, spc_m.group('NEED_ESCAPE'))
                )

            elif spc_m.start('QUOTES') > -1:
                quot = spc_m.group()
                if quot == "'":
                    self._parsed.append((self.QUOTES, ('`', "'")[quoted[0]]))
                    quoted[0] = 1-quoted[0]
                elif quot == '"':
                    self._parsed.append((self.QUOTES, ('``', "''")[quoted[1]]))
                    quoted[1] = 1-quoted[1]

            elif spc_m.start('FOOTNOTE') > -1:
                if self._footnotes is None:
                    raise MarkdownSyntaxError(
                        'nested footnote',
                        (self._filename, lineno, start, len_, line)
                    )

                label = spc_m.group('FOOTNOTE')
                if label not in self._footnotes:
                    raise MarkdownSyntaxError(
                        'undefined footnote label',
                        (self._filename, lineno, start+2, len_-3, line)
                    )

                self._parsed.append((self.FOOTNOTE, label))

            elif spc_m.start('SPECIAL') > -1:
                self._parsed.append(
                    (
                        self.OTHER, SpecialText(
                            [(lineno, line)], self._footnotes, self._filename,
                            offset=start
                        )
                    )
                )

            elif spc_m.start('AT_CMD') > -1:
                # parse @[here]...@
                m_at = self.AT_CMD_RE.match(line, pos=offset-1)
                if m_at is None:
                    raise MarkdownSyntaxError(
                        '"[" function-name "]" is expected',
                        (self._filename, lineno, offset-2, len_, line)
                    )

                offset = m_at.end()
                name = m_at.group('NAME')
                self._parsed.append((self.AT_CMD, name))
                at_flags.append(name)
                at_offset.append(start)

                if name == 'align':
                    prog = re.compile(r'\\.|[^@\\]+')
                    m_al = prog.match(line, pos=offset)
                    tmp = ''
                    while m_al is not None:
                        g = m_al.group()
                        tmp += '@' if g == '\\@' else g

                        offset = m_al.end()
                        m_al = prog.match(line, pos=offset)

                    self._parsed.append((self.LATEX, tmp))
                    if offset < len(line) and line[offset] == '@':
                        at_flags.pop()
                        at_offset.pop()
                        self._parsed.append((self.AT_CMD, self.CLOSE))
                        offset += 1
                    else:
                        raise MarkdownSyntaxError(
                            'unclosed @[align]',
                            (self._filename, lineno, at_offset[-1], 8, line)
                        )

            elif spc_m.start('AT_CLOSE') > -1:
                if not at_flags:
                    raise MarkdownSyntaxError(
                        'invalid @ command',
                        (self._filename, lineno, offset-1, 1, line)
                    )

                at_flags.pop()
                at_offset.pop()
                self._parsed.append((self.AT_CMD, self.CLOSE))

            elif (
                    spc_m.start('BOLD') > -1
                    or spc_m.start('ITALIC') > -1
            ):
                kind = (
                    self.BOLD if spc_m.start('BOLD') > -1 else
                    self.ITALIC
                )
                if 0 < markers[kind] <= len_:
                    # already emphasized with current marker
                    if emphs[-1][0] != kind:
                        oft = [e[1] for e in emphs if e[0] == kind][0]
                        raise MarkdownSyntaxError(
                            'ill-formed nesting',
                            (self._filename, lineno, oft, end-oft, line)
                        )

                    self._parsed.append((kind, self.CLOSE))
                    if markers[kind] < len_:
                        print(
                            MarkdownSyntaxWarning(
                                'weird emphasis marker',
                                (
                                    self._filename, lineno,
                                    start+markers[kind], len_-markers[kind],
                                    line
                                )
                            ).diagnose(), file=sys.stderr
                        )
                        print(
                            MarkdownSyntaxNote(
                                'previous emphasis here',
                                (
                                    self._filename, lineno, emphs[-1][1],
                                    start+markers[kind]-emphs[-1][1], line
                                )
                            ).diagnose(), file=sys.stderr
                        )
                        emphs.pop()
                        emphs.append((kind, start+markers[kind]))
                        self._parsed.append((kind, self.OPEN))
                        markers[kind] = len_-markers[kind]

                    else:
                        emphs.pop()
                        markers[kind] = 0

                elif len_ < markers[kind]:
                    self._parsed.append((self.NORMAL, spc_m.group()))

                else:
                    # not emphasized with current marker
                    emphs.append((kind, start))
                    self._parsed.append((kind, self.OPEN))
                    markers[kind] = len_

            else:
                marker = spc_m.group()
                if marker[0] == '$' and emphs:
                    note = MarkdownSyntaxNote(
                        'previous marker opened here',
                        (
                            self._filename, lineno, emphs[-1][1],
                            markers[emphs[-1][0]], line
                        )
                    )
                    raise MarkdownSyntaxError(
                        '$ is not allowed in marked context',
                        (self._filename, lineno, offset-1, 1, line),
                        note=note
                    )

                next_ = line.find(marker, end)
                if next_ == -1:
                    raise MarkdownSyntaxError(
                        'unclosed '+marker,
                        (self._filename, lineno, start, len_, line)
                    )

                kind = (
                    self.VERBATIM if marker[0] == '`' else
                    self.LATEX if marker[0] == '&' else
                    self.MATH if marker[0] == '$' else
                    None  # for future extension
                )

                if next_+len_ < len(line) and line[next_+len_] == marker[0]:
                    # we want to warn weird markers
                    prog = re.compile(self.re_escape(marker[0])+'+')
                    m = prog.match(line, pos=next_+len_)
                    ml = m.end()-m.start()
                    print(
                        MarkdownSyntaxWarning(
                            'weird marker',
                            (self._filename, lineno, next_+len_, ml, line)
                        ).diagnose(), file=sys.stderr
                    )
                    print(
                        MarkdownSyntaxNote(
                            'previous marker here',
                            (
                                self._filename, lineno, start,
                                next_-start+len_, line
                            )
                        ).diagnose(), file=sys.stderr
                    )

                subj = line[end:next_]
                if subj.lstrip(' ')[:1] == marker[0]:
                    # [:1] for avoiding IndexError
                    subj = subj[1:]
                if subj.rstrip(' ')[-1:] == marker[0]:
                    # [-1:] for avoiding IndexError
                    subj = subj[:-1]

                self._parsed.append((kind, subj))
                offset = next_+len_

            spc_m = spc_re.search(line, pos=offset)

        if emphs:
            raise MarkdownSyntaxError(
                'unclosed emphasis marker',
                (
                    self._filename, lineno, emphs[-1][1],
                    markers[emphs[-1][0]], line
                )
            )

        if at_flags:
            raise MarkdownSyntaxError(
                'unclosed @ command',
                (
                    self._filename, lineno, at_flags[-1],
                    len(at_offset[-1])+3, line
                )
            )

        self._parsed.append((self.NORMAL, line[offset:]))

    def to_latex(self, sep=None):
        res = ''
        count_seps = 0
        num_seps = self.num_seps()
        emph_flags = 0  # EMPH_TYPE
        at_flags = []
        for kind, snippet in self._parsed:
            if kind == self.NORMAL:
                res += self.escape(snippet, at_flags)

            elif kind == self.ESCAPED:
                if snippet == '&':
                    res += r'\&'
                elif snippet == '\\':
                    res += r'\char`\\{}'
                else:
                    res += self.escape('\\'+snippet, at_flags)

            elif kind == self.SEPARATOR:
                if sep is None:
                    raise ValueError(sep)

                count_seps += 1
                if 1 < count_seps < num_seps:
                    res += sep

            elif kind == self.NEED_ESCAPE:
                res += self.escape(snippet, at_flags)

            elif kind == self.QUOTES:
                res += '{'+snippet+'}'

            elif kind == self.FOOTNOTE:
                res += self._footnotes[snippet].to_latex(
                    type_=self._kwargs['type_']
                )

            elif kind in (self.BOLD, self.ITALIC):
                if emph_flags & kind:
                    res += '}'
                else:
                    res += (
                        '\\textbf{' if kind == self.BOLD else
                        '\\textit{' if kind == self.ITALIC else
                        None
                    )

                emph_flags ^= kind

            elif kind == self.AT_CMD:
                if snippet == self.CLOSE:
                    if at_flags[-1] == 'align':
                        res += '\\]'
                    else:
                        res += '}'
                    at_flags.pop()
                    continue

                at_flags.append(snippet)

                if snippet == 'tt':
                    res += '\\texttt{'
                elif snippet == 'sc':
                    res += '\\textsc{'
                elif snippet == 'align':
                    res += '\\['
                elif snippet.startswith('color:#'):
                    r, g, b = map(
                        lambda n: int(n, 16),
                        [snippet[i:i+2] for i in range(7, 12, 2)]
                    )
                    res += '{{\\color[rgb]{{{:.2f}, {:.2f}, {:.2f}}}'.format(
                        r/255, g/255, b/255
                    )
                elif snippet.startswith('color:'):
                    res += '{{\\color{{{}}}'.format(snippet[6:])
                elif snippet in ('TeX', 'LaTeX', 'LaTeXe'):
                    res += '\\' + snippet + '{'
                else:
                    assert False

            elif kind == self.VERBATIM:
                res += '\\texttt{'+self.verbatim(snippet)+'}'

            elif kind == self.LATEX:
                res += snippet

            elif kind == self.MATH:
                res += '$'+snippet+'$'

            elif kind == self.OTHER:
                res += snippet.to_latex()

            else:
                raise KeyError(kind)

        if self._kwargs['type_'] == self.TEXT_TYPE:
            res += '\n'
        elif self._kwargs['type_'] == self.TABLE_TYPE:
            res += '\\\\\n'

        return res

    def to_html(self, sep=None):
        res = ''
        count_seps = 0
        num_seps = self.num_seps()
        emph_flags = 0  # EMPH_TYPE
        at_flags = []
        NAMED_ENTITIES = {
            '&': '&amp;',
            '"': '&quot;',
            "'": '&apos;',
            '<': '&lt;',
            '>': '&gt;',
        }

        for kind, snippet in self._parsed:
            if kind == self.NORMAL:
                snippet = snippet.replace('&', '&amp;')
                for ch in '"\'<>':
                    snippet = snippet.replace(ch, NAMED_ENTITIES[ch])
                res += snippet

            elif kind == self.ESCAPED:
                res += NAMED_ENTITIES.get(snippet, '&#'+str(ord(snippet))+';')

            elif kind == self.SEPARATOR:
                if sep is None:
                    raise ValueError(sep)

                count_seps += 1
                if 1 < count_seps:
                    res += '</' + sep + '>'
                if count_seps < num_seps:
                    res += '<' + sep + '>'

            elif kind == self.NEED_ESCAPE:
                res += NAMED_ENTITIES.get(snippet, snippet)

            elif kind == self.QUOTES:
                res += NAMED_ENTITIES[snippet]

            elif kind == self.FOOTNOTE:
                res += self._footnotes[snippet].to_html(
                    type_=self._kwargs['type_']
                )

            elif kind in (self.BOLD, self.ITALIC):
                tag = (
                    'strong' if kind == self.BOLD else
                    'em' if kind == self.ITALIC else
                    None
                )
                if emph_flags & kind:
                    res += '</' + tag + '>'
                else:
                    res += '<' + tag + '>'

                emph_flags ^= kind

            elif kind == self.AT_CMD:
                if snippet == self.CLOSE:
                    res += '</span>'
                    at_flags.pop()
                    continue

                at_flags.append(snippet)
                res += '<span at_name="' + snippet+ '">'

                # if snippet == self.CLOSE:
                #     if at_flags[-1] == 'align':
                #         res += '\\]'
                #     else:
                #         res += '}'
                #     at_flags.pop()
                #     continue

                # at_flags.append(snippet)

                # if snippet == 'tt':
                #     res += '\\texttt{'
                # elif snippet == 'sc':
                #     res += '\\textsc{'
                # elif snippet == 'align':
                #     res += '\\['
                # elif snippet.startswith('color:#'):
                #     r, g, b = map(
                #         lambda n: int(n, 16),
                #         [snippet[i:i+2] for i in range(7, 12, 2)]
                #     )
                #     res += '{{\\color[rgb]{{{:.2f}, {:.2f}, {:.2f}}}'.format(
                #         r/255, g/255, b/255
                #     )
                # elif snippet.startswith('color:'):
                #     res += '{{\\color{{{}}}'.format(snippet[6:])
                # elif snippet in ('TeX', 'LaTeX', 'LaTeXe'):
                #     res += '\\' + snippet + '{'
                # else:
                #     assert False

            elif kind == self.VERBATIM:
                res += '<code>'
                snippet = snippet.replace('&', '&amp;')
                for ch in '"\'<>':
                    snippet = snippet.replace(ch, NAMED_ENTITIES[ch])
                res += snippet
                res += '</code>'

            elif kind == self.LATEX:
                raise NotImplementedError

            elif kind == self.MATH:
                res += '$'+snippet+'$'

            elif kind == self.OTHER:
                res += snippet.to_html()

            else:
                raise KeyError(kind)

        res += '\n'
        return res

    def num_seps(self):
        return len([
            None for type_, content in self._parsed if type_ == self.SEPARATOR
        ])


class Footnote(MarkdownElement):
    RE = re.compile(r'#\[\^(?P<LABEL>[\w-]+)\]: ?(?P<TEXT>.*)')

    def __init__(self, lines_withno, footnotes, filename):
        MarkdownElement.__init__(self, lines_withno, footnotes, filename)
        self._is_used = False

    def _parse(self):
        lineno, line = self._lines_withno[0]
        m = self.RE.match(line)
        self._footnotetext = Text(
            [(lineno, line)], None, self._filename, offset=m.start('TEXT'),
            type_=Text.FOOTNOTE_TYPE
        )

    def to_latex(self, type_=None):
        self._is_used = True
        if type_ == Text.SECTION_TYPE:
            return '\\protect\\footnote{'+self._footnotetext.to_latex()+'}'
        elif type_ == Text.TABLE_TYPE:
            return '\\tablefootnote{'+self._footnotetext.to_latex()+'}'
        elif type_ == Text.ITEMENUM_TYPE:
            return (
                '\\begin{footnote}'
                + self._footnotetext.to_latex()
                + '\\end{footnote}'
            )
        elif type_ == Text.AUTHOR_TYPE:
            return '\\thanks{'+self._footnotetext.to_latex()+'}'
        elif type_ in (Text.TEXT_TYPE, Text.TITLE_TYPE):
            # ??? for titles and authors, other results may be desired
            return '\\footnote{'+self._footnotetext.to_latex()+'}'
        else:
            raise KeyError(type_)

    def to_html(self, type_=None):
        res = '<span class="footnote">'
        res += self._footnotetext.to_html()
        res += '</span>'

    @property
    def used(self):
        """Return whether its label is referenced in the document."""
        return self._is_used

    def __lt__(self, other):
        """Compare them based on the position they appear.  Eariler one
        is considered to be less."""
        return self._lines_withno[0][0] < other._lines_withno[0][0]


class Title(MarkdownElement):
    RE = re.compile(r'#(?P<COLON>:+) ?')

    def _parse(self):
        self._parsed = []  # [(colon, Text), ...]
        for lineno, line in self._lines_withno:
            m = self.RE.match(line)
            self._parsed.append(
                (
                    m.end('COLON')-m.start('COLON'), Text(
                        [(lineno, line)], self._footnotes, self._filename,
                        offset=m.end(), type_=Text.TITLE_TYPE
                    )
                )
            )

    def to_latex(self):
        SIZE = (
            None, r'{\Huge ', r'{\huge ', r'{\LARGE ', r'{\Large ',
            r'{\large ', r'{\normalsize ', r'{\small ', r'{\footnotesize ',
            r'{\scriptsize ', r'{\tiny ',
        )
        res = '\\title{\n'
        for i, (colon, text) in enumerate(self._parsed):
            res += '  '+SIZE[colon]+text.to_latex()+'}'
            if i+1 < len(self._parsed):
                res += r'\\'

            res += '\n'

        res += '}\n'
        return res

    def to_html(self):
        res = ''
        for i, (colon, text) in enumerate(self._parsed):
            res += text.to_html()

        return '<title>' + res.strip() + '</title>'


class Author(MarkdownElement):
    RE = re.compile(r'#@ ?')

    def _parse(self):
        self._parsed = []
        for lineno, line in self._lines_withno:
            m = self.RE.match(line)
            self._parsed.append(
                Text(
                    [(lineno, line)], self._footnotes, self._filename,
                    offset=m.end(), type_=Text.AUTHOR_TYPE
                )
            )

    def to_latex(self):
        res = '\\author{\n'
        for i, text in enumerate(self._parsed):
            res += '  '+text.to_latex()
            if i+1 < len(self._parsed):
                res += r'\\'

            res += '\n'

        res += '}\n'
        return res

    def to_html(self):
        return ''  # XXX


class Section(MarkdownElement):
    RE = re.compile(r'(?P<HASH>#+) (?P<TEXT>.+)')

    def _parse(self):
        lineno, line = self._lines_withno[0]
        m = self.RE.match(line)
        assert m is not None
        self._level = m.end('HASH')-1
        self._text = Text(
            self._lines_withno, self._footnotes, self._filename,
            offset=m.start('TEXT'), type_=Text.SECTION_TYPE
        )

    def to_latex(self):
        SECTIONS = (r'\section', r'\subsection', r'\subsubsection')
        if self._level >= len(SECTIONS):
            raise MarkdownSyntaxError(
                'too low level section',
                (
                    self._filename, self._lines_withno[0][0], 0, self._level+1,
                    self._lines_withno[0][1]
                )
            )

        return SECTIONS[self._level]+'{'+self._text.to_latex()+'}\n'

    def to_html(self):
        return '<h{0}>{1}</h{0}>\n'.format(
            self._level+1, self._text.to_html().strip()
        )


class TableDelimiter(MarkdownElement):
    RE = re.compile(r'\s*(?:(?P<PIPE>\|+)|(?P<SEP>[-=:]-+[-=:]))\s*')
    PIPE, LEFT, CENTER, RIGHT = range(4)

    def _parse(self):
        self._parsed = []
        lineno, line = self._lines_withno[0]
        offset = 0
        m = self.RE.match(line)
        while m is not None:
            if m.group('PIPE') is not None:
                self._parsed.append(self.PIPE)
            else:
                sep = m.group('SEP')
                if sep[0] == sep[-1]:
                    self._parsed.append(self.CENTER)
                elif sep[0] == ':':
                    self._parsed.append(self.LEFT)
                else:
                    self._parsed.append(self.RIGHT)

            offset = m.end()
            m = self.RE.match(line, pos=offset)

        if offset != len(line):
            raise MarkdownSyntaxError(
                'ill-formed delimiter row',
                (self._filename, lineno, offset, len(line)-offset, line)
            )

        if not self.num_columns():
            print(
                MarkdownSyntaxWarning(
                    'no alignment specified',
                    (self._filename, lineno, 0, len(line), line)
                ).diagnose(), file=sys.stderr
            )
            print(
                MarkdownSyntaxNote(
                    'you may use :--, :-:, --:, or ---',
                    (self._filename, None, None, None, None)
                ).diagnose(), file=sys.stderr
            )

    def to_latex(self):
        return ''.join('|lcr'[k] for k in self._parsed)

    def to_html(self):
        return ''

    def num_columns(self):
        """Return the number of columns that its contains."""
        return len([s for s in self._parsed if s != self.PIPE])


class TableRow(Text):
    def __init__(self, lines_withno, footnotes, filename, sep='|', offset=0):
        Text.__init__(
            self, lines_withno, footnotes, filename, sep=sep, offset=offset,
            type_=Text.TABLE_TYPE
        )


class Table(MarkdownElement):
    RE = re.compile(
        r'(?P<THIN>-{3,}\s*$)|(?P<THICK>=\+{3,}\s*$)|(?P<PIPE>\|)'
    )
    HLINE, = range(1)

    def _parse(self):
        self._parsed = []
        lines_withno = iter(self._lines_withno)

        # head row
        for lineno, line in lines_withno:
            # loop until actual head row appears
            m = self.RE.match(line)
            if m is None:
                raise MarkdownSyntaxError(
                    'ill-formed table',
                    (self._filename, lineno, 0, len(line), line)
                )

            if m.start('THIN') > -1:
                self._parsed.append(self.HLINE)
            elif m.start('THICK') > -1:
                self._parsed.append(self.HLINE)
                self._parsed.append(self.HLINE)
            else:
                head_lineno, head_line = lineno, line
                head = TableRow(
                    [(lineno, line)], self._footnotes, self._filename, sep='|'
                )
                self._parsed.append(head)
                break
        else:
            raise MarkdownSyntaxError(
                'delimiter row not found',
                (self._filename, self._lines_withno[-1][0], None, None, None)
            )

        # delimiter row
        try:
            del_lineno, del_line = next(lines_withno)
        except StopIteration:
            raise MarkdownSyntaxError(
                'unexpected EOF while looking for delimiter row',
                (self._filename, lineno, None, None, None)
            )

        m = TableDelimiter.RE.match(del_line)
        if m is None:
            raise MarkdownSyntaxError(
                'delimiter row must appear after the head row',
                (self._filename, del_lineno, 0, len(del_line), del_line)
            )

        self._delimiter = TableDelimiter(
            [(del_lineno, del_line)], self._footnotes, self._filename
        )
        num_columns = self._delimiter.num_columns()

        if num_columns != head.num_seps()-1:
            print(
                MarkdownSyntaxWarning(
                    'the number of columns mismatch',
                    (self._filename, head_lineno, 0, len(head_line), head_line)
                ).diagnose(), file=sys.stderr
            )
            print(
                MarkdownSyntaxNote(
                    'expected {}, but got {}'.format(
                        num_columns, head.num_seps()-1
                    ), (self._filename, del_lineno, 0, len(del_line), del_line)
                ).diagnose(), file=sys.stderr
            )

        self._parsed.append(self.HLINE)  # should be more decoratable

        # body
        for lineno, line in lines_withno:
            m = self.RE.match(line)
            if m is None:
                raise MarkdownSyntaxError(
                    'ill-formed table',
                    (self._filename, lineno, 0, len(line), line)
                )

            if m.start('THIN') > -1:
                self._parsed.append(self.HLINE)
            elif m.start('THICK') > -1:
                self._parsed.append(self.HLINE)
                self._parsed.append(self.HLINE)
            else:
                body = TableRow(
                    [(lineno, line)], self._footnotes, self._filename, sep='|'
                )
                self._parsed.append(body)
                if num_columns != body.num_seps()-1:
                    print(
                        MarkdownSyntaxWarning(
                            'the number of columns mismatch',
                            (self._filename, lineno, 0, len(line), line)
                        ).diagnose(), file=sys.stderr
                    )
                    print(
                        MarkdownSyntaxNote(
                            'expected {}, but got {}'.format(
                                num_columns, body.num_seps()-1
                            ), (
                                self._filename, del_lineno, 0, len(del_line),
                                del_line
                            )
                        ).diagnose(), file=sys.stderr
                    )

    def to_latex(self):
        res = '\\begin{table}[h]\n'
        res += '  \\centering\n'
        res += '  \\begin{tabular}{'+self._delimiter.to_latex()+'}\n'
        for elem in self._parsed:
            res += '    '
            if elem == self.HLINE:
                res += '\\hline\n'
                continue

            assert isinstance(elem, Text)
            res += elem.to_latex(sep=' & ')

        res += '  \\end{tabular}\n'
        res += '\\end{table}\n'
        return res

    def to_html(self):
        res = '<table class="table table-striped table-condensed">\n'

        it = iter(self._parsed)
        for tr in it:
            res += '  <thead>\n'
            res += '    <tr>\n'
            res += '      ' + tr.to_html(sep='th')
            res += '    </tr>\n'
            res += '  </thead>\n'
            break
        for i, tr in enumerate(it):
            if i == 0:
                res += '  <tbody>\n'

            if tr == self.HLINE:
                continue

            res += '    <tr>\n'
            res += '      ' + tr.to_html(sep='td')
            res += '    </tr>\n'
        else:
            res += '  </tbody>\n'

        res += '</table>\n'
        return res


class ItemEnum(MarkdownElement):
    # FIXME: what is the best way to indent verbose REs?
    RE = re.compile(
        r"""
        \s*(?:
        (?P<ITEM>[-+])
        | (?P<ENUM>[1-9]\d*\. | \(?[1-9]\d*\))
        )
        \ (?P<TEXT>.+)""",
        flags=re.VERBOSE
    )
    BEGIN_ITEM, BEGIN_ENUM, END_ITEM, END_ENUM = range(4)

    def _parse(self):
        self._parsed = []
        opened = []  # [(indent, kind), ...]
        for lineno, line in self._lines_withno:
            m = self.RE.match(line)
            if m is None:
                m = re.search(r'\S', line)
                raise MarkdownSyntaxError(
                    'ill-formed itemization/enumeration',
                    (
                        self._filename, lineno, m.start(), len(line)-m.start(),
                        line
                    )
                )

            kind, indent = (
                (self.BEGIN_ITEM, m.start('ITEM')) if m.start('ITEM') > -1
                else (self.BEGIN_ENUM, m.start('ENUM'))
            )
            if not opened or opened[-1][0] < indent:
                opened.append((indent, kind))
                self._parsed.append(kind)
            elif opened[-1][0] == indent:
                if opened[-1][1] != kind:
                    raise MarkdownSyntaxError(
                        'ill-formed itemization/enumeration (mismatch types)',
                        (
                            self._filename, lineno, indent,
                            m.end('TEXT')-indent, line)
                    )
            else:
                while opened and opened[-1][0] > indent:
                    tmp = opened.pop()
                    self._parsed.append(
                        self.END_ITEM if tmp[1] == self.BEGIN_ITEM
                        else self.END_ENUM
                    )
                else:
                    if not opened or opened[-1][0] < indent:
                        raise MarkdownSyntaxError(
                            'ill-formed indentation',
                            (
                                self._filename, lineno, indent,
                                m.start('TEXT')-(indent+1), line
                            )
                        )

            self._parsed.append(
                Text(
                    [(lineno, line)], self._footnotes, self._filename,
                    offset=m.start('TEXT'), type_=Text.ITEMENUM_TYPE
                )
            )

        while opened:
            tmp = opened.pop()
            self._parsed.append(
                self.END_ITEM if tmp[1] == self.BEGIN_ITEM else self.END_ENUM
            )

    def to_latex(self):
        indent = 0
        res = ''
        for elem in self._parsed:
            if elem == self.BEGIN_ITEM:
                res += '  '*indent + '\\begin{itemize}\n'
                indent += 1
            elif elem == self.BEGIN_ENUM:
                res += '  '*indent + '\\begin{enumerate}\n'
                indent += 1
            elif elem == self.END_ITEM:
                indent -= 1
                res += '  '*indent + '\\end{itemize}\n'
            elif elem == self.END_ENUM:
                indent -= 1
                res += '  '*indent + '\\end{enumerate}\n'
            else:
                assert isinstance(elem, Text)
                res += '  '*(indent-1) + '\\item ' + elem.to_latex() + '\n'

        return res

    def to_html(self):
        indent = 0
        res = ''
        for elem in self._parsed:
            if elem == self.BEGIN_ITEM:
                res += '  '*indent + '<ul>\n'
                indent += 1
            elif elem == self.BEGIN_ENUM:
                res += '  '*indent + '<ol>\n'
                indent += 1
            elif elem == self.END_ITEM:
                indent -= 1
                res += '  '*indent + '</ul>\n'
            elif elem == self.END_ENUM:
                indent -= 1
                res += '  '*indent + '</ol>\n'
            else:
                assert isinstance(elem, Text)
                res += '  '*indent + '<li>'+elem.to_html().strip()+'</li>\n'

        return res


class CodeBlock(MarkdownElement):
    PAT = '#`'

    def _parse(self):
        self._parsed = [line for lineno, line in self._lines_withno]

    def to_latex(self):
        # NOTE: care should be taken the case:
        #       "\end{lstlisting}" in self._parsed
        return (
            '\n  '.join(['\\begin{lstlisting}']+self._parsed)
            + '\n\\end{lstlisting}\n'
        )

    def to_html(self):
        return '<pre>' + '\n'.join(self._parsed) + '\n</pre>'


class ShellBlock(MarkdownElement):
    PAT = '#$'
    RE = re.compile(r'(?P<TOP>[$#>EIOeio]) ?(?P<TEXT>.*)')
    STDIN, STDOUT, STDERR, NEWLINE = range(4)
    IND = {'i': STDIN, 'o': STDOUT, 'e': STDERR}

    def _parse(self):
        self._parsed = []
        for lineno, line in self._lines_withno:
            m = self.RE.match(line)
            if m is None:
                raise MarkdownSyntaxError(
                    'ill-formed shell block',
                    (self._filename, lineno, 0, len(line), line)
                )

            ch, text = m.group('TOP', 'TEXT')
            if ch in '$#>':
                self._parsed.append((self.STDERR, ch+' '))
                self._parsed.append((self.STDIN, text))
                self._parsed.append((self.NEWLINE, '\n'))
            else:
                self._parsed.append((self.IND[ch.lower()], text))
                if ch.isupper():
                    self._parsed.append((self.NEWLINE, '\n'))

    def to_latex(self):
        res = '\\begin{framed}\n'
        res += '\\parskip=0ex\n'
        for type_, text in self._parsed:
            if type_ == self.STDIN:
                res += r'\texttt{'+Text.verbatim(text)+'}'
            if type_ == self.STDOUT:
                res += r'\textlt{'+Text.verbatim(text)+'}'
            if type_ == self.STDERR:
                res += r'{\color[named]{Gray}\textlt{'+Text.verbatim(text)+'}}'
            if type_ == self.NEWLINE:
                res += '\n\n'

        res += '\\end{framed}\n'
        return res

    def to_html(self):
        raise NotImplementedError


class RawText(MarkdownElement):
    PAT = '#&'

    def _parse(self):
        self._parsed = [line for lineno, line in self._lines_withno]

    def to_latex(self):
        return '\n'.join(self._parsed) + '\n'

    def to_html(self):
        return '\n'.join(self._parsed) + '\n'


# ---- Parser ------------
class MarkdownParser(object):
    COMMLINE_PAT = '#%'
    COMMBLOCKBEGIN_PAT, COMMBLOCKEND_PAT = '#{', '#}'

    def __init__(self, fin):
        self._fin = fin
        self._filename = fin.name

        # fin.seek(0, os.SEEK_SET)
        self._save_lines()

        # now, we do not touch self._fin anymore
        self._parse()

    def translate(self, fout, fmt='LaTeX', **kwargs):
        """Translate a file (given in __init__()) into given format and
        write it down in the file."""
        if fmt in ('latex', 'LaTeX'):
            self.to_latex(fout, **kwargs)
        elif fmt.lower() == 'latex':
            print(
                'You should write "LaTeX" (not {})'.format(fmt),
                file=sys.stderr
            )
        elif fmt.lower() == 'html':
            self.to_html(fout, **kwargs)
        else:
            raise NotImplementedError

    def to_latex(self, fout, **kwargs):
        """Translate a file (given in __init__) into LaTeX format and
        write it down in the file."""
        options = kwargs['options']
        if not options.get('--as-part', False):
            fout.write(kwargs['preamble'])

            if self._title is not None:
                fout.write(self._title.to_latex())

            if self._author is not None:
                fout.write(self._author.to_latex())

            fout.write('\\begin{document}\n')

            if self._title is not None:
                fout.write('\\maketitle\n')

        for elem in self._parsed:
            fout.write(elem.to_latex())

        if not options.get('--as-part', False):
            fout.write('\\end{document}\n')

        unused_footnotes = [
            (v, k) for k, v in self._footnotes.items() if not v.used
        ]
        unused_footnotes.sort()
        for footnote, label in unused_footnotes:
            print(
                MarkdownSyntaxWarning(
                    'defined but unused footnote',
                    (
                        self._filename, footnote._lines_withno[0][0], 0,
                        len(label)+4, footnote._lines_withno[0][1]
                    )
                ).diagnose(), file=sys.stderr
            )

    def to_html(self, fout, **kwargs):
        """Translate a file (given in __init__) into HTML format and
        write it down in the file."""
        options = kwargs['options']
        if not options.get('--as-part', False):
            fout.write('<!DOCTYPE html>\n')
            fout.write('<html>\n')

            fout.write('  <head>\n')
            fout.write(kwargs['preamble'])

            if self._title is not None:
                fout.write('    '+self._title.to_html()+'\n')

            if self._author is not None:
                fout.write('    '+self._author.to_html()+'\n')

            fout.write('  </head>\n')

        fout.write('  <body>\n')
        last = None
        for elem in self._parsed:
            last_text = (isinstance(last, Text) and last)
            elem_text = (isinstance(elem, Text) and elem)
            if last_text:
                if not elem_text:
                    fout.write('</p>\n')
            elif elem_text:
                fout.write('<p>')

            fout.write(elem.to_html())
            last = elem

        if isinstance(last, Text) and last:
            fout.write('</p>\n')

        fout.write('  </body>\n')
        fout.write('</html>\n')

        unused_footnotes = [
            (v, k) for k, v in self._footnotes.items() if not v.used
        ]
        unused_footnotes.sort()
        for footnote, label in unused_footnotes:
            print(
                MarkdownSyntaxWarning(
                    'defined but unused footnote',
                    (
                        self._filename, footnote._lines_withno[0][0], 0,
                        len(label)+4, footnote._lines_withno[0][1]
                    )
                ).diagnose(), file=sys.stderr
            )

    def _save_lines(self):
        """Save lines in the given file for parsing.  Now we ignore the
        comment block and treat footnotes specially."""
        self._lines = []  # [(lineno, line), ...]
        self._footnotes = {}  # {label: footnotetext, ...}

        e_fin = enumerate(self._fin)  # do this for nested loop
        for lineno, line in e_fin:
            line = line.rstrip(' \t\n')  # keep ^L
            if line.startswith(self.COMMLINE_PAT):
                continue

            if line.startswith(self.COMMBLOCKBEGIN_PAT):
                opened = 1
                pre_lineno = lineno
                pre_line = line
                for lineno, line in e_fin:
                    if line.startswith(self.COMMBLOCKBEGIN_PAT):
                        opened += 1
                    elif line.startswith(self.COMMBLOCKEND_PAT):
                        opened -= 1
                        if not opened:
                            break
                else:
                    if opened:
                        raise MarkdownSyntaxError(
                            'unclosed comment block',
                            (
                                self._filename, pre_lineno, 0, len(pre_line),
                                pre_line
                            )
                        )

                continue

            if line.startswith(RawText.PAT):
                pre_lineno = lineno
                pre_line = line
                self._lines.append((lineno, line))
                for lineno, line in e_fin:
                    line = line.rstrip()
                    self._lines.append((lineno, line))
                    if line == pre_line:
                        break

                else:
                    raise MarkdownSyntaxError(
                        'unclosed raw block',
                        (
                            self._filename, pre_lineno, 0, len(pre_line),
                            pre_line
                        )
                    )

                continue

            if line.startswith(CodeBlock.PAT):
                pre_lineno = lineno
                pre_line = line
                self._lines.append((lineno, line))
                for lineno, line in e_fin:
                    line = line.rstrip()
                    self._lines.append((lineno, line))
                    if line == pre_line:
                        break

                else:
                    raise MarkdownSyntaxError(
                        'unclosed code block',
                        (
                            self._filename, pre_lineno, 0, len(pre_line),
                            pre_line
                        )
                    )

                continue

            if line.startswith(ShellBlock.PAT):
                pre_lineno = lineno
                pre_line = line
                self._lines.append((lineno, line))
                for lineno, line in e_fin:
                    line = line.rstrip()
                    self._lines.append((lineno, line))
                    if line == pre_line:
                        break

                else:
                    raise MarkdownSyntaxError(
                        'unclosed shell block',
                        (
                            self._filename, pre_lineno, 0, len(pre_line),
                            pre_line
                        )
                    )

                continue                

            m = Footnote.RE.match(line)
            if m is not None:
                label = m.group('LABEL')
                if label in self._footnotes:
                    prev_line = self._footnotes[label]._lines_withno[0]
                    raise MarkdownSyntaxError(
                        'redefinition of footnote text',
                        (self._filename, lineno, 0, len(line), line),
                        MarkdownSyntaxNote(
                            'previously defined here',
                            (
                                self._filename, prev_line[0], 0, len(label)+4,
                                prev_line[1]
                            )
                        )
                    )

                self._footnotes[label] = Footnote(
                    [(lineno, line)], None, self._filename,
                )
                continue

            self._lines.append((lineno, line))

    def _parse(self):
        """Parse the contents in given file.  We do not parse with
        scanning the file, but with iterating the list of lines that is
        created with self._save_lines()."""
        self._parsed = []  # [element, ...]
        title = []
        author = []

        lines = iter(self._lines)
        for lineno, line in lines:
            if line in ('\f', '\f\f'):
                self._parsed.append(
                    Control([(lineno, line)], self._footnotes, self._filename)
                )
                continue

            if Section.RE.match(line):
                self._parsed.append(
                    Section([(lineno, line)], self._footnotes, self._filename)
                )
                continue

            if ItemEnum.RE.match(line):
                lines_withno = [(lineno, line)]
                for lineno, line in lines:
                    if not line.strip():
                        break
                    lines_withno.append((lineno, line))

                self._parsed.append(
                    ItemEnum(lines_withno, self._footnotes, self._filename)
                )
                continue

            if Table.RE.match(line):
                lines_withno = [(lineno, line)]
                for lineno, line in lines:
                    if not line.strip():
                        break
                    lines_withno.append((lineno, line))

                self._parsed.append(
                    Table(lines_withno, self._footnotes, self._filename)
                )
                continue

            if line.startswith(CodeBlock.PAT):
                pre_line = line
                lines_withno = []
                for lineno, line in lines:
                    if line == pre_line:
                        break
                    lines_withno.append((lineno, line))

                self._parsed.append(
                    CodeBlock(lines_withno, self._footnotes, self._filename)
                )
                continue

            if Title.RE.match(line):
                title.append((lineno, line))
                continue

            if Author.RE.match(line):
                author.append((lineno, line))
                continue

            if line.startswith(RawText.PAT):
                pre_line = line
                lines_withno = []
                for lineno, line in lines:
                    if line == pre_line:
                        break
                    lines_withno.append((lineno, line))

                self._parsed.append(
                    RawText(lines_withno, self._footnotes, self._filename)
                )
                continue

            if line.startswith(ShellBlock.PAT):
                pre_line = line
                lines_withno = []
                for lineno, line in lines:
                    if line == pre_line:
                        break
                    lines_withno.append((lineno, line))

                self._parsed.append(
                    ShellBlock(lines_withno, self._footnotes, self._filename)
                )
                continue

            # Add here if new elements are implemented

            if line.startswith('#'):
                raise MarkdownSyntaxError(
                    'unknown directive',
                    (self._filename, lineno, 0, len(line), line)
                )

            self._parsed.append(
                Text([(lineno, line)], self._footnotes, self._filename)
            )

        self._title = (
            Title(title, self._footnotes, self._filename) if title else None
        )
        self._author = (
            Author(author, self._footnotes, self._filename) if author else None
        )
        if not [
                t._lines_withno for t in self._parsed
                if len(t._lines_withno) > 1 or t._lines_withno[0][1].strip()
        ] and self._title is None:
            print(
                MarkdownSyntaxWarning(
                    'empty input file',
                    (self._filename, None, None, None, None)
                ).diagnose(), file=sys.stderr
            )


LATEX_PREAMBLE = r"""\documentclass[a4paper]{jsarticle}

% symbols
\usepackage[T1]{fontenc}
\usepackage{textcomp}

% color
\usepackage[dvipdfmx]{color}

% font
\usepackage{lmodern}
\newcommand{\textlt}[1]{\texttt{\fontseries{l}\selectfont #1}}

% title-formatting
\usepackage{titlesec}
\titleformat*{\section}{\Large\bfseries\rmfamily}
\titleformat*{\subsection}{\large\bfseries\rmfamily}
\titleformat*{\subsubsection}{\normalsize\bfseries\rmfamily}

% spacing
\setlength{\parskip}{.5\baselineskip}
\makeatletter
\titlespacing{\section}{0pt}%
             {.5\Cvs \@plus.5\Cdp \@minus.2\Cdp}{0pt \@plus.3\Cdp}
\titlespacing{\subsection}{0pt}%
             {.5\Cvs \@plus.5\Cdp \@minus.2\Cdp}{0pt \@plus.3\Cdp}
\titlespacing{\subsubsection}{0pt}%
             {.5\Cvs \@plus.5\Cdp \@minus.2\Cdp}{0pt \@plus.3\Cdp}
\newcount\ch
\ch=0\loop
  \ifnum\ch<256
  \xspcode \ch=3
\advance\ch by1
\repeat

% source-code
\usepackage{listings}
\lstset{
  basicstyle=\ttfamily\footnotesize,
  frame=single,
  flexiblecolumns=true,
  upquote=true,
  keepspaces=true,
}

\usepackage{enumerate}

% math
\usepackage{euler}
\usepackage{amsmath}
\thinmuskip=6mu
\SetMathAlphabet{\mathrm}{normal}{T1}{ccr}{m}{n}
\DeclareSymbolFont{concreteop}{T1}{ccr}{m}{n}
\makeatletter
\renewcommand{\operator@font}{\mathgroup\symconcreteop}
\makeatother

% linking
\usepackage{tablefootnote}
\usepackage{footnotehyper}
\usepackage[
  dvipdfmx,
  linktocpage=true,
  plainpages=false,
]{hyperref}
\hypersetup{pdftex, colorlinks=true, allcolors=blue}

% framing
\usepackage{framed}
"""

HTML_HEAD = r"""    <meta charset="utf-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1">

    <link href="/css/bootstrap.min.css" rel="stylesheet">
    <script src="/js/bootstrap.min.js"></script>

    <link href="https://fonts.googleapis.com/css?family=Cuprum&subset=latin" rel="stylesheet" type="text/css">
    <link href="https://fonts.googleapis.com/css?family=Lato:400,700" rel="stylesheet" type="text/css">
    <link href="/css/base.css" rel="stylesheet">
    <link href="/css/table.css" rel="stylesheet">
"""


def main():
    argc, argv = len(sys.argv), sys.argv[:]
    spec_fmt = None
    if 1 >= argc:
        print('warning: read from standard input.', file=sys.stderr)
        # print(argv[0], '<MARKDOWN>', '[LaTeX]', file=sys.stderr)
        # return 2
    elif argv[1] in ('-h', '--help'):
        print(argv[0], '<MARKDOWN>', '[LaTeX]')
        return 0

    options = {}
    # XXX use smart arg-parsers
    for arg in argv[:(argv.index('--') if '--' in argv else len(argv))]:
        if arg == '--as-part':
            argv.pop(argv.index('--as-part'))
            argc -= 1
            options['--as-part'] = True
        elif arg == '--html':
            argv.pop(argv.index(arg))
            argc -= 1
            spec_fmt = 'HTML'
        elif arg.lower() in ('--tex', '--latex'):
            argv.pop(argv.index(arg))
            argc -= 1
            spec_fmt = 'LaTeX'

    inname = argv[1] if 1 < argc else '-'
    if inname == '-':
        inname = '/dev/stdin'

    if argc <= 2:
        if inname == '/dev/stdin':
            outname = '/dev/stdout'
        else:
            if spec_fmt == 'HTML':
                outname = os.path.splitext(inname)[0] + '.html'
            else:
                outname = os.path.splitext(inname)[0] + '.tex'
    else:
        outname = argv[2]

    if outname == '-':
        outname = '/dev/stdout'

    with open(inname) as fin, open(outname, 'w') as fout:
        try:
            parser = MarkdownParser(fin)
            ext = os.path.splitext(outname)[1]
            fmt = (
                'HTML' if ext in ('.html',) else
                spec_fmt or 'LaTeX'  # default one
            )
            preamble = (
                HTML_HEAD if fmt == 'HTML' else
                LATEX_PREAMBLE if fmt == 'LaTeX' else
                ''
            )
            parser.translate(fout, fmt, preamble=preamble, options=options)
        except MarkdownSyntaxError as e:
            print(e.diagnose(), file=sys.stderr)
            if e.note is not None:
                print(e.note.diagnose(), file=sys.stderr)

            return 1

    return 0


if __name__ == '__main__':
    exit(main())

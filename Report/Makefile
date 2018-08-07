MD2TeX	= md2tex
LaTeX	= platex
DVI2PDF	= dvipdfmx
DVIFLAGS	= -f ~/.local/share/fonts/map/dvipdfmx/cid-x.map
PDF	= report.pdf

TeXDocs	= veb-tree.tex dc-online.tex cyk.tex rb-tree.tex cb-tree.tex \
	tsurai-parser.tex \
	todo.tex

all:	$(PDF)

$(PDF):	report.tex $(TeXDocs)
	$(LaTeX) report.tex
	$(LaTeX) report.tex  # rerun
	$(DVI2PDF) $(DVIFLAGS) report.dvi

report.tex:	$(TeXDocs)
	touch report.tex

%.tex:	%.md
	md2tex --as-part $<

clean:
	rm -vf *.{aux,log,toc,out,dvi} $(TeXDocs) *~

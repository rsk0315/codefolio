MD2TeX	= ./md2tex.py
LaTeX	= platex
DVI2PDF	= dvipdfmx
DVIFLAGS	= -f ./cid-x.map
PDF	= report.pdf
STAMP	= touch

TeXDocs	= veb-tree.tex dc-online.tex cyk.tex rb-tree.tex cb-tree.tex \
	tsurai-parser.tex sp-array.tex sp-uf.tex quick-find.tex \
	weighted-qf.tex \
	todo.tex

all:	$(PDF)

$(PDF):	report.tex $(TeXDocs) $(MD2TeX)
	$(LaTeX) report.tex
	$(LaTeX) report.tex  # rerun
	$(DVI2PDF) $(DVIFLAGS) report.dvi

report.tex:	$(TeXDocs)
	$(STAMP) report.tex

%.tex:	%.md
	$(MD2TeX) --as-part $<

clean:
	rm -vf *.{aux,log,toc,out,dvi} $(TeXDocs) *~

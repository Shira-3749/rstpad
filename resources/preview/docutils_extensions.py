global Node

import docutils.writers.html5_polyglot as html_writer
from docutils.nodes import Node

# store original methods
global orig_starttag
global orig_visit_document

orig_starttag = html_writer.HTMLTranslator.starttag
orig_visit_document = html_writer.HTMLTranslator.visit_document

# starttag() override
def starttag(self, node, tagname, suffix='\n', empty=False, **attributes):
    # define the data-line attribute if this is a Node with a known line
    line = None
    
    if isinstance(node, Node):
        line = node.line

        if line is None and node.parent is not None:
            line = node.parent.line

    if line is not None:
        attributes['data-line'] = str(node.line)

    return orig_starttag(self, node, tagname, suffix, empty, **attributes)

# visit_document() override
def visit_document(self, node):
    self.head.append('<link rel="stylesheet" type="text/css" href="qrc:///preview/preview.css">')
    self.head.append('<script type="text/javascript" src="qrc:///preview/preview.js"></script>')
    orig_visit_document(self, node)

# replace methods    
html_writer.HTMLTranslator.starttag = starttag
html_writer.HTMLTranslator.visit_document = visit_document

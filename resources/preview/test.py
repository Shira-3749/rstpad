import os
import docutils.core

execfile(os.path.dirname(os.path.realpath(__file__)) + '/docutils_extensions.py')

source = """

Hello
=====

How are you

""";

print docutils.core.publish_string(source, None, None, None, 'standalone', None, 'restructuredtext', None, 'html5', None, None, {'output_encoding': 'unicode'})

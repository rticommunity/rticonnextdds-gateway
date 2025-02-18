#!/usr/bin/env python3
# -*- coding: utf-8 -*-
#
# RTI Routing Service MQTT Adapter documentation build configuration file.
#
# This file is execfile()d with the current directory set to its
# containing dir.
#
# Note that not all possible configuration values are present in this
# autogenerated file.
#
# All configuration values have a default; values that are commented out
# serve to show the default.

# If extensions (or modules to document with autodoc) are in another directory,
# add these directories to sys.path here. If the directory is relative to the
# documentation root, use os.path.abspath to make it absolute, like shown here.
#
import os
import sys
sys.path.insert(0, os.path.abspath('.'))

# -- General configuration ------------------------------------------------

# If your documentation needs a minimal Sphinx version, state it here.
#
# needs_sphinx = '1.0'

# Add any Sphinx extension module names here, as strings. They can be
# extensions coming with Sphinx (named 'sphinx.ext.*') or your custom
# ones.
extensions = ['sphinx.ext.extlinks',
              'sphinx.ext.mathjax',
              'sphinx.ext.imgconverter',
              'breathe']
        #       'exhale']

# Add any paths that contain templates here, relative to this directory.
# templates_path = ['templates']

# The suffix(es) of source filenames.
# You can specify multiple suffix as a list of string:
source_suffix = '.rst'

# The master toctree document.
master_doc = 'index'

# General information about the project.
project = 'RTI Routing Service MQTT Adapter'
copyright = '2019, Real-Time Innovations, Inc'
author = 'Real-Time Innovations, Inc.'

# The version info for the project you're documenting, acts as replacement for
# |version| and |release|, also used in various other places throughout the
# built documents.
#
# The full version, including alpha/beta/rc tags.
release = "7.3.0"
version = release

# The language for content autogenerated by Sphinx. Refer to documentation
# for a list of supported languages.
#
# This is also used if you do content translation via gettext catalogs.
# Usually you set "language" from the command line for these cases.
language = "en"

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This patterns also effect to html_static_path and html_extra_path
exclude_patterns = ['doc', 'Thumbs.db', '.DS_Store']

# The name of the Pygments (syntax highlighting) style to use.
pygments_style = 'sphinx'

# If true, `todo` and `todoList` produce output, else they produce nothing.
todo_include_todos = False

numfig = True
numfig_format = {'figure': 'Figure %s',
                 'table': 'Table %s',
                 'code-block': 'Listing %s',
                 'section': 'Section %s'}

# -- Options for HTML output ----------------------------------------------

# The theme to use for HTML and HTML Help pages.  See the documentation for
# a list of builtin themes.
#
html_theme = 'sphinx_rtd_theme'
# html_theme_path = [sphinx_rtd_theme.get_html_theme_path()]
# Override default css to get a larger width for local build


def setup(app):
    app.add_css_file('theme_overrides.css')
    # app.add_js_file('custom.js')

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
#
html_theme_options = {
    "collapse_navigation" : False
}

# Add any paths that contain custom themes here, relative to this directory.
# html_theme_path = []

# The name for this set of Sphinx documents.
# "<project> v<release> documentation" by default.
#
# html_title = u'RTI Web Integration Service v5.2.3'

# A shorter title for the navigation bar.  Default is the same as html_title.
#
# html_short_title = None

# The name of an image file (relative to this directory) to place at the top
# of the sidebar.
#
html_logo = "static/rti-logo-FINALv2-White-OrangeDot.png"

# The name of an image file (relative to this directory) to use as a favicon of
# the docs.  This file should be a Windows icon file (.ico) being 16x16 or
# 32x32 pixels large.
#
html_favicon = "static/favicon.ico"

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
html_static_path = ['static']


# -- Options for HTMLHelp output ------------------------------------------

# Output file base name for HTML help builder.
htmlhelp_basename = 'RTIConnextGatewayMqttDoc'


# -- Options for LaTeX output ---------------------------------------------

latex_engine = 'lualatex'

latex_elements = {
    # The paper size ('letterpaper' or 'a4paper').
    #
     'papersize': 'letterpaper',

    # The font size ('10pt', '11pt' or '12pt').
    #
    'pointsize': '11pt',

    'preamble': '''\
\\sphinxsetup{TitleColor={named}{black},InnerLinkColor={named}{black},OuterLinkColor={named}{blue}}
\\usepackage[utf8]{inputenc}
\\usepackage[titles]{tocloft}
\\usepackage{newunicodechar}
\\usepackage{multirow}
\\usepackage{fontspec}
\\usepackage{hyperref}
\\usepackage{graphicx}
\\setkeys{Gin}{width=.85\\textwidth}
\\hypersetup{bookmarksnumbered}
\\setcounter{tocdepth}{3}
\\usepackage[draft]{minted}\\fvset{breaklines=true, breakanywhere=true}''',
    'printindex': '\\footnotesize\\raggedright\\printindex',
    'inputenc': '',
    'utf8extra': '',

    # Latex figure (float) alignment
    #
    # 'figure_align': 'htbp',

    #'tableofcontents' : ''
    'classoptions': ',openany,oneside',
    #releasename
    'releasename': 'Version',
    #fncychap (formatting of the chapter title)
    'fncychap': '',
    # main page
    'maketitle': '''\
        \\pagenumbering{Roman} %%% to avoid page 1 conflict with actual page 1

        \\begin{titlepage}
            \\centering

            \\vspace{40mm} %%% * is used to give space from top
            \\textbf{\\Huge{''' + project + '''}}
            
            \\vspace{7mm}
            \\textbf{\\Large{User\'s Manual}}
            
            \\vspace{17mm}
            \\textbf{\\Large{Version ''' + version + '''}}
            
            \\vspace{100mm}
            \\begin{figure}[!hb]
                \\centering
                \\includegraphics[scale=0.2]{rti-logo-tag-StackedRight.png}
            \\end{figure}

            \\vspace{0mm}

        \\end{titlepage}
        '''
}

latex_use_modindex = True

# Grouping the document tree into LaTeX files. List of tuples
# (source start file, target name, title,
#  author, documentclass [howto, manual, or own class]).
latex_documents = [
    (master_doc, 'RTIRoutingServiceMQTT.tex', 'RTI Routing Service MQTT Adapter User\'s Manual',
     'Real-Time Innovations, Inc.', 'manual'),
]

latex_logo = "static/rti-logo-tag-StackedRight.png"


# -- Options for manual page output ---------------------------------------

# One entry per manual page. List of tuples
# (source start file, name, description, authors, manual section).
man_pages = [
    (master_doc, 'rtiroutingservicemqtt', 'RTI Routing Service MQTT Adapter Documentation',
     [author], 1)
]


# -- Options for Texinfo output -------------------------------------------

# Grouping the document tree into Texinfo files. List of tuples
# (source start file, target name, title, author,
#  dir menu entry, description, category)
texinfo_documents = [
    (master_doc, 'RTIRoutingServiceMqtt', 'RTI Routing Service MQTT Adapter Documentation',
     author, 'RTIRoutingServiceMqtt', 'An adapter that enables access to the MQTT data domain.',
     'Routing'),
]

# -- Custom roles
from docutils import nodes

def role_litrep(name, rawtext, text, lineno, inliner,
            options={}, content=[]):

    node = nodes.Text(text)
    node.source, node.line = inliner.reporter.get_source_and_line(lineno)
    return [node],[]

from docutils.parsers.rst import roles
roles.register_local_role('litrep', role_litrep)

rst_epilog = """
.. |version| replace:: {0}
.. |project| replace:: {1}
""".format(version, project)

# Setup the breathe extension
breathe_projects = {
    "RTI Routing Service MQTT Adapter": "./doxyoutput/xml"
}
breathe_default_project = "RTI Routing Service MQTT Adapter"

# Setup the exhale extension
# exhale_args = {
#     # These arguments are required
#     "containmentFolder":     "./api",
#     "rootFileName":          "api.rst",
#     "rootFileTitle":         "Full API Reference",
#     "doxygenStripFromPath":  "..",
#     # Suggested optional arguments
#     "createTreeView":        True,
#     # TIP: if using the sphinx-bootstrap-theme, you need
#     # "treeViewIsBootstrap": True,
#     "exhaleExecutesDoxygen": True,
#     "exhaleUseDoxyfile": True,
# #     "afterTitleDescription": """
# # """
# #     "exhaleDoxygenStdin":    "INPUT = ../include"
# }

# Tell sphinx what the primary language being documented is.
primary_domain = 'c'

# Tell sphinx what the pygments highlight language should be.
highlight_language = 'c'

This is a meshlab plugins, SimUVex, implemented for linux (normally it's used on window with a old version of meshlab and qtcreator as well).

1-install Qt
2-install meshlab (github)
3-install VCG as meshlab page says (github)
4-copy this folder on src/meshlabplugins/
5-generate .pro file by the command qmake -project, insert "include (../../shade.pri) and put TEMPLATE=lib
6-generate Makefile by the command qmake -makefile (if some problems appers, calcel the RESOURCES on .pro file)
7-Do the same thing with external and common
8-Finally, do the same thing with meshlab_mini, adding the edit_uv line
9-Meshlab is in distrib folder

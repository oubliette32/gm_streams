premake4 --os=windows --platform=x32 --file=BuildProjects.lua vs2010
premake5 --os=macosx --platform=universal32 --file=BuildProjects.lua gmake
premake4 --os=linux --platform=x32 --file=BuildProjects.lua gmake
pause
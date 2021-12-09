@echo off

IF NOT EXIST W:\build\ mkdir W:\build\
pushd W:\build\
	cl -TC -MTd -nologo -Gm- -GR- -EHsc -EHa- -Od -Oi -WX -W4 -wd4201 -wd4100 -wd4189 -wd4505 -wd4706 -Z7 W:\src\Catin.c /link -opt:ref -incremental:no
popd

exit %errorlevel%

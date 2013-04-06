del /F /Q *.ncb *.plog *.pdb *.lib *.ilk *.exp *.*.user *.sdf
del /F /Q /A:H  *.suo

rmdir /S /Q Debug_LIB_md
rmdir /S /Q Debug_LIB_mt
rmdir /S /Q Debug_DLL_md
rmdir /S /Q Debug_DLL_mt
rmdir /S /Q Release_LIB_md
rmdir /S /Q Release_LIB_mt
rmdir /S /Q Release_DLL_md
rmdir /S /Q Release_DLL_mt
rmdir /S /Q ipch

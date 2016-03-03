setlocal
set SHADER_COMPILER=%VK_SDK_PATH%\Bin\glslangValidator.exe
%SHADER_COMPILER% -V -l -o SimpleVS.spv SimpleVS.vert
%SHADER_COMPILER% -V -l -o SimpleFS.spv SimpleFS.frag
endlocal

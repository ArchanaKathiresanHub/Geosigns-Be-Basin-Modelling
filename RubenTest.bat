@echo off
echo off

:Start

net time \\%computername% |find "Current time"
"E:\APPS\Microsoft Visual Studio 11.0\Common7\IDE\tf.exe" checkout "$/Basin Modeling/IBS/Trunk/RubenTest.bat"
"E:\APPS\Microsoft Visual Studio 11.0\Common7\IDE\tf.exe" undo "$/Basin Modeling/IBS/Trunk/RubenTest.bat" /noprompt


Sleep 5
GOTO Start
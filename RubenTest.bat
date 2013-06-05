@echo off
echo off

net time \\%computername% |find "Current time"
"E:\APPS\Microsoft Visual Studio 11.0\Common7\IDE\tf.exe" checkout "$/Basin Modeling/IBS/Trunk/RubenTest.bat"
"E:\APPS\Microsoft Visual Studio 11.0\Common7\IDE\tf.exe" undo "$/Basin Modeling/IBS/Trunk/RubenTest.bat" /noprompt

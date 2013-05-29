@echo off
echo off

"C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\tf.exe" checkout "$/Basin Modeling/IBS/Trunk/RubenTest.bat"
"C:\Program Files (x86)\Microsoft Visual Studio 11.0\Common7\IDE\tf.exe" undo "$/Basin Modeling/IBS/Trunk/RubenTest.bat" /noprompt

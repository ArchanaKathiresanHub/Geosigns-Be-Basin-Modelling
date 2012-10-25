'
' Helper script to create initial CMAKE configuration
'

Option Explicit

Dim wshell
Set wshell = CreateObject("WScript.Shell")
Dim objfso
Set objfso = CreateObject("Scripting.FileSystemObject")

Dim source_directory 
source_directory = objfso.GetParentFolderName(Wscript.ScriptFullName)

Dim project_directory

project_directory = SelectFolder(wshell.CurrentDirectory & "\..")
If project_directory = vbNull Then
    WScript.Echo "Cancelled"
Else
    WScript.Echo "Binary Dir: """ & project_directory & """" & vbNewLine & "Source Dir: """ & source_directory
    wshell.CurrentDirectory = project_directory
    wshell.Run ( "cmd /C " & source_directory & "\bootstrap32.bat" )
End If


Function SelectFolder( myStartFolder )
' This function opens a "Select Folder" dialog and will
' return the fully qualified path of the selected folder
'
' Argument:
'     myStartFolder    [string]    the root folder where you can start browsing;
'                                  if an empty string is used, browsing starts
'                                  on the local computer
'
' Returns:
' A string containing the fully qualified path of the selected folder
'
' Written by Rob van der Woude
' http://www.robvanderwoude.com

    ' Standard housekeeping
    Dim objFolder, objItem, objShell
    
    ' Custom error handling
    On Error Resume Next
    SelectFolder = vbNull

    ' Create a dialog object
    Set objShell  = CreateObject( "Shell.Application" )
    Set objFolder = objShell.BrowseForFolder( 0, "Please select/create folder for the build", 0, myStartFolder )

    ' Return the path of the selected folder
    If IsObject( objfolder ) Then SelectFolder = objFolder.Self.Path

    ' Standard housekeeping
    Set objFolder = Nothing
    Set objshell  = Nothing
    On Error Goto 0
End Function

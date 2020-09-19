Rem Run this VBScript script with cscript in MiKTeX fonts folder
Rem C:\Users\user\AppData\Local\Programs\MiKTeX 2.9\fonts>cscript genFontmap.vbs

Set objShell = WScript.CreateObject("WScript.Shell")
Set objExec = objShell.Exec("cmd /c dir /s/b *.pfb")

Do While Not objExec.StdOut.AtEndOfStream
  fullPath = objExec.StdOut.ReadLine()
  relPath = Right(fullPath, Len(fullPath) - Len(objShell.CurrentDirectory) - 1)

  Set objStream = CreateObject("ADODB.Stream")
  objStream.CharSet = "utf-8"
  objStream.Open()
  objStream.LoadFromFile(fullPath)
  strContents = objStream.ReadText()
  objStream.Close()
  Set objStream = Nothing

  r = "/FontName /"
  i = InStr(1, strContents, r)
  j = InStr(i + Len(r), strContents, " ")

  If i = 0 Or j = 0 Then
    WScript.Echo "% FontName cannot be extracted from" & _
      " (" & Replace(relPath, "\", "/") & ") ;"
  Else
    WScript.Echo "/" & Mid(strContents, i + Len(r), j - (i + Len(r))) & _
      " (" & Replace(relPath, "\", "/") & ") ;"
  End If
Loop

Set objExec = Nothing
Set objShell = Nothing

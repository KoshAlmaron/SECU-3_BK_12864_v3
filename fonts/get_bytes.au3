$FilePath = 'X:\Dropbox\Data\AVR\SECU-3_BK_12864_v3\fonts\Letter.xbm'

$AppName = 'XBM'

$Count = 0
$XBMPrev = ''

HotKeySet('{F5}', 'get_xbm')
HotKeySet('{END}', 'na_moroz')

ProgressOn($AppName, '����-� (' & String($Count) & ')', 'F5 ��� ������', @DesktopWidth - 350, 0, 16)

While True
   Sleep(100)
WEnd
;~ get_xbm()

Func get_xbm()
   ConsoleWrite('�������� ���������� �����' & @CR)
   $FileData = FileRead($FilePath)



   $XBMData = ''
   $N = 1

   For $Line In StringSplit($FileData, @LF)
	  ConsoleWrite($Line & @CR)
	  If $Line <> '' Then
		 If $N >= 4 Then
			$Line = StringReplace($Line, '};', '')
			$Line = StringStripWS($Line, 3)
			If $N = 5 Then
			   $XBMData = $Line
			Else
			   $XBMData = $XBMData & @CRLF & @TAB & @TAB & $Line
			EndIf
		 EndIf
	  EndIf
	  $N = $N + 1
   Next
   $XBMData = $XBMData & ','
   ClipPut($XBMData)

   $Result = ''

   If $XBMData = $XBMPrev Then
	  $Result = '������!'
   Else
	  $Result = 'OK'
   EndIf

   $XBMPrev = $XBMData
   ConsoleWrite($XBMData & @CR)
   $Count = $Count + 1
   ProgressSet(50, '����-� (' & String($Count) & ')', $Result)
EndFunc

; ������� ������.
Func na_moroz()
   If MsgBox(48+4+256, '��������?', '�� ���� ������ �������� ������ �������?') = 6 Then
	  Exit
   EndIf
EndFunc
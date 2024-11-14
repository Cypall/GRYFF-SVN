// $Id: README-gryff.txt 36 2005-05-10 20:36:16Z Rasqual $
// *
// *   gryff, a GRF archive management utility, README-gryff.txt
// *   Copyright (C) 2003, 2004, 2005 Rasqual Twilight
// *
// *   This program is distributed in the hope that it will be useful,
// *   but WITHOUT ANY WARRANTY; without even the implied warranty of
// *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// *

gryff - GRF archive management utility
======================================

Copyright © 2003-2005 Rasqual Twilight, All rights reserved.
http://gryff.ragnarok-online.info/?act=gryff
Not affiliated with Gravity, Inc.

Introduction
------------

gryff is a Windows GUI program supporting the Ragnarok
Online (tm) archive format, Grf version 1 and 2.
It uses a multiple document architecture which allows to
manage operations on several archives at a time (*).
Information on Ragnarok Online (tm) is available from 
http://www.ragnarokonline.com/

gryff is free software. Many features are not fully implemented,
and there are certainly many bugs. Use it at your own risk.

gryff is written using WTL, a set of C++ classes published under
the common public license (CPL). WTL can be found at
http://wtl.sourceforge.net/ and requires at least a Microsoft
C++ compiler, version 7.0 or later, to compile (due to ATL
dependencies).

Feedback is welcome, and it is likely that any reported bugs will
be at least examined, at best fixed.
Send any feedback or bug reports to the support board at
http://gryff.ragnarok-online.info/?act=forum

(*) Planned feature


Requirements
------------

Windows NT 5.0 (2000) or later.

Runtimes:
	msvcr71.dll  (Visual C / C++ runtimes from Visual Studio .NET 2003)
	msvcp71.dll
	grf-priv-1.2.0.x.dll  (libgrf private dll build)

There is also a version of gryff, known as "gryff_s" that does not
link to any of the above dlls.

Using
-----
gryff needs no installation and runs as is. You may, however, associate
.grf and .gpf extensions to the application using the shell.
There are three ways to open a file : using the command line, using the
Open File menu entry, or by drag-and-dropping a file into the main
window.

You can drag and drop files to add to a document into the listview.
By default, all dropped items are supposed to have proper Unicode encoding,
however many GRF unpackers offer the option to extract files with so-called
"ASCII" filenames, which are in fact CP-949 encoded filenames. This is
often justified by the fact many tools don't support Unicode filenames.
What's more, it is impossible to use Unicode filenames on a command line
if they cannot be converted to the system code page.
This is why gryff allows to convert filenames to Unicode. To achieve
this, hold the [Alt] key when dropping such files. Think of it as
an "alternative interpretation" of the filenames.

If you have a folder name in CP-949 you would like to use, you can prepend
a backslash before you enter it when renaming a folder.


Limitations
-----------
Only 0x200 grf can be output by gryff.
Future modifications may be done to be able to write Alpha GRFs.


Known "features"
----------------
. The sorting algorithm uses a primary and a secondary key in the listview,
  depending on the order the headers are clicked.


Known issues
------------
. Drag and drop between documents is not possible for this version.
. Some errors are silently discarded when importing a GRF containing
  invalid entries.


Acknowledgements
----------------
gryff uses code from libgrf, a GPL'ed grf library written by
 the OpenKore contributors, using several routines of the zlib library,
itself being written by Jean-loup Gailly and Mark Adler.
http://openkore.sourceforge.net/ http://zlib.net/
Please note, source code for building the libgrf dynamic link library
shall be found at where you downloaded this program in conformance with
the terms of the GNU GPL. If it not the case, you may visit
my website, http://gryff.ragnarok-online.info/?act=gryff and get one.

gryff uses a modified version of 'CSimpleHtmlCtrl - A RTF-based HTML Viewer'
wrapper classes, copyright (c) 2003 Bjarke Viksoe.
http://www.viksoe.dk/

Robert Edward Caldecott's CFileDialogFilter is a nifty class allowing to
quickly build file filters for common open file dialogs using a pipe
separator instead of nuls. It may be found at its CodeProject page:
http://www.codeproject.com/wtl/wtlfilterstring.asp

The CMultiFileDialog class implementation in WTL is greatly inspired from
"Multiple Selection in a File Dialog" By PJ Arends, 
http://www.codeproject.com/dialog/pja_multiselect.asp

Thanks to Gabriel Kniznik for his brilliant article and code draft
on implementing the MDI architecture with WTL.

Ragnarok icons were designed by Rose Besch, aka bara-chan. I don't
remember where to obtain them, however.


License
-------

This program is free software; you can redistribute it and/or 
modify it under the terms of the GNU General Public License 
as published by the Free Software Foundation;  either 
version 2 of the License, or (at your option) any later 
version.

This program is distributed in the hope that it will be useful, 
but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A 
PARTICULAR PURPOSE.  See the GNU General Public 
License for more details.

You should have received a copy of the GNU General 
Public License along with this program; if not, write to the 
Free Software Foundation, Inc., 59 Temple Place - Suite 
330, Boston, MA  02111-1307, USA.


Changelog
---------
See Changelog for details

Dev. roadmap
------------
Don't take it for granted :)
1.0   R/W Grf                                                       [OK]
      Modif: Add(file/dir/grf)                                      [OK]
      Deleting                                                      [OK]
      View: List/details                                            [OK]
      Drag and Drop from file manager                               [OK]
      Moving entries                                                [OK]

1.1   Open with associated                                          [OK]
      Extract f/d                                                   [OK]
      Search dialog as seen in WinRAR. Possibly support of RE for patterns.

1.2   User options
      Listing generation
      File manager Contextual menus

1.3   Copy/Move conflicts resolution
      Application clipboard

1.5   Multiple views
      Drag and Drop support between 2 frames,
        betw. 2 instances and betw. application and explorer/other app

1.6   Views - details with accurate info
      File icons
      File associations, shell ext.

1.8   Customize toolbar
      Language files

1.9   Property pages
      Progress dialog (More options, etc.)

2.0   Listview caching
      Navigation history
      MRUs                                                          [OK]
      Patch client emulator (append mode)


Extensions that may make it to the app one day:

x.x   Undo history
      Hexadecimal preview
      file descriptions (.diz extension)
      File watcher (reincorporate changes to extracted files)

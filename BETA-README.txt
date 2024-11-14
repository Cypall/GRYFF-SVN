// $Id: BETA-README.txt 36 2005-05-10 20:36:16Z Rasqual $
// *
// *   gryff, a GRF archive management utility, BETA-README.txt
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

Guidelines
----------

Required info:
	- Operating System, system and user locale
	- whether East-Asian support is installed
	- CPU (essentially: 32-bit or 64-bit)
	- RAM / swap, free disk space
	- Version (beta / RC / release / SVN)

Type of beta-tester:
	- With Microsoft Visual Studio .NET 2002, 2003 or more (can compile and/or debug)
	- Normal user


Basic test suite
----------------

* opening
	.  from removable media  (->what happens when the tray is opened while reading?) or bad sectors (lol)
	.  file w/ Unicode filename (try with read-only media / permissions set to read only)
	.  network : UNC and mount - UNC path larger than 260 characters
	.  truncated file / invalid format / crafted data (/!\ buffer sizes)

* Inserting / Modifying
	.  File or folder names whose len is > GRF_MAX_PATH (260)
	.  Unicode filenames (korean or not)
	.  CP-949 import : files and directories  with mix Unicode or not
	.  Renaming tests (buflen, charset, invalid chars)
	.  Drag and Drop tests (target conditions, within and out of document, interaction with shell / third party app such as text ed.)

* Deleting
	.  One / Several files
	.  One empty folder
	.  One folder with files
	.  One folder with files and non-empty subfolders
	.  Several files and several folders

* Closing
	.  Saving as a document already open in the manager
	.  Make some I/O operations fail
	.  "Race conditions" tests when two documents cross reference with possibly a neutral third one.

* Refresh
	.  After insertion / suppression / a referenced doc has been saved
	.  Navigation
	.  Changing another view (n.i.)
	.  Changing view type (list, details) - how is selection affected?

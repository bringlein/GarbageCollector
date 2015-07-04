# GarbageCollector
*GarbageCollector* - a multithreaded File Carver - **winner of the File-Carving-Contest organized by the [Chair of Computer Security](https://www1.cs.fau.de/)** at the [FAU](https://www.fau.de/) in Summer-Term 2015! 


## Features 

Currently *GarbageCollector* can scan disc images as well as devices for files of the following types:
* pdf
* jpg 
* png 
* sqlite

It can recover - unlike other popular file carvers e.g. foremost - **files of any size**. 
*GarbageCollector* is still a file carver, thus only non-fragmented files can be recovered. Detection is based on format-specific headers and footers regardless of the underlying file system. 
As *GarbageCollector* supports multithreading by design, it is **faster than other** popular file carving tools, too. 


## Module structure
![Module structure](https://raw.github.com/btlorch/GarbageCollector/master/structure.png)


## Authors: 
###Team: The Dumpster Divers 

- Lukas Dresel 
- Julius Lohmann
- [Benedikt Lorch](https://github.com/btlorch)
- [Burkhard Ringlein](https://github.com/cliffTifflor)
- Andreas Rupp
- Yuriy Sulima 
- Carola Touchy


## Usage 

`./GarbageCollector path/to/image path/to/output_destination  [-N] [-T]`

-N: inserts the name of the carved image or device in the name of the output directory

-T: inserts a timestamp in the name of the output directory 


## License 

	Copyright (c) 2015. Lukas Dresel, Julius Lohmann, Benedikt Lorch, Burkhard Ringlein, Andreas Rupp, Yuriy Sulima, Carola Touchy

	GarbageCollector is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	GarbageCollector is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with GarbageCollector.  If not, see <http://www.gnu.org/licenses/>.


## Installation 

Simply:

	git clone git@github.com:cliffTifflor/GarbageCollector.git GarbageCollector 

Or go to the [Realease-Page](https://github.com/cliffTifflor/GarbageCollector/releases) and download the latest zip. 
Then: 

	cd GarbageCollector
	make 



Of course you need the build essentials of your system (C++, GNU Make, g++, etc.). 






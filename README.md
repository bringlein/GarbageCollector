# GarbageCollector
*GarbageCollector* - a multithreaded File Carver - **winner of the File-Carving-Contest from the [Chair for Computer Security](https://www1.cs.fau.de/)** at the [FAU](https://www.fau.de/) in Summer-Term 2015! 


## Features 

Currently *GarbageCollector* can scan disc images as same as whole discs for the following formats:
* pdf
* jpg 
* png 

He detects - in opposite of other popular File Carver e.g. foremost - **files of any size**. 
But, *GarbageCollector* is still a File Carver, i.e. he detects non-fragmented files due to its typical headers and footers and not through analysing the file system. 
Because *GarbageCollector* is multithreaded, he is **faster than other** popular File Carver too. 


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

-N: inserts the namme of the Image in the name of the output directory

-T: inserts a Timestamp in the name of the output directory 


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

`git clone git@github.com:cliffTifflor/GarbageCollector.git GarbageCollector 
 cd GarbageCollector
 make `

Of course you need the build essentials of your system (C++, GNU Make, g++, etc.). 





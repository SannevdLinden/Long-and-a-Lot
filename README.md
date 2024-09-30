# Long and a Lot Code

Welcome to our GitHub page. 

This page contains the code for the Long and a Lot (LoLo) paper for visualizing and analyzing long event sequences. 

The folder named code contains our code. The folder dummy data includes a very small dummy data set to illustrate the interactions. The folder named demo contains a demo of LoLo. You can run this by downloading
the entire demo folder and then opening the executable in this folder.   



## Dependencies:
This project is based on [Qt 6.4.0 MinGW 64 bit](https://www.qt.io/download-dev). The Qt files in the demo folder are also covered under the LGPL license v3, for more info see [this page](https://www.qt.io/product/features?license-model=lgpl-v3). 



## Data Files:
The folder named dummy data contains a dummy sequence file (test_seq.csv), sequence meta file (seq_info.csv), and pattern file (pattern_file.csv). 

The sequence file contains all the sequence data. The file should have the following format: three columns; "id" (sequence id from 0 to the number of sequences - 1), "event" (the event category), 
and "time" (timestamp in the following format YYYY-MM-DDTHH:MM:SS.ssZ). A comma should separate the columns and enter the rows. Ensure that the file does not contain a white/empty line at the end.   

The sequence meta files contain the "id" of the sequences (same id as in the sequence file) and a sequence name. There are no headers and the same formatting hold as for the sequence file.  

The pattern file contains six headers: "pat_id" (unique patterns id), "pattern" (list/array with the pattern), support (support of the pattern), sequence (sequence id where this occurrence 
of the pattern occurred), start (start index of the pattern in the sequence), end (first index after the pattern). Each pattern has its own row. The patterns are ordered based on start index. 



## Other libraries:
The source code uses a [DBscan library](https://github.com/CallmeNezha/SimpleDBSCAN) for the clustering from ZJ Jiang (Nezha)  and a library for the [UMAP](https://github.com/libscran/umappp) dimensionality reduction computation from Aaron Lun . 
Their code can be found and downloaded at their respective pages. 

The demo folder contains a windows executable file of LoLo. The DBscan library and the UMAP libary are used. The UMAP library also depends of other libraries. 
A list of the libraries (with their respective code/licenses) are:
- [DBscan](https://github.com/CallmeNezha/SimpleDBSCAN) library for the clustering from ZJ Jiang (Nezha), [license](https://rem.mit-license.org/) 

- [UMAP](https://github.com/libscran/umappp) from Aaron Lun, [license](https://github.com/libscran/umappp/blob/master/LICENSE)
- [aarand](https://github.com/LTLA/aarand/tree/master) from Aaron Lun, [license](https://github.com/LTLA/aarand/blob/master/LICENSE)
- [knncolle](https://github.com/knncolle/knncolle) from Aaron Lun,[license](https://github.com/knncolle/knncolle/tree/master?tab=MIT-1-ov-file#readme)
- [irlba](https://github.com/LTLA/CppIrlba) from Aaron Lun, [license](https://github.com/LTLA/CppIrlba/blob/master/LICENSE)
- [kmeans](https://github.com/LTLA/CppKmeans) from Aaron Lun, [license](https://github.com/LTLA/CppKmeans?tab=MIT-1-ov-file#readme)
- [annoy](https://github.com/spotify/annoy/tree/main) from Spotify , [license](https://github.com/spotify/annoy/blob/main/LICENSE)
- [hnswlib](https://github.com/nmslib/hnswlib/tree/master) from Malkov et al., [license](https://github.com/nmslib/hnswlib/tree/master?tab=License-1-ov-file)
- [Eigen](https://gitlab.com/libeigen/eigen/-/tree/master?ref_type=heads) started by Benoît Jacob and Gaël Guennebaud, and now many others, [license](https://gitlab.com/libeigen/eigen/-/blob/master/COPYING.MPL2?ref_type=heads)



## Reference to paper:

Please Cite our work (currently under review/submitted):
"Sanne van der Linden, Bram Cappers, Anna Vilanova, Stef van den Elzen. 2024. Long and a Lot (LoLo): a Visual Analytics Approach for Analyzing Long Event Sequences." 



## License: 
The license for the code of LoLo is [LGPL v3](https://www.gnu.org/licenses/lgpl-3.0.en.html), see license file. The LGPL has some references to the GNU GPL license, which can be found [here](https://www.gnu.org/licenses/gpl-3.0.en.html). 


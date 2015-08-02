# telecom
a crack at the problem in https://www.reddit.com/r/programming/comments/3dmltg/how_we_store_400m_phone_number_data_with_fast/

the __gun_parallel secions can be replaced with STL equivlents 

To build you will need a c++14 ( c++11 may work I haven't been testing for that)
Run make or to create the executables. 
There are three: 
1. eatram-This program just eats up the systems ram quickly. I've used it to clear my chaches. A bit of a hack, but it works. 
I suggest NOT running this on a system with swap space enabled.

2. real-This is the actuall program that reads and searches the csv file that contains the numbers.
This program takes one optoinal command line argument, the name of the file that will be read in. The default is "dump".

3. gen-This program generates the data for real and places it in a file called "dump".
This file can be read by real. It takes one optional command line argument, the number of entriest to generate.
The default is 600,000,000. WARNING this is a 20GB file by default.


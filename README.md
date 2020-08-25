CSCI-340
Programs for CSCI-340 Operating System Principles
Professor: John Svadlenka


**************** <br>
Disclaimer: The code contained in these programs is not to be copied, claimed, or submitted by anyone other than the original owner.
<br>

CS340-MultiThread<br>
<br>
Multithreaded sorting program <br>
Using C posix threads <br>
<br>
---------------------------------- <br>
<br>
Project 2—Multithreaded Sorting Application
Write a multithreaded sorting program that works as follows: Alist of integers
is divided into two smaller lists of equal size. Two separate threads (which we
P-26 will term sorting threads) sort each sublist using a sorting algorithm of your
choice. The two sublists are then merged by a third thread—a merging thread
—which merges the two sublists into a single sorted list.
Because global data are shared across all threads, perhaps the easiest way
to set up the data is to create a global array. Each sorting thread will work on
one half of this array. A second global array of the same size as the unsorted
integer array will also be established. The merging thread will then merge the
two sublists into this second array. Graphically, this program is structured as
in Figure 4.27.
This programming project will require passing parameters to each of the
sorting threads. In particular, it will be necessary to identify the starting index
from which each thread is to begin sorting. Refer to the instructions in Project
1 for details on passing parameters to a thread.
The parent threadwill output the sorted array once all sorting threads have
exited. <br>
<br>
![Example](https://i.imgur.com/XKi8wkH.png)<br>

<br>

CS340-Shell <br>
Smash is the newest and hottest shell not on the market.
Designed by a litteral code chimp smash is the kind of shell that you would not want to use willingly.

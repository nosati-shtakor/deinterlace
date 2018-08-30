# deinterlace
This was a task given to me as a part of the job interview. Once I have sent my first solution, I noticed that it was wrong.
Although I blew my chance, I decided to fix it and share it here.

The interlace method described in the task is:

"Blending" algorithm: line N in the output image is the average of lines N and N-1 from the input image.
The first line stays unchanged.

In other words, it is not any of the official deinterlacing methods like BOB, motion adaptive, motion compensation, etc...
The code might still contain some bugs, so do some testing on your own if you intend to use it. You have been warned.
If you have any questions, try: nkalisni@gmail.com


Note:

A project called libjpeg is a part of this solution. You need it if you want to build this on Windows. It is created from the libjpeg project source files (version 6b). That software is the work of Tom Lane, Philip Gladstone, Jim Boucher, Lee Crocker, Julian Minguillon, Luis Ortiz, George Phillips, Davide Rossi, Guido Vollbeding, Ge' Weijers, and other members of the Independent JPEG Group.

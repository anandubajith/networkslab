# Question
Write a client/server program with the socket interface. The client program may send out the words
to the server once the user presses "Enter" and the server will respond to these words with any
meaningless terms. However, the server will close the connection once it receives the word "Bye".
Also, once someone keys in "GivemeyourVideo", the server will immediately send out a 50 MB data
file with message size of 500 bytes.

a. Calculate and record the data transmission rate every 0.1 second for a 50 MB data transmission
with message size of 500 bytes.

b. Use xgraph and gnuplot to display the results [Hint: Use timer and handler functions].

c. Create a socket that implements stop and wait ARQ protocol and analyze the RTT for varying
message sizes.


# Notes

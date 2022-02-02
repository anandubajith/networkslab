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

- 50MB = 50 * 1024 * 1024 bytes
    => Duration?
    => 500 bytes => 0.1 seconds
    => 5kB => 1second
    => 1000kb = 1mb => 200seconds
    => 50MB => 166 minutes

- This would mean we are directly sending right? [ we need to calculate data taransmission rate ]

- what results you want us to display??
    - Transmission rate? in realtime?

- ARQ -> Automatic Repeat Request
    - Stop and wait ARQ => Send packet wait for ACK, if not ACK in x time, resend packet [ keep packet in server memory ]
    - GoBackN ARQ => ?
    - SelectiveRepeat ARQ => ?


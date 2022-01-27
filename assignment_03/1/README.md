# Question
**Implementation of basic Client Server program using both TCP and UDP Socket.**
Write a client/server program with the socket interface. The server has the list of five
fruits 'apple', 'mango', 'banana', 'chikoo' and 'papaya' available along with their count
stored in a buffer. The user presses "Fruits" and the server responds with
"Enter the name of the fruit". The user then responds with the fruit name and the count.
The server decreases the count of the entered fruit value from its buffer.
If the count of the fruit the user needs is more than the available count, the server responds with
"Not available". Once someone keys in "SendInventory", the server will send a message which
includes all the fruit names along with their count.


# Notes
- All of them need to take PORT argument
- Inventory code is abstracted away



## Implementation
### Sender Implementation

In my sender implementation, located in `sender.c`, The sender effectively breaks down the data into fixed-size chunks and dispatches them to the receiver. To counter potential delays or packet loss, I've implemented a timeout mechanism. If an acknowledgment for a specific chunk isn't received within 0.1 seconds, the sender automatically resends that chunk. An important feature of this implementation is its ability to maintain an uninterrupted data flow – it doesn't wait for acknowledgments of previously sent chunks before transmitting subsequent ones.

In this, I've introduced an `ack_received` array to monitor the acknowledgment status of each chunk. The sender operates within an infinite loop, ensuring packets are continuously sent until every one of them is acknowledged. Notably, it continuously resends packets for which acknowledgments haven't been received yet, ensuring that no data is left unacknowledged. Once all packets have been acknowledged, the sender sends a termination signal, instructing the receiver to halt further data reception. This strategy ensures reliable and efficient data delivery, combining persistent retransmissions with acknowledgment tracking for seamless communication.
### Receiver Implementation

The receiver, implemented in `receiver.c`, awaits the arrival of data chunks. Upon receiving a termination signal, it decodes the message by grouping the received chunks based on their unique sequence numbers. The receiver then arranges the chunks in the correct order, reconstructing the original message accurately. To add an element of randomness and test the sender's robustness, the receiver occasionally skips packets. If a packet is skipped, it is not acknowledged, prompting the sender to resend the packet.



## How is **my** implementation of data sequencing and retransmission different from traditional TCP?

My implementation endeavors to integrate data sequencing and retransmission, pivotal elements of TCP, within the confines of UDP sockets. Here's how my approach differs from the traditional TCP protocol:

### 1. **Data Sequencing:**

**TCP:**
- TCP inherently employs sequencing through packet sequence numbers. It ensures in-order delivery, buffering out-of-order data until missing packets are received.

**My Implementation:**
- I manually partition the data into chunks, each assigned a specific sequence number. This manual sequencing becomes necessary due to UDP's lack of inherent sequencing.
- Unlike TCP, my implementation doesn't guarantee in-order delivery. If packets arrive out of order, my system doesn't buffer and reorder them. Instead, it relies on manual sequencing to arrange data correctly at the receiver's end.

### 2. **Retransmissions:**

**TCP:**
- TCP automatically retransmits lost packets. Upon not receiving an acknowledgment within a set timeout, TCP assumes packet loss and retransmits it. TCP optimally uses a sliding window protocol for efficient flow control and retransmissions.

**My Implementation:**
- My implementation also involves retransmissions, but they are managed manually. If an acknowledgment isn't received within a specified timeout, I initiate a packet resend.
- Unlike TCP, my approach doesn't utilize a sliding window protocol. It waits for an acknowledgment for the current packet before transmitting the next one, lacking the efficiency of TCP's sliding window approach.

| Aspect                      | TCP                                       | My Implementation                           |
|------------------------------|-------------------------------------------|-----------------------------------------------|
| **Data Sequencing**          | - Inherent sequencing with sequence numbers. - Guarantees in-order delivery. - Buffers out-of-order data.  | - Manual sequencing with assigned sequence numbers to chunks. - Does not guarantee in-order delivery. - Does not buffer out-of-order data. |
| **Retransmissions**          | - Automatic retransmission of lost packets. - Uses sliding window protocol for efficient flow control. | - Manual retransmission upon timeout. - Does not use sliding window protocol; waits for acknowledgment before sending the next packet. |



## How can I extend my implementation to account for [flow control](https://en.wikipedia.org/wiki/Transmission_Control_Protocol#Flow_control)? (Ignoring Deadlocks)

Incorporating flow control into the UDP-based data transfer system involves the implementation of a sliding window protocol. This protocol regulates the amount of data sent by the sender to match the receiver's processing capabilities, preventing the receiver from becoming overwhelmed and ensuring efficient data transfer.

To achieve this, the following modifications can be made to both the sender and receiver sides of the implementation:

**Sender Side:**
- Introduction of variables such as `base` and `next_seq_num` to manage the sliding window.
- `base` represents the sequence number of the oldest unacknowledged packet, and `next_seq_num` represents the sequence number of the next packet to be sent.
- Sending packets is restricted to within the window size (`window_size`). Only packets falling within the window range (`base` to `base + window_size - 1`) are transmitted.
  
**Receiver Side:**
- Modification of acknowledgment sending logic to acknowledge packets up to a certain sequence number (`base + window_size`).
- Introduction of the `expected_seq_num` variable to keep track of the next expected sequence number.
- Upon receiving a packet, it is checked if its sequence number matches `expected_seq_num`. If so, the packet is processed; otherwise, it is buffered for later processing.
- Cumulative acknowledgments are sent up to the highest received sequence number within the window, indicating the successful reception of packets up to that point.

**Timeout Handling:**
- Implementation of timers for each unacknowledged packet to track their expiration.
- If a timer expires for the packet at the base of the window, packets within the window (from `base` to `base + window_size - 1`) are retransmitted to ensure their reliable delivery.

By incorporating these modifications, the UDP-based data transfer system can maintain efficient flow control, preventing congestion and ensuring reliable data transmission between sender and receiver. This approach enhances the system's robustness and adaptability in varying network conditions.

---
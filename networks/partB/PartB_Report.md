## Implementation
### Sender Implementation

The sender, implemented in `sender.c`, operates by segmenting the data into fixed-size chunks. These chunks are then transmitted to the receiver. To handle potential delays or packet loss, the sender employs a timeout mechanism. If an acknowledgment for a chunk is not received within 0.1 seconds, the sender resends the chunk. Notably, the sender does not wait for acknowledgments of previously sent chunks before transmitting subsequent ones, ensuring continuous data transmission. The sender terminates its operation only when acknowledgments are received for all transmitted chunks. A termination signal is then sent to instruct the receiver to cease further data reception.

### Receiver Implementation

The receiver, implemented in `receiver.c`, awaits the arrival of data chunks. Upon receiving a termination signal, it decodes the message by grouping the received chunks based on their unique sequence numbers. The receiver then arranges the chunks in the correct order, reconstructing the original message accurately. To add an element of randomness and test the sender's robustness, the receiver occasionally skips packets. If a packet is skipped, it is not acknowledged, prompting the sender to resend the packet.



## How is **my** implementation of data sequencing and retransmission different from traditional TCP?

My implementation attempts to incorporate data sequencing and retransmission, key features of TCP, using UDP sockets. Here's how my implementation differs from traditional TCP:

### 1. **Data Sequencing:**

**TCP:**
- In TCP, data sequencing is inherent. Each packet sent has a sequence number, and the receiver arranges packets in the correct order using these sequence numbers.
- TCP guarantees in-order delivery of data. If packets arrive out of order, TCP buffers the out-of-order data until the missing packets are received.

**My Implementation:**
- In my implementation, I manually divide the data into chunks and assign sequence numbers to each chunk. This manual sequencing is not a part of the UDP protocol but is necessary in your case due to the lack of inherent sequencing in UDP.
- UDP does not guarantee the order of delivery. If packets arrive out of order, your implementation does not buffer and reorder them. Instead, it relies on the manual sequencing to arrange the data correctly at the receiver's end.
  
### 2. **Retransmissions:**

**TCP:**
- TCP implements automatic retransmission of lost packets. When the sender doesn't receive an acknowledgment within a certain timeout period, it assumes the packet was lost and retransmits it.
- TCP uses a sliding window protocol to manage the flow control and retransmissions efficiently.

**My Implementation:**
- My implementation also includes retransmissions, but they are handled manually. If an acknowledgment is not received within a specified timeout, you resend the packet.
- Unlike TCP, My implementation does not use a sliding window protocol. It waits for an acknowledgment for the current packet before sending the next one. This is a simplification and might not be as efficient as TCP's sliding window approach, especially in high-latency or unreliable networks.


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

By incorporating these modifications, the UDP-based data transfer system now maintains efficient flow control, preventing congestion and ensuring reliable data transmission between sender and receiver. This approach enhances the system's robustness and adaptability in varying network conditions.

---
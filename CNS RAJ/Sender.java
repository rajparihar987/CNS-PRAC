import java.io.*;
import java.net.*;
import java.util.*;

public class Sender {
    public static void main(String[] args) throws Exception {
        Scanner sc = new Scanner(System.in);
        System.out.print("Select Mode: 1 for Go-Back-N, 2 for Selective Repeat: ");
        int mode = sc.nextInt();

        Socket socket = new Socket("localhost", 9090);
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));

        int windowSize = 4;
        int totalPackets = 12;
        String[] packets = new String[totalPackets];

        // Generate circular sequence numbers 0 1 2 3 0 1 2 3...
        for (int i = 0; i < totalPackets; i++) {
            int seq = i % windowSize;
            packets[i] = seq + ":Packet " + i;
        }

        if (mode == 1) { // Go-Back-N
            int base = 0;
            int next = 0;

            while (base < totalPackets) {
                while (next < base + windowSize && next < totalPackets) {
                    out.println(packets[next]);
                    System.out.println("Sent: " + packets[next]);
                    next++;
                }

                socket.setSoTimeout(2000); // 2 sec timeout

                try {
                    String ackStr = in.readLine();
                    int ack = Integer.parseInt(ackStr);
                    System.out.println("Received ACK: " + ack);

                    // Find last index with this sequence number
                    int moveTo = base;
                    for (int i = base; i < totalPackets; i++) {
                        if (i % windowSize == ((ack + 1) % windowSize)) {
                            moveTo = i;
                            break;
                        }
                    }
                    base = moveTo;
                } catch (SocketTimeoutException e) {
                    System.out.println("Timeout! Resending from base: " + base);
                    next = base;
                }
            }
        }

        else if (mode == 2) { // Selective Repeat
            boolean[] acked = new boolean[totalPackets];
            int base = 0;

            while (base < totalPackets) {
                for (int i = base; i < base + windowSize && i < totalPackets; i++) {
                    if (!acked[i]) {
                        out.println(packets[i]);
                        System.out.println("Sent: " + packets[i]);
                    }
                }

                socket.setSoTimeout(2000);

                try {
                    String ackStr = in.readLine();
                    int ack = Integer.parseInt(ackStr);
                    System.out.println("Received ACK: " + ack);

                    // Mark all unacked packets with matching seq number
                    for (int i = base; i < totalPackets; i++) {
                        if (i % windowSize == ack && !acked[i]) {
                            acked[i] = true;
                            break;
                        }
                    }

                    // Slide window
                    while (base < totalPackets && acked[base]) base++;
                } catch (SocketTimeoutException e) {
                    System.out.println("Timeout (simulated), will resend unacked packets.");
                }

                Thread.sleep(1000); // Small delay
            }
        }

        socket.close();
        System.out.println("All packets sent and acknowledged. Sender shutting down.");
    }
}

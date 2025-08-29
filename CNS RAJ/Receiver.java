import java.io.*;
import java.net.*;
import java.util.*;

public class Receiver {
    public static void main(String[] args) throws Exception {
        Scanner sc = new Scanner(System.in);
        System.out.print("Select Mode: 1 for Go-Back-N, 2 for Selective Repeat: ");
        int mode = sc.nextInt();

        ServerSocket server = new ServerSocket(9090);
        System.out.println("Receiver is waiting for sender...");
        Socket socket = server.accept();
        System.out.println("Connected to sender.");

        BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        PrintWriter out = new PrintWriter(socket.getOutputStream(), true);

        int expected = 0;
        int windowSize = 4;
        int totalPackets = 12;
        boolean[] received = new boolean[windowSize];
        String[] buffer = new String[windowSize];

        int delivered = 0;

        while (delivered < totalPackets) {
            String data = in.readLine();
            if (data == null) continue;

            String[] parts = data.split(":");
            int seq = Integer.parseInt(parts[0]);
            String msg = parts[1];

            System.out.println("Received packet " + seq + ": " + msg);

            if (mode == 1) { // Go-Back-N
                if (seq == expected % windowSize) {
                    System.out.println("Accepted packet " + seq);
                    expected++;
                    delivered++;
                } else {
                    System.out.println("Discarded packet " + seq + ", expected " + (expected % windowSize));
                }
                out.println((expected - 1) % windowSize);
            }

            else if (mode == 2) { // Selective Repeat
                if (!received[seq]) {
                    received[seq] = true;
                    buffer[seq] = msg;
                    System.out.println("Buffered packet " + seq);
                }

                while (received[expected % windowSize]) {
                    System.out.println("Delivered packet " + (expected % windowSize) + ": " + buffer[expected % windowSize]);
                    received[expected % windowSize] = false;
                    buffer[expected % windowSize] = null;
                    expected++;
                    delivered++;
                }

                out.println(seq);
            }
        }

        socket.close();
        server.close();
        System.out.println("All packets received. Receiver shutting down.");
    }
}

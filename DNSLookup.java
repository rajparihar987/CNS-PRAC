import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.Scanner;

public class DNSLookup {

    public static void main(String[] args) {
        Scanner scanner = new Scanner(System.in);

        System.out.println("Choose lookup type:");
        System.out.println("1. Hostname to IP");
        System.out.println("2. IP to Hostname");
        System.out.print("Enter choice (1 or 2): ");
        int choice = scanner.nextInt();
        scanner.nextLine(); // consume newline

        try {
            switch (choice) {
                case 1 ->                     {
                        System.out.print("Enter hostname (e.g. www.google.com): ");
                        String hostname = scanner.nextLine();
                        InetAddress[] addresses = InetAddress.getAllByName(hostname);
                        System.out.println("IP addresses for " + hostname + ":");
                        for (InetAddress addr : addresses) {
                            System.out.println(" - " + addr.getHostAddress());
                        }                          }
                case 2 ->                     {
                        System.out.print("Enter IP address (e.g. 8.8.8.8): ");
                        String ip = scanner.nextLine();
                        InetAddress inetAddress = InetAddress.getByName(ip);
                        String hostname = inetAddress.getHostName();
                        System.out.println("Hostname for IP " + ip + ": " + hostname);
                    }
                default -> System.out.println("Invalid choice.");
            }
        } catch (UnknownHostException e) {
            System.out.println("Error: Unable to resolve host/IP. " + e.getMessage());
        } finally {
            scanner.close();
        }
    }
}

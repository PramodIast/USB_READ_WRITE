#include <iostream>
#include <fstream>
#include <string>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <thread>

using namespace std;

// Function to send a CAN message
int send_can_message(const string& can_iface, bool success) {
    struct ifreq ifr;
    struct sockaddr_can addr;
    struct can_frame frame;
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);

    if (sock < 0) {
        cerr << "Error while opening socket" << endl;
        return 1;
    }

    strcpy(ifr.ifr_name, can_iface.c_str()); // Copying the CAN interface name
    if (ioctl(sock, SIOCGIFINDEX, &ifr) < 0) {
        cerr << "Error in ioctl: " << strerror(errno) << endl;
        close(sock);
        return 1;
    }

    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        cerr << "Error in socket bind: " << strerror(errno) << endl;
        close(sock);
        return 1;
    }

    frame.can_id = 0x123; // Example CAN ID
    frame.can_dlc = 1;
    frame.data[0] = success ? 0x01 : 0x00; // 0x01 for pass, 0x00 for fail

    if (write(sock, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
        cerr << "Error writing to CAN socket: " << strerror(errno) << endl;
        close(sock);
        return 1;
    }

    close(sock);
    return 0;
}

int main() {
    string usb_path = "/mnt/sandisk";
    string file_path = usb_path + "/test_file.txt";
    string write_content = "test";
    string read_content;

    // Step 1: Create and write to the file
    ofstream outfile(file_path);
    if (!outfile) {
        cerr << "Error: Unable to create file at " + file_path << endl;
        send_can_message("can0", false); // Report failure over CAN0
        return 1;
    }
    outfile << write_content;
    outfile.close();

    // Step 2: Read the content back
    ifstream infile(file_path);
    if (!infile) {
        cerr << "Error: Unable to open file at " + file_path << endl;
        send_can_message("can0", false); // Report failure over CAN0
        return 1;
    }
    infile >> read_content;
    infile.close();

    // Step 3: Verify the content
    bool success = (read_content == write_content);
    if (success) {
        cout << "Pass: The file content matches the expected string." << endl;
    } else {
        cout << "Fail: The file content does not match the expected string." << endl;
    }

    // Step 4: Send CAN messages every 1 second
    while (true) {
        if (send_can_message("can0", success) != 0) {
            cerr << "Error: Unable to send CAN message" << endl;
            return 1;
        }
        cout << "CAN message sent." << endl;

        // Write to the file again every 1 second
        ofstream outfile(file_path, ios::app); // Open in append mode
        if (!outfile) {
            cerr << "Error: Unable to append to file at " + file_path << endl;
            send_can_message("can0", false); // Report failure over CAN0
            return 1;
        }
        outfile << write_content << endl;
        outfile.close();

        this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}


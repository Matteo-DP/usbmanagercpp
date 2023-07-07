#include <iostream>
#include <libudev.h>
#include <string.h>
#include <vector>
#include <string>
#include <stdio.h>
#include <fstream>
using namespace std;

// For usage in popen()
#define MAX_COMMAND_LENGTH 1024
#define MAX_FILENAME_LENGTH 256

/*
    TODO:
    - Do something with device event
    - Look at what listening to device events does lol
    - Fix evtest path (maybe)
*/

struct device 
{
    const bool completeInfo; // If true, print all device info
    /*
        Strings are used here since char pointers become invalid once the udev device is freed
        Strings cannot be NULL so we must check for it before assinging a value
    */
    string syspath;
    string devnode;
    string vendor;
    string product;
};

class Devices
{
    public:
        bool verbose = false;
        bool debug = false;
        void init();
        void printDevices();
        void cleanup();
    private:
        /*
            A vector is a dynamic array that can be resized
            This is used to store the devices found by udev
            A vector is used here since the number of devices is unknown
        */
        vector<device> deviceArray;
        struct udev* udev;
        struct udev_enumerate* enumerate;
        struct udev_list_entry* devices;
        struct udev_list_entry* entry;
        struct udev_device* dev;
        const char* syspath;
        const char* devnode;
        const char* vendor;
        const char* product;
        bool completeInfo;
        const char* driver;

        void getInfo(const char *syspath);
        void listenForDeviceEvents(const char *syspath);
        void evtest(const char *syspath);
        void attachCustomUdevRule(const char *syspath);
};

void Devices::init()
{
    udev = udev_new();
    if(!udev)
    {
        cerr << "Can't create udev" << endl;
        return;
    }
    debug && cout << "Successfully created udev device" << endl;

    // Create a udev enumeration for USB devices
    enumerate = udev_enumerate_new(udev);
    udev_enumerate_add_match_subsystem(enumerate, "usb");
    udev_enumerate_scan_devices(enumerate);

    // Get a list of devices from the enumeration
    devices = udev_enumerate_get_list_entry(enumerate);
}

void Devices::printDevices()
{
    int i = 0;
    udev_list_entry_foreach(entry, devices)
    {
        syspath = udev_list_entry_get_name(entry);
        debug && cout << "Found device: " << syspath << endl;
        dev = udev_device_new_from_syspath(udev, syspath);
        
        // Get the device attributes
        devnode = udev_device_get_devnode(dev);
        vendor = udev_device_get_sysattr_value(dev, "manufacturer");
        product = udev_device_get_sysattr_value(dev, "product");

        // Print the device information
        if(verbose)
        {
            (devnode || vendor || product) && cout << i << "." << endl;
            devnode && cout << "Device Node: " << devnode << endl;
            vendor && cout << "Vendor: " << vendor << endl;
            product && cout << "Product: " << product << endl;
            (devnode || vendor || product) && cout << "--------------------------------------" << endl;
        }

        completeInfo = (devnode && vendor && product);
        deviceArray.push_back
        ({
            completeInfo, 
            string(syspath),
            devnode ? string(devnode) : "", // If devnode is null, set to empty string (to avoid segfault)
            vendor ? string(vendor) : "", 
            product ? string(product) : ""
        });
        i++;

        // Free the device
        udev_device_unref(dev);
    }
    verbose && cout << "Found " << i << " devices" << endl;
    verbose && cout << endl;
    cout << "AVAILABLE DEVICES: " << endl;
    cout << "------------------" << endl;
    i = 0;
    for(const auto& device : deviceArray) 
    {
        device.completeInfo && cout << " " << i  << ". " << device.product << endl;
        i++;
    }
    cout << "------------------" << endl;
    bool validInput = false;
    while(validInput == false)
    {
        cout << "Select device 0-" << i << ": ";
        int input;
        cin >> input;

        if(input >= 0 && input < deviceArray.size())
        {
            cout << "Selected device: " << deviceArray[input].product << endl << endl;
            validInput = true;
            getInfo(deviceArray[input].syspath.c_str());
        }
        else
        {
            cout << "Invalid input. Select device 0-" << i << endl;
        }
    }
};

void Devices::getInfo(const char *syspath)
{
    dev = udev_device_new_from_syspath(udev, syspath);

    cout << "GENERAL INFO:" << endl;
    udev_device_get_action(dev) && cout << "  action: " << udev_device_get_action(dev) << endl;
    udev_device_get_devnode(dev) && cout << "  node: " << udev_device_get_devnode(dev) << endl;
    udev_device_get_subsystem(dev) && cout << "  subsystem: " << udev_device_get_subsystem(dev) << endl;
    udev_device_get_devtype(dev) && cout << "  devtype: " << udev_device_get_devtype(dev) << endl;
    udev_device_get_sysattr_value(dev, "manufacturer") && cout << "  vendor:" << udev_device_get_sysattr_value(dev, "manufacturer") << endl;
    udev_device_get_sysattr_value(dev, "product") && cout << "  product:" << udev_device_get_sysattr_value(dev, "product") << endl;
    udev_device_get_sysattr_value(dev, "type") && cout << "  type:" << udev_device_get_sysattr_value(dev, "type") << endl;
    udev_device_get_sysattr_value(dev, "name") && cout << "  name:" << udev_device_get_sysattr_value(dev, "name") << endl;
    udev_device_get_sysattr_value(dev, "id") && cout << "  id:" << udev_device_get_sysattr_value(dev, "id") << endl;
    udev_device_get_sysattr_value(dev, "serial") && cout << "  serial:" << udev_device_get_sysattr_value(dev, "serial") << endl;
    udev_device_get_sysattr_value(dev, "modalias") && cout << "  modalias:" << udev_device_get_sysattr_value(dev, "modalias") << endl;
    udev_device_get_devpath(dev) && cout << "  devpath:" << udev_device_get_devpath(dev) << endl;
    udev_device_get_syspath(dev) && cout << "  syspath:" << udev_device_get_syspath(dev) << endl;
    udev_device_get_sysname(dev) && cout << "  sysname:" << udev_device_get_sysname(dev) << endl;
    !verbose && cout << endl;
    // Advanced info
    verbose && udev_device_get_sysnum(dev) && cout << "  sysnum:" << udev_device_get_sysnum(dev) << endl;
    verbose && udev_device_get_devnum(dev) && cout << "  devnum:" << udev_device_get_devnum(dev) << endl;
    verbose && udev_device_get_seqnum(dev) && cout << "  seqnum:" << udev_device_get_seqnum(dev) << endl;
    verbose && udev_device_get_is_initialized(dev) && cout << "  is_initialized:" << udev_device_get_is_initialized(dev) << endl;
    verbose && udev_device_get_devlinks_list_entry(dev) && cout << "  devlinks:" << udev_device_get_devlinks_list_entry(dev) << endl;
    verbose && udev_device_get_properties_list_entry(dev) && cout << "  properties:" << udev_device_get_properties_list_entry(dev) << endl;
    verbose && udev_device_get_tags_list_entry(dev) && cout << "  tags:" << udev_device_get_tags_list_entry(dev) << endl;
    verbose && udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device") && cout << "  parent:" << udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device") << endl;
    verbose && udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device") && cout << "  parent with subsystem devtype:" << udev_device_get_parent_with_subsystem_devtype(dev, "usb", "usb_device") << endl << endl;

    driver = udev_device_get_driver(dev);
    cout << "DRIVER INFO:" << endl;
    driver && cout << "Driver: " << driver << endl << endl;

    udev_device_unref(dev);

    cout << "ACTIONS:" << endl;
    cout << "--------------------------------------" << endl;
    cout << "0. Exit" << endl;
    cout << "1. Listen for device events (kernel)" << endl;
    cout << "2. Listen for device events (HID input, requires evtest)" << endl;
    cout << "3. Attach custom udev rule" << endl;
    cout << "--------------------------------------" << endl;

    int input;
    bool validInput = false;
    while(validInput == false)
    {
        cout << "Select action 0-3: ";
        cin >> input;

        switch(input)
        {
            case 0:
                printDevices();
            case 1:
                listenForDeviceEvents(syspath);
                validInput = true;
                break;
            case 2:
                evtest(syspath);
                validInput = true;
                break;
            case 3:
                attachCustomUdevRule(syspath);
                validInput = true;
                break;
            default:
                cout << "Invalid input" << endl;
                break;
        }
    }
}

void Devices::evtest(const char *syspath)
{
    // Run evtest with symbolic link in /dev/input/by-path
    // Requires root privileges & evtest
    // Wacky and unreliable, works on my machine, untested on others.

    /*
        devpath:/devices/pci0000:00/0000:00:08.1/0000:04:00.4/usb3/3-2/3-2.4
        syspath:/sys/devices/pci0000:00/0000:00:08.1/0000:04:00.4/usb3/3-2/3-2.4

        lrwxrwxrwx 1 root root 10 jul  7 11:27 pci-0000:04:00.4-usb-0:2.4:1.0-event -> ../event26
        lrwxrwxrwx 1 root root 10 jul  7 11:27 pci-0000:04:00.4-usb-0:2.4:1.0-event-kbd -> ../event20
    */

    // Get the device path
    string path = string(syspath);

    debug && cout << "path: " << path << endl;
    string pci = path.substr(path.find_first_of("p"), path.find_first_of(":") + 1 - path.find_first_of("p")); // pci0000:
    pci.insert(pci.find_first_of("i") + 1, "-"); // pci-0000:
    debug && cout << "pci: " << pci << endl;

    string misc = path.substr(path.find_first_of("u") - 8, 7); // 04:00.4
    misc.insert(misc.length(), "-"); // 04:00.4-
    debug && cout << "misc: " << misc << endl;

    /* TODO: not hard code usb-X path */
    string usb = "usb-0"; // usb-0
    usb.insert(usb.length(), ":"); // usb-0:
    debug && cout << "usb: " << usb << endl;

    string misc2 = path.substr(path.length() - 3, 3); // 2.4
    misc2.insert(misc2.length(), ":"); // 2.4:
    debug && cout << "misc2: " << misc2 << endl;

    /* TODO: not hard code 1.X */
    string misc3 = "1.0"; // 1.0
    misc3.insert(misc3.length(), "-"); // 1.0-
    debug && cout << "misc3: " << misc3 << endl;

    string event = pci + misc + usb + misc2 + misc3 + "event"; // pci-0000:04:00.4-usb-0:2.4:1.0-event
    (verbose || debug) && cout << "event: " << event << endl;

    FILE* fp = popen(("ls -s /dev/input/by-path/" + event + "*").c_str(), "r");
    if (fp == NULL)
    {
        cout << "Failed to find symbolic link: failed to execute 'ls' command" << endl;
        return;
    }

    char filename[MAX_FILENAME_LENGTH];
    vector<string> filenames; // List with dynamic size for all filenames
    while (fgets(filename, MAX_FILENAME_LENGTH, fp)) {
        string filenameStr(filename);
        filenameStr.erase(0, 2); // Erase the leading number and space
        filenameStr.erase(filenameStr.find_last_not_of(" \n\r\t") + 1); // Erase trailing whitespace

        debug && cout << "filename: " << filenameStr << endl;
        filenames.push_back(filenameStr);
    }
    pclose(fp);

    if(filenames.size() == 0)
    {
        cout << "Failed to find symbolic link: no symbolic links found" << endl;
        return;
    }

    string symbolicLink;
    bool validInput = false;
    int input;
    if(filenames.size() > 1)
    {
        cout << endl << "MULTIPLE SYMBOLIC LINKS FOUND" << endl;
        cout << "Select symbolic link to use:" << endl;
        cout << "--------------------------------------" << endl;
        for (int i = 0; i < filenames.size(); i++)
        {
            cout << i + 1 << ". " << filenames[i] << endl;
        }
        cout << "--------------------------------------" << endl;
        while(validInput == false)
        {
            cout << "Select link 1-" << filenames.size() - 1 + 1 << ": ";
            cin >> input;
            if(input > filenames.size() - 1 + 1 || input < 0 + 1)
            {
                cout << "Invalid input" << endl;
            }
            else
            {
                validInput = true;
            };
        }
        cout << "Using symbolic link: " << filenames[input - 1] << endl << endl;
        symbolicLink = filenames[input - 1];
    }
    else
    {
        symbolicLink = filenames[0];
    };

    system(("evtest " + symbolicLink).c_str());
}

void Devices::listenForDeviceEvents(const char *syspath)
{
    dev = udev_device_new_from_syspath(udev, syspath);
    devnode = udev_device_get_devnode(dev);
    vendor = udev_device_get_sysattr_value(dev, "manufacturer");
    product = udev_device_get_sysattr_value(dev, "product");

    cout << "Listening for device events (generated by kernel or udev)..." << endl;

    udev_device_unref(dev);

    // Create a monitor handle to listen for events
    struct udev_monitor* mon = udev_monitor_new_from_netlink(udev, "udev");
    udev_monitor_filter_add_match_subsystem_devtype(mon, "usb", NULL);
    udev_monitor_enable_receiving(mon);

    // Get the file descriptor (fd) for the monitor
    int fd = udev_monitor_get_fd(mon);

    // Create a file descriptor set (fds) and add the monitor fd to it
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    // Create a timeout for the select() function
    struct timeval tv;
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    // Listen for events
    while(true)
    {
        // Select() blocks execution until an event is received
        select(fd+1, &fds, NULL, NULL, &tv);

        // Check if the monitor fd is ready to read
        if(FD_ISSET(fd, &fds))
        {
            // Receive the device event
            dev = udev_monitor_receive_device(mon);
            if(dev)
            {
                cout << "Got device event" << endl;
                // TODO: Do something with device event                
            }
            else
            {
                cout << "No Device from receive_device(). An error occured." << endl;
            }
        }
    }         
}

void Devices::attachCustomUdevRule(const char *syspath)
{
    /*
        ACTION=="add" KERNEL=="sd*", ATTRS{vendor}=="Yoyodyne", ATTRS{model}=="XYZ42", ATTRS{serial}=="123465789", RUN+="/pathto/script"
        /lib/udev/rules.d/X.rules
        udevadm control --reload-rules

        Scripts ran by udev are blocking for the udev daemon
        It is recommended to fork long running tasks
    */

    int input;

    cout << endl << "Attach custom udev rule" << endl;
    cout << "--------------------------------------" << endl;
    cout << "0. Exit" << endl;
    cout << "1. Run script upon USB device plug-in" << endl;
    cout << "--------------------------------------" << endl;
    cout << "Select option 0-1: ";
    cin >> input;

    bool validUserInput = false;
    while(validUserInput == false)
    {
        switch(input)
        {
            case 0:
                printDevices();
                validUserInput = true;
                break;
            case 1:
                validUserInput = true;
                break;
            default:
                cout << "Invalid input" << endl;
                break;
        }
    };

    cout << "Selected option: run script upon USB device plug-in" << endl << endl;
    cout << "--------------------------------------" << endl;
    cout << "Description: " << endl;
    cout << "Creates a custom udev rule that runs a script upon USB device plug-in" << endl;
    cout << "Warning: Scripts ran by udev are blocking for the udev daemon, it is recommended to fork long-running scripts" << endl;
    cout << "--------------------------------------" << endl << endl;

    /*
        1. Create file /etc/udev/rules.d/99-usb.rules or open file
        2. Add rule to file
        3. Reload rules
    */

    const char* filename = "/etc/udev/rules.d/99-usb.rules"; // Filename for udev rule
    ofstream file; // Create and writes to files

    dev = udev_device_new_from_syspath(udev, syspath);
    vendor = udev_device_get_sysattr_value(dev, "manufacturer");
    const char* model = udev_device_get_sysattr_value(dev, "product");
    const char* serial = udev_device_get_sysattr_value(dev, "serial");

    // Get user input
    string path;
    bool validInput = false;
    while(validInput == false)
    {
        cout << "Enter path to script: ";
        getline(cin, path);

        // Check path validity
        if(!ifstream(path)) // Reads from files
        {
            cout << "Invalid path: " << path << endl;
        }
        else
        {
            validInput = true;
        }
    }

    file.open(filename, ios::app);
    if(file.is_open())
    {
        file << "# USB device rule: run script upon USB device plug-in" << endl;
        file << "ACTION==\"add\" KERNEL==\"sd*\", ATTRS{vendor}==\"" << vendor << "\", ATTRS{model}==\"" << model << "\", ATTRS{serial}==\"" << serial << "\", RUN+=\"" << path << "\"" << endl;
        file.close();
    }
    else
    {
        cout << "Failed to open file " << filename << endl;
        udev_device_unref(dev);
        return;
    }
    
    // Reload rules
    system("sudo udevadm control --reload-rules");

    cout << "Successfully created udev rule at " << filename << endl;
    cout << "Action: run script at " << path << " upon USB device " << model << " plug-in" << endl;
    cout << "Reloaded rules" << endl;
    udev_device_unref(dev);
    return;
}

void Devices::cleanup()
{
    // Cleanup
    udev_enumerate_unref(enumerate);
    udev_unref(udev);
};

int main(int argv, char* argc[])
{
    Devices devices;

    // Parse command line arguments
    for(int i = 0; i < argv; i++)
    {
        if(strcmp(argc[i], "-v") == 0)
        {
            devices.verbose = true;
        }
        if(strcmp(argc[i], "-d") == 0)
        {
            devices.debug = true;
        }
    }

    devices.init();
    devices.printDevices();
    devices.cleanup();

    return 0;
}
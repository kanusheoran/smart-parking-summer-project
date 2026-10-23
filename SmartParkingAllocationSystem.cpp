
#include <iostream>
#include <unordered_map>
#include <queue>
#include <vector>
#include <string>
#include <iomanip>
#include <ctime>
using namespace std;

// ---------- Slot structure (Array element) ----------
struct Slot {
    int id;
    bool occupied;
    string vehicleNo;
    time_t entryTime;
    double totalOccupiedSeconds; // cumulative usage, for utilization report

    Slot() : id(0), occupied(false), vehicleNo(""), entryTime(0), totalOccupiedSeconds(0) {}
};

class ParkingSystem {
private:
    vector<Slot> slots;                          // Array of slots
    unordered_map<string, int> vehicleToSlot;    // Hashing: vehicleNo -> slot index
    queue<string> waitlist;                      // FIFO waitlist
    vector<int> hourlyEntries;                   // index 0-23, count of entries per hour (peak-hour analysis)

    int findFreeSlot() {
        for (int i = 0; i < (int)slots.size(); i++) {
            if (!slots[i].occupied) return i;
        }
        return -1; // lot full
    }

public:
    ParkingSystem(int totalSlots) {
        slots.resize(totalSlots);
        for (int i = 0; i < totalSlots; i++) slots[i].id = i + 1;
        hourlyEntries.assign(24, 0);
    }

    // Vehicle arrives: allocate a slot, or waitlist if lot is full
    void vehicleArrival(const string& vehicleNo) {
        if (vehicleToSlot.count(vehicleNo)) {
            cout << "[INFO] Vehicle " << vehicleNo << " is already parked.\n";
            return;
        }

        int idx = findFreeSlot();
        if (idx == -1) {
            waitlist.push(vehicleNo);
            cout << "[FULL] No free slot. Vehicle " << vehicleNo
                 << " added to waitlist (position " << waitlist.size() << ").\n";
            return;
        }
        allocate(idx, vehicleNo);
    }

    // Vehicle leaves: free the slot using hash lookup, then serve waitlist
    void vehicleDeparture(const string& vehicleNo) {
        auto it = vehicleToSlot.find(vehicleNo);
        if (it == vehicleToSlot.end()) {
            cout << "[ERROR] Vehicle " << vehicleNo << " not found in parking lot.\n";
            return;
        }

        int idx = it->second;
        Slot& s = slots[idx];
        double duration = difftime(time(nullptr), s.entryTime);
        s.totalOccupiedSeconds += duration;

        cout << "[EXIT] Vehicle " << vehicleNo << " left Slot " << s.id
             << " after " << (int)duration << " sec.\n";

        s.occupied = false;
        s.vehicleNo = "";
        vehicleToSlot.erase(it);

        // Serve next waitlisted vehicle (FIFO)
        if (!waitlist.empty()) {
            string nextVehicle = waitlist.front();
            waitlist.pop();
            cout << "[WAITLIST] Allocating freed Slot " << s.id
                 << " to waitlisted vehicle " << nextVehicle << ".\n";
            allocate(idx, nextVehicle);
        }
    }

    void showStatus() {
        cout << "\n----- Current Slot Status -----\n";
        for (auto& s : slots) {
            cout << "Slot " << setw(2) << s.id << ": "
                 << (s.occupied ? ("Occupied by " + s.vehicleNo) : "Free") << "\n";
        }
        cout << "Waitlist size: " << waitlist.size() << "\n";
        cout << "--------------------------------\n";
    }

    // Utilization + peak-hour report
    void generateReport() {
        cout << "\n===== Utilization Report =====\n";
        for (auto& s : slots) {
            cout << "Slot " << s.id << ": total occupied time = "
                 << (int)s.totalOccupiedSeconds << " sec\n";
        }

        int peakHour = 0;
        for (int h = 1; h < 24; h++)
            if (hourlyEntries[h] > hourlyEntries[peakHour]) peakHour = h;

        cout << "\nEntries by hour (24h format):\n";
        for (int h = 0; h < 24; h++) {
            if (hourlyEntries[h] > 0)
                cout << setw(2) << setfill('0') << h << ":00 -> "
                     << hourlyEntries[h] << " entries\n";
        }
        cout << setfill(' ');
        cout << "\nPeak hour: " << setw(2) << setfill('0') << peakHour
             << ":00 with " << hourlyEntries[peakHour] << " entries\n";
        cout << setfill(' ') << "===============================\n";
    }

private:
    void allocate(int idx, const string& vehicleNo) {
        Slot& s = slots[idx];
        s.occupied = true;
        s.vehicleNo = vehicleNo;
        s.entryTime = time(nullptr);
        vehicleToSlot[vehicleNo] = idx;

        tm* lt = localtime(&s.entryTime);
        hourlyEntries[lt->tm_hour]++;

        cout << "[ENTRY] Vehicle " << vehicleNo << " allocated Slot " << s.id << ".\n";
    }
};

// ---------- Demo Driver ----------
int main() {
    int totalSlots = 3; // small number so it's easy to demo waitlist behavior
    ParkingSystem lot(totalSlots);

    int choice;
    string vehicleNo;

    cout << "===== Smart Parking Slot Allocation System =====\n";
    cout << "Total slots available: " << totalSlots << "\n";

    while (true) {
        cout << "\n1. Vehicle Arrival\n2. Vehicle Departure\n3. Show Status\n"
             << "4. Generate Report\n5. Exit\nEnter choice: ";
        if (!(cin >> choice)) break;

        switch (choice) {
            case 1:
                cout << "Enter vehicle number: ";
                cin >> vehicleNo;
                lot.vehicleArrival(vehicleNo);
                break;
            case 2:
                cout << "Enter vehicle number: ";
                cin >> vehicleNo;
                lot.vehicleDeparture(vehicleNo);
                break;
            case 3:
                lot.showStatus();
                break;
            case 4:
                lot.generateReport();
                break;
            case 5:
                cout << "Exiting system.\n";
                return 0;
            default:
                cout << "Invalid choice.\n";
        }
    }
    return 0;
}

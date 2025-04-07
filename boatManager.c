#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#define MAX_LINE_LENGTH 1024
#define MAX_BOATS 120
#define MAX_NAME_LEN 128
#define MAX_LICENSE_LEN 16

typedef enum {
    slip,
    land,
    trailor,
    storage,
    no_place
} PlaceType;

// Union for the different place types with their specific information
typedef union {
    int slip_number;      // For slip: 1-85
    char bay_letter;      // For land: A-Z
    char license[MAX_LICENSE_LEN]; // For trailor: license tag
    int storage_number;   // For storage: 1-50
} PlaceInfo;

// Boat structure
typedef struct {
    char name[MAX_NAME_LEN];
    int length;
    PlaceType type;
    PlaceInfo info;
    double amount_owed;
} Boat;

// Global variables
Boat* boats[MAX_BOATS];
int boat_count = 0;
char* filename;

//-------------------------------------------------------------------------------------------------
//----Convert a string to a place
PlaceType StringToPlaceType(char* PlaceString) {
    if (!strcasecmp(PlaceString, "slip")) {
        return slip;
    }
    if (!strcasecmp(PlaceString, "land")) {
        return land;
    }
    if (!strcasecmp(PlaceString, "trailor")) {
        return trailor;
    }
    if (!strcasecmp(PlaceString, "storage")) {
        return storage;
    }
    return no_place;
}
//-------------------------------------------------------------------------------------------------
//----Convert a place to a string
char* PlaceToString(PlaceType Place) {
    switch (Place) {
        case slip:
            return "slip";
        case land:
            return "land";
        case trailor:
            return "trailor";
        case storage:
            return "storage";
        case no_place:
            return "no_place";
        default:
            printf("How the faaark did I get here?\n");
            exit(EXIT_FAILURE);
    }
}
//-------------------------------------------------------------------------------------------------
// Find a boat's index by name (case insensitive)
int findBoatIndexByName(const char* name) {
    for (int i = 0; i < boat_count; i++) {
        if (strcasecmp(boats[i]->name, name) == 0) {
            return i;
        }
    }
    return -1;
}

//-------------------------------------------------------------------------------------------------
// Compare function for qsort to sort boats by name
int compareBoats(const void* a, const void* b) {
    Boat* boatA = *(Boat**)a;
    Boat* boatB = *(Boat**)b;
    return strcasecmp(boatA->name, boatB->name);
}

//-------------------------------------------------------------------------------------------------
// Load data from CSV file
bool loadBoatsFromFile() {
    FILE* file = fopen(filename, "r");
    if (!file) {
        // If file doesn't exist, it's not an error - we'll create it later
        return true;
    }

    char line[MAX_LINE_LENGTH];
    char type_str[32];
    char extra_info[MAX_LICENSE_LEN];
    
    while (fgets(line, MAX_LINE_LENGTH, file) && boat_count < MAX_BOATS) {
        // Remove newline
        line[strcspn(line, "\n")] = 0;
        
        Boat* new_boat = (Boat*)malloc(sizeof(Boat));
        if (!new_boat) {
            printf("Memory allocation failed\n");
            fclose(file);
            return false;
        }
        
        // Parse the line
        if (sscanf(line, "%127[^,],%d,%31[^,],%15[^,],%lf", 
                  new_boat->name, &new_boat->length, type_str, 
                  extra_info, &new_boat->amount_owed) != 5) {
            printf("Error parsing line: %s\n", line);
            free(new_boat);
            continue;
        }
        
        // Set the place type
        new_boat->type = StringToPlaceType(type_str);
        
        // Set the place info based on type
        switch (new_boat->type) {
            case slip:
                new_boat->info.slip_number = atoi(extra_info);
                break;
            case land:
                new_boat->info.bay_letter = extra_info[0];
                break;
            case trailor:
                strncpy(new_boat->info.license, extra_info, MAX_LICENSE_LEN - 1);
                new_boat->info.license[MAX_LICENSE_LEN - 1] = '\0';
                break;
            case storage:
                new_boat->info.storage_number = atoi(extra_info);
                break;
            default:
                printf("Invalid place type in file: %s\n", type_str);
                free(new_boat);
                continue;
        }
        
        boats[boat_count++] = new_boat;
    }
    
    fclose(file);
    
    // Sort the boats by name
    qsort(boats, boat_count, sizeof(Boat*), compareBoats);
    
    return true;
}

//-------------------------------------------------------------------------------------------------
// Save data to CSV file
bool saveBoatsToFile() {
    FILE* file = fopen(filename, "w");
    if (!file) {
        printf("Error: Could not open file %s for writing\n", filename);
        return false;
    }
    
    for (int i = 0; i < boat_count; i++) {
        Boat* b = boats[i];
        char extra_info[MAX_LICENSE_LEN];
        
        // Format extra info based on type
        switch (b->type) {
            case slip:
                sprintf(extra_info, "%d", b->info.slip_number);
                break;
            case land:
                sprintf(extra_info, "%c", b->info.bay_letter);
                break;
            case trailor:
                strncpy(extra_info, b->info.license, MAX_LICENSE_LEN - 1);
                extra_info[MAX_LICENSE_LEN - 1] = '\0';
                break;
            case storage:
                sprintf(extra_info, "%d", b->info.storage_number);
                break;
            default:
                strcpy(extra_info, "");
                break;
        }
        
        fprintf(file, "%s,%d,%s,%s,%.2f\n", 
                b->name, b->length, PlaceToString(b->type), 
                extra_info, b->amount_owed);
    }
    
    fclose(file);
    return true;
}

//-------------------------------------------------------------------------------------------------
// Print the boat inventory
void printBoatInventory() {
    for (int i = 0; i < boat_count; i++) {
        Boat* b = boats[i];
        printf("%-22s %2d' ", b->name, b->length);
        
        // Format the place information
        switch (b->type) {
            case slip:
                printf("%7s   # %2d", PlaceToString(b->type), b->info.slip_number);
                break;
            case land:
                printf("%7s      %c", PlaceToString(b->type), b->info.bay_letter);
                break;
            case trailor:
                printf("%7s %6s", PlaceToString(b->type), b->info.license);
                break;
            case storage:
                printf("%7s   # %2d", PlaceToString(b->type), b->info.storage_number);
                break;
            default:
                printf("%7s", PlaceToString(b->type));
                break;
        }
        
        printf("   Owes $%6.2f\n", b->amount_owed);
    }
}

//-------------------------------------------------------------------------------------------------
// Add a boat to the inventory
bool addBoatToInventory() {
    if (boat_count >= MAX_BOATS) {
        printf("Error: Boat inventory is full\n");
        return false;
    }
    
    char input[MAX_LINE_LENGTH];
    char name[MAX_NAME_LEN];
    char type_str[32];
    char extra_info[MAX_LICENSE_LEN];
    int length;
    double amount_owed;

    // Prompt user
    printf("Please enter the boat data in CSV format                 : ");
    
    // Clear any remaining input from the buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (!fgets(input, sizeof(input), stdin)) {
        printf("Input error.\n");
        return false;
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Parse input
    int parsed = sscanf(input, "%127[^,],%d,%31[^,],%15[^,],%lf", 
                       name, &length, type_str, extra_info, &amount_owed);

    if (parsed != 5) {
        printf("Invalid format. Please try again.\n");
        return false; 
    }

    // Validate
    if (length < 1 || length > 100) {
        printf("Invalid boat length. Must be 1-100.\n");
        return false;
    }

    PlaceType type = StringToPlaceType(type_str);
    if (type == no_place) {
        printf("Invalid place type.\n");
        return false;
    }
    
    // Create a new boat
    Boat* new_boat = (Boat*)malloc(sizeof(Boat));
    if (!new_boat) {
        printf("Memory allocation failed\n");
        return false;
    }
    
    // Set basic boat information
    strncpy(new_boat->name, name, MAX_NAME_LEN - 1);
    new_boat->name[MAX_NAME_LEN - 1] = '\0';
    new_boat->length = length;
    new_boat->type = type;
    new_boat->amount_owed = amount_owed;
    
    // Validate and set extra info based on type
    switch (type) {
        case slip: {
            int slip_num = atoi(extra_info);
            if (slip_num < 1 || slip_num > 85) {
                printf("Invalid slip number.\n");
                free(new_boat);
                return false;
            }
            new_boat->info.slip_number = slip_num;
            break;
        }
        case land: {
            if (strlen(extra_info) != 1 || extra_info[0] < 'A' || extra_info[0] > 'Z') {
                printf("Invalid bay letter.\n");
                free(new_boat);
                return false;
            }
            new_boat->info.bay_letter = extra_info[0];
            break;
        }
        case storage: {
            int storage_num = atoi(extra_info);
            if (storage_num < 1 || storage_num > 50) {
                printf("Invalid storage number.\n");
                free(new_boat);
                return false;
            }
            new_boat->info.storage_number = storage_num;
            break;
        }
        case trailor:
            strncpy(new_boat->info.license, extra_info, MAX_LICENSE_LEN - 1);
            new_boat->info.license[MAX_LICENSE_LEN - 1] = '\0';
            break;
        default:
            printf("Invalid place type.\n");
            free(new_boat);
            return false;
    }
    
    // Add the boat to the inventory and sort
    boats[boat_count++] = new_boat;
    qsort(boats, boat_count, sizeof(Boat*), compareBoats);
    
    return true;
}

//-------------------------------------------------------------------------------------------------
// Remove a boat from the inventory
bool removeBoatFromInventory() {
    char name[MAX_NAME_LEN];
    
    // Prompt user for boat name
    printf("Please enter the boat name                               : ");
    
    // Clear any remaining input from the buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (!fgets(name, sizeof(name), stdin)) {
        printf("Input error.\n");
        return false;
    }
    
    // Remove newline
    name[strcspn(name, "\n")] = 0;
    
    // Find the boat
    int idx = findBoatIndexByName(name);
    if (idx == -1) {
        printf("No boat with that name\n");
        return false;
    }
    
    // Free the boat memory
    free(boats[idx]);
    
    // Shift all boats after idx down by one
    for (int i = idx; i < boat_count - 1; i++) {
        boats[i] = boats[i + 1];
    }
    
    boat_count--;
    return true;
}

//-------------------------------------------------------------------------------------------------
// Accept a payment for a boat
void acceptPayment() {
    char name[MAX_NAME_LEN];
    double amount;

    printf("Please enter the boat name                               : ");
    
    // Clear any remaining input from the buffer
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    
    if (!fgets(name, sizeof(name), stdin)) {
        printf("Input error.\n");
        return;
    }
    
    // Remove newline
    name[strcspn(name, "\n")] = 0;

    int idx = findBoatIndexByName(name);
    if (idx == -1) {
        printf("No boat with that name\n");
        return;
    }

    printf("Please enter the amount to be paid                       : ");
    scanf("%lf", &amount);

    if (amount > boats[idx]->amount_owed) {
        printf("That is more than the amount owed, $%.2f\n", boats[idx]->amount_owed);
        return;
    }

    boats[idx]->amount_owed -= amount;
}

//-------------------------------------------------------------------------------------------------
// Update monthly charges for all boats
void updateMonthlyCharges() {
    for (int i = 0; i < boat_count; i++) {
        Boat* b = boats[i];
        double rate = 0.0;
        
        switch (b->type) {
            case slip: rate = 12.50; break;
            case land: rate = 14.00; break;
            case trailor: rate = 25.00; break;
            case storage: rate = 11.20; break;
            default: break;
        }
        
        b->amount_owed += rate * b->length;
    }
}

//-------------------------------------------------------------------------------------------------
// Main function
int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        printf("Usage: %s <csv_file>\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    // Set the filename
    filename = argv[1];
    
    // Load boats from the CSV file
    if (!loadBoatsFromFile()) {
        printf("Error loading boats from file %s\n", filename);
        return EXIT_FAILURE;
    }
    
    printf("\nWelcome to the Boat Management System\n");
    printf("-------------------------------------\n\n");
    
    char choice;
    bool exited = false;
    
    while (!exited) {
        printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
        scanf(" %c", &choice);
        
        // Convert choice to lowercase
        choice = tolower(choice);
        
        switch (choice) { 
            case 'i': 
                printBoatInventory();
                break;
            case 'a':
                addBoatToInventory();
                break; 
            case 'r':
                removeBoatFromInventory();
                break;
            case 'p':
                acceptPayment();
                break;
            case 'm':
                updateMonthlyCharges();
                break;        
            case 'x':
                printf("\nExiting the Boat Management System\n");
                
                // Save data to file
                saveBoatsToFile();
                
                // Free allocated memory
                for (int i = 0; i < boat_count; i++) {
                    free(boats[i]);
                }
                
                exited = true;
                break;
            default:
                printf("Invalid option %c\n", choice);
        }
        
        printf("\n");
    }
    
    return EXIT_SUCCESS;
}

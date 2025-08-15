# include <stdio.h>
# include <string.h>
# include <time.h>
# include <stdlib.h>

# define NUM_LEN 20
# define NAME_LEN 50
# define TIME_LEN 50
# define STATUS_LEN 10
# define SPACE_DB_LEN 50
# define VEHICLE_DB_LEN 100
# define VEHICLE_FILE "vehicle_data.txt"
# define PARKING_FILE "parking_data.txt"


typedef enum {FAILURE , SUCCESS }statusCode;
typedef enum {FALSE , TRUE}boolean;
typedef enum {BY_PARKINGS , BY_AMT_PAID}sort_vehicle;
typedef enum {BY_OCCUPANCY , BY_MAX_REVENUE}sort_spaces;

typedef struct vehicle_details
{
     char vehicle_number[NUM_LEN];
     char owner_name[NUM_LEN];
     time_t arrival_time;
     time_t departure_time;
     char membership[STATUS_LEN];
     long int total_parking_hours;
     int parking_space_ID;
     int parking_count;
     double total_amount_paid;
     struct vehicle_details *next;
}vehicle;

typedef struct parking_space_status
{
     int space_ID;
     int status;
     int occupancy_count;
     double revenue_generated;
     struct parking_space_status *next;
}space;

// creating a node while inserting vehicle details for a new vehicle
statusCode createVehicleNode(char *number , char *name , time_t time , int id , vehicle **newVehicle)
{
    statusCode sc = SUCCESS;
    *newVehicle = (vehicle *)malloc(sizeof(vehicle));
    if(*newVehicle == NULL)
    {
        sc = FAILURE;
    }
    else
    {
        strcpy((*newVehicle) -> vehicle_number , number);
        strcpy((*newVehicle) -> owner_name , name);
        (*newVehicle) -> arrival_time = time;
        strcpy((*newVehicle) -> membership , "NULL");
        (*newVehicle) -> total_parking_hours = 0;
        (*newVehicle) -> parking_count = 1;
        (*newVehicle) -> total_amount_paid = 0;
        (*newVehicle) -> parking_space_ID = id;
        (*newVehicle) -> next = NULL;
    }
    return sc;
}

space *findByMembership(space *head)
{
    int found = 0;
    space *space = NULL, *curr = head;
    while(curr != NULL && found == 0)
    {
        if(curr -> status == 0)//space is free
        {
            space = curr;
            found = 1;
        }
        curr = curr -> next;
    }
    return space;
}

vehicle *searchVehicle(char *number , vehicle *vptr)
{
    vehicle *curr = vptr;
    vehicle *vehicle = NULL;
    int found = 0;
    while(curr != NULL)
    {
        if((strcmp(curr -> vehicle_number , number) == 0))
        {
            found = 1;
            vehicle = curr;
        }
        curr = curr -> next;
    }
    return  vehicle;
}

space *findSpace(vehicle **vehicleData , char *number , space *golden , space *silver , space *general , vehicle *vptr)
{
    *vehicleData = searchVehicle(number , vptr);
    space *space = NULL;
    if(*vehicleData != NULL)//vehicle is already registered
    {
        //check for membership
        if(strcmp((*vehicleData) -> membership , "GOLDEN") == 0)
        {
           space = findByMembership(golden);
        }
        else if(strcmp((*vehicleData) -> membership , "SILVER") == 0)
        {
            space = findByMembership(silver);
        }
        else
        {
            space = findByMembership(general);
        }
    }
    else // vehicle not registered
    {
        space = findByMembership(general);
    }
    return space;
}

statusCode insertAtStart(vehicle **head , vehicle *newVehicle , char *number , char *name ,time_t time , int id)
{
    statusCode sc;
    sc = createVehicleNode(number , name , time , id , &newVehicle);
    if(sc != FAILURE)
    {
        newVehicle -> next = *head;
        *head = newVehicle;
    }
    return sc;
}

void updateDetails(time_t time , vehicle *vehicle , space *space)
{
    vehicle -> arrival_time = time;
    vehicle -> parking_space_ID = space -> space_ID;
    vehicle -> parking_count += 1;
    space -> occupancy_count +=1;
    space -> status = 1;
}

statusCode updateInsertVehicle(char *number , char *name , time_t time , space *golden , space *silver , space *general , vehicle **head)
{
    //first check if the space exists for the vehicle
    statusCode sc = SUCCESS;
    vehicle *newVehicle = NULL;
    vehicle *vehicle = NULL;//vehicle node at which vehicle data is present if vehicle is already registered
    space *freeSpace = findSpace(&vehicle , number , golden , silver , general , *head);
    //index at which vehicle data is found
    if(freeSpace != NULL)
    {
        if(vehicle != NULL)//vechicle is already registerd
        {
            updateDetails(time , vehicle , freeSpace);
            printf("Vehicle details updated successfully with space id : %d\n",freeSpace -> space_ID);
        }
        else
        {   
            sc = insertAtStart(head , newVehicle , number , name , time , freeSpace -> space_ID);
            printf("Vehicle details inserted successfully with space id : %d\n",freeSpace -> space_ID);
        	freeSpace -> status = 1; // now space is occupied
        	freeSpace -> occupancy_count = freeSpace -> occupancy_count + 1;
    	}
    }
    
    else//space not found
    {
        sc = FAILURE;
        printf("Parking space exhausted can not insert or update the information.\n");
    }
    return sc;
}

space* findSpaceByID(int id , space *head , int start)
{
    space *curr = head;
    int i = start;
    while(curr && i < id)
    {
        curr = curr -> next;
        i++;
    }
    return curr;
}

time_t convertToTimeT(const char *timeString) 
{
    struct tm timeInfo;
    memset(&timeInfo, 0, sizeof(struct tm));  // Initialize struct to zero
    
    // Parse the input string
    if (sscanf(timeString, "%d-%d-%d_%d:%d", 
               &timeInfo.tm_year, &timeInfo.tm_mon, 
               &timeInfo.tm_mday, &timeInfo.tm_hour, &timeInfo.tm_min) != 5) {
        printf("Invalid time format!\n");
        return -1;  // Return -1 to indicate error
    }
    
    timeInfo.tm_year -= 1900; // Adjust year (tm_year is years since 1900)
    timeInfo.tm_mon -= 1;     // Adjust month (tm_mon is 0-based)
    timeInfo.tm_sec = 0;      // Set seconds to 0

    return mktime(&timeInfo); // Convert to time_t
}

vehicle* loadVehiclesFromFile() 
{
    FILE *file = fopen(VEHICLE_FILE, "r");
    if (!file) 
	{
        printf("No vehicle data found. Starting fresh.\n");
        return NULL;
    }

    vehicle *head = NULL, *newVehicle;
    char number[NUM_LEN], name[NAME_LEN], membership[STATUS_LEN];
    int spaceID, count;
    long int hours;
    double amount;
    time_t arrival, departure;
    char time1[TIME_LEN],time2[TIME_LEN];
    while (fscanf(file, "%s %s %s %s %s %ld %d %d %lf", number, name, time1, time2, membership, &hours, &spaceID, &count, &amount) == 9) 
	{
        newVehicle = (vehicle*)malloc(sizeof(vehicle));
        if (!newVehicle) 
		{
            printf("Memory allocation failed!\n");
            fclose(file);
            return head;
        }
        arrival = convertToTimeT(time1);
        departure = convertToTimeT(time2);
        strcpy(newVehicle->vehicle_number, number);
        strcpy(newVehicle->owner_name, name);
        strcpy(newVehicle->membership, membership);
        newVehicle->arrival_time = arrival;
        newVehicle->departure_time = departure;
        newVehicle->total_parking_hours = hours;
        newVehicle->parking_space_ID = spaceID;
        newVehicle->parking_count = count;
        newVehicle->total_amount_paid = amount;
        newVehicle->next = head;
        head = newVehicle;
    }

    fclose(file);
    return head;
}

void convertTimeToString(time_t timeValue, char *output) 
{
    struct tm *timeInfo = localtime(&timeValue); // Convert to local time
    
    if (timeInfo == NULL) 
    {
        strcpy(output, "0000-00-00_00:00");  // Handle error case
        return;
    }

    sprintf(output, "%04d-%02d-%02d_%02d:%02d", 
            timeInfo->tm_year + 1900, 
            timeInfo->tm_mon + 1,     
            timeInfo->tm_mday,        
            timeInfo->tm_hour,        
            timeInfo->tm_min);        
}


void saveVehiclesToFile(vehicle *head) 
{
    FILE *file = fopen(VEHICLE_FILE, "w");
    if (!file) 
	{
        printf("Error opening vehicle data file!\n");
        return;
    }
    char time1[TIME_LEN],time2[TIME_LEN];
    
    vehicle *curr = head;
    while (curr) 
    {
        convertTimeToString(curr-> arrival_time , time1);
        convertTimeToString(curr -> departure_time , time2);
        fprintf(file, "%s %s %s %s %s %ld %d %d %lf\n", 
                curr->vehicle_number, curr->owner_name, time1, time2, 
                curr->membership, curr->total_parking_hours, 
                curr->parking_space_ID, curr->parking_count, curr->total_amount_paid);
        curr = curr->next;
    }

    fclose(file);
}

space* makeSpceList(int start , int end) 
{
	int i;
    space *head = NULL, *temp = NULL, *newNode;   
    for (i = start; i <= end; i++) 
    {
        newNode = (space*)malloc(sizeof(space));
        newNode->space_ID = i;
        newNode->status = 0; // Initially all spaces are free
        newNode->occupancy_count = 0;
        newNode->revenue_generated = 0;
        newNode->next = NULL;
        if (head == NULL) 
        { // First node
            head = newNode;
            temp = head;
        } 
        else 
        {
            temp->next = newNode;
            temp = newNode;
        }
    }
    temp -> next = NULL;
    return head;
}

void loadParkingSpacesFromFile(space **golden, space **silver, space **general) 
{
    FILE *file = fopen(PARKING_FILE, "r");
    if (!file) 
	{
        printf("No parking data found. Initializing spaces.\n");
        return;
    }

    space *allSpaces = makeSpceList(1, 50);
    space *curr = allSpaces;
    int id, status, occupancy;
    double revenue;

    while (fscanf(file, "%d %d %d %lf", &id, &status, &occupancy, &revenue) == 4) 
	{
        space *spaceNode = findSpaceByID(id, allSpaces, 1);
        if (spaceNode) 
		{
            spaceNode->status = status;
            spaceNode->occupancy_count = occupancy;
            spaceNode->revenue_generated = revenue;
        }
    }

    // Assign correct membership sections
    *golden = allSpaces;
    *silver = findSpaceByID(11, allSpaces, 1);
    *general = findSpaceByID(21, allSpaces, 1);

    fclose(file);
}

void saveParkingSpacesToFile(space *golden, space *silver, space *general) 
{
    FILE *file = fopen(PARKING_FILE, "w");
    if (!file) 
	{
        printf("Error opening parking space data file!\n");
        return;
    }

    boolean written[SPACE_DB_LEN] = {FALSE};  // Track saved space IDs
	int i;
    space *lists[] = {golden, silver, general};
    for (i = 0; i < 3; i++) 
	{
        space *curr = lists[i];
        while (curr) 
		{
            if (!written[curr->space_ID]) 
			{
                fprintf(file, "%d %d %d %.2lf\n", curr->space_ID, curr->status, curr->occupancy_count, curr->revenue_generated);
                written[curr->space_ID] = TRUE; // Mark as written
            }
            curr = curr->next;
        }
    }

    fclose(file);
}
//functions to copy node
void copyVehicle(vehicle *newNode , char *number , char *name , char *membership , long int hours , int id , int count , double amount )
{
    strcpy(newNode -> vehicle_number , number);
    strcpy(newNode -> owner_name , name);
    strcpy(newNode -> membership , membership);
    newNode -> total_parking_hours = hours;
    newNode -> parking_space_ID = id;
    newNode -> parking_count = count;
    newNode -> total_amount_paid = amount;
    newNode->next = NULL;
}

void copySpace(space *newNode , int id , int status , int occupancy_count , double revenue)
{
    newNode -> space_ID = id;
    newNode -> status = status;
    newNode -> occupancy_count = occupancy_count;
    newNode -> revenue_generated = revenue;
}

//functions to copy link lists
vehicle* copyVehicleList(vehicle* head) 
{
    vehicle *newhead = NULL , *tail = NULL;
    while (head != NULL) 
    {
        vehicle* newNode = (vehicle *)malloc(sizeof(vehicle));
        copyVehicle(newNode , head -> vehicle_number , head -> owner_name , head -> membership , head -> total_parking_hours , head -> parking_space_ID ,head -> parking_count,head -> total_amount_paid );
        if (newhead == NULL)
        {
            newhead = newNode;
            tail = newhead;
        }
        else 
        {
            tail->next = newNode;
            tail = newNode;
        }
        head = head->next;
    }
    return newhead;
}

space* copySpaceList(space* head) 
{
    space *newhead = NULL , *tail = NULL;
    while (head != NULL) 
    {
        space* newNode = (space *)malloc(sizeof(space));
        copySpace(newNode , head -> space_ID , head -> status , head -> occupancy_count , head -> revenue_generated);
        if (newhead == NULL)
        {
            newhead = newNode;
            tail = newhead;
        }
        else 
        {
            tail->next = newNode;
            tail = newNode;
        }
        head = head->next;
    }
    return newhead;
}


//function to divide list into two lists
vehicle *divideVehicleList(vehicle *lptr)
{
    vehicle *temp , *slow , *fast;
    slow = lptr;
    fast = lptr -> next;
    while(fast!=NULL && fast -> next != NULL)
    {
        fast = fast -> next;
        fast = fast -> next;
        slow = slow -> next;
    }
    temp = slow -> next;
    slow -> next = NULL;
    return temp;
}

space *divideSpaceList(space *lptr)
{
    space *temp , *slow , *fast;
    slow = lptr;
    fast = lptr -> next;
    while(fast!=NULL && fast -> next != NULL)
    {
        fast = fast -> next;
        fast = fast -> next;
        slow = slow -> next;
    }
    temp = slow -> next;
    slow -> next = NULL;
    return temp;
}


//function to compare the parameters provided for sorting
boolean compareVehicles(sort_vehicle parameter , vehicle *ptr1 , vehicle *ptr2)
{
    boolean retVal = TRUE;
    if(parameter == BY_PARKINGS)
    {
        if(ptr1 -> parking_count <= ptr2 -> parking_count)
            retVal = FALSE;
    }
    else if(parameter == BY_AMT_PAID)
    {
        if(ptr1 -> total_amount_paid <= ptr2 -> total_amount_paid)
            retVal = FALSE;
    }
    return retVal;
}

boolean compareSpaces(sort_spaces parameter , space *ptr1 , space *ptr2)
{
    boolean retVal = TRUE;
    if(parameter == BY_OCCUPANCY)
    {
        if(ptr1 -> occupancy_count <= ptr2 -> occupancy_count)
            retVal = FALSE;
    }
    else if(parameter == BY_MAX_REVENUE)
    {
        if(ptr1 -> revenue_generated <= ptr2 -> revenue_generated)
            retVal = FALSE;
    }
    return retVal;
}

//function to merge two lists
vehicle *mergeVehicleLists(vehicle *list1 , vehicle *list2 , sort_vehicle parameter)
{
    vehicle *ptr1 , *ptr2 , *result , *tail;
    ptr1 = list1;
    ptr2 = list2;
    if(compareVehicles(parameter ,ptr1 , ptr2))
    {
        result = tail = list1;
        ptr1 = ptr1 -> next;
    }
    else
    {
        result = tail = list2;
        ptr2 = ptr2 -> next;
    }
    while((ptr1 != NULL) && (ptr2 != NULL))
    {
        if(compareVehicles(parameter , ptr1 , ptr2))
        {
            tail -> next = ptr1;
            tail = tail -> next;
            ptr1 = ptr1 -> next;
        }
        else
        {
            tail -> next = ptr2;
            tail = tail -> next;
            ptr2 = ptr2 -> next;
        }
    }
    if(ptr1 != NULL)
    {
        tail -> next = ptr1;
    }
    else if(ptr2 != NULL)
    {
        tail -> next = ptr2;
    }
    return result;
}

space *mergeSpaceLists(space *list1 , space *list2 , sort_spaces parameter)
{
    space *ptr1 , *ptr2 , *result , *tail;
    ptr1 = list1;
    ptr2 = list2;
    if(compareSpaces(parameter ,ptr1 , ptr2))
    {
        result = tail = list1;
        ptr1 = ptr1 -> next;
    }
    else
    {
        result = tail = list2;
        ptr2 = ptr2 -> next;
    }
    while((ptr1 != NULL) && (ptr2 != NULL))
    {
        if(compareSpaces(parameter , ptr1 , ptr2))
        {
            tail -> next = ptr1;
            tail = tail -> next;
            ptr1 = ptr1 -> next;
        }
        else
        {
            tail -> next = ptr2;
            tail = tail -> next;
            ptr2 = ptr2 -> next;
        }
    }
    if(ptr1 != NULL)
    {
        tail -> next = ptr1;
    }
    else if(ptr2 != NULL)
    {
        tail -> next = ptr2;
    }
    return result;
}

//mergesort function

//sorting vehicle details
vehicle* mergeSort_vehicle(vehicle* newhead, sort_vehicle parameter) 
{
    if (!newhead || !newhead->next) return newhead;  
	    
    vehicle *first = newhead, *second = divideVehicleList(newhead);
    first = mergeSort_vehicle(first, parameter);
    second = mergeSort_vehicle(second, parameter);
    
    return mergeVehicleLists(first, second, parameter);
}

space* mergeSort_space(space* newhead, sort_spaces parameter) 
{
    if (!newhead || !newhead->next) return newhead;  
	    
    space *first = newhead, *second = divideSpaceList(newhead);
    first = mergeSort_space(first, parameter);
    second = mergeSort_space(second, parameter);
    
    return mergeSpaceLists(first, second, parameter);
}
void freeSpaceList(space *sptr)
{
    space *curr = sptr;
    space *temp;
    while(temp)
    {
        temp = curr -> next;
        free(curr);
        curr = temp;
    }
}

void freeVehicleList(vehicle *vptr)
{
    vehicle *curr = vptr;
    vehicle *temp;
    while(temp)
    {
        temp = curr -> next;
        free(curr);
        curr = temp;
    }
}

void displayVehicleReport(vehicle *head , sort_vehicle parameter)
{
    vehicle *curr = head;
    if(parameter == BY_AMT_PAID)
    {
        printf("generating report by amount paid:\n");
    }
    else
    {
        printf("generating report by number of parkings done:\n");
    }
    while(curr != NULL)
    {
        printf("vehicle no. :%s  name :%s  parking count :%d  amount paid :%lf ",curr -> vehicle_number , curr -> owner_name , curr -> parking_count ,  curr -> total_amount_paid);
        printf("\n");
        curr = curr -> next;
    }
    printf("\n");
    freeVehicleList(head);
}

void displaySpaceReport(space *head , sort_spaces parameter)
{
    space *curr = head;
    if(parameter == BY_OCCUPANCY)
    {
        printf("generating report by occupancy count:\n");
    }
    else
    {
        printf("generating report by revenue generated:\n");
    }
    while(curr != NULL)
    {
        printf("space ID :%d  occupancy count :%d  revenue :%lf ",curr -> space_ID , curr -> occupancy_count , curr -> revenue_generated);
        printf("\n");
        curr = curr -> next;
    }
    freeSpaceList(head);
}
//generating reports
void vehicleReports(vehicle *head , sort_vehicle parameter)
{
    vehicle *newhead = copyVehicleList(head);
    newhead = mergeSort_vehicle(newhead ,  parameter);
    displayVehicleReport(newhead , parameter);
}

void spaceReports(space **golden, space **silver, space **general, sort_spaces parameter) 
{
    printf("\nSorting all parking spaces by %s...\n", (parameter == BY_OCCUPANCY) ? "occupancy count" : "revenue generated");

    space *mergedList = copySpaceList(*golden);
    mergedList = mergeSort_space(mergedList , parameter);
    printf("Generating report...\n");
    displaySpaceReport(mergedList , parameter);
}

statusCode ExitVehicle(char *number, time_t time, vehicle *vptr , space *golden , space *silver , space *general)
{
	vehicle *Vehicle = searchVehicle(number , vptr);
    statusCode sc = SUCCESS;
    if(Vehicle == NULL)
    {
        printf("Error: Vehicle not found in the records.\n");
        sc=FAILURE;
    }
    else
    {
        space *space;
        if(strcmp(Vehicle -> membership , "GOLDEN") == 0)
        {
            space = findSpaceByID(Vehicle -> parking_space_ID , golden , 1);
        }
        else if(strcmp(Vehicle -> membership , "SILVER") == 0 )
        {
            space = findSpaceByID(Vehicle -> parking_space_ID , silver , 11);
        }
        else
        {
            space = findSpaceByID(Vehicle -> parking_space_ID , general , 21);
        }
        double charges = 0.0;
        int id = Vehicle -> parking_space_ID; // id at which the vehicle is parked
	    Vehicle -> departure_time = time;
	    int hours_spent = difftime(time , Vehicle -> arrival_time)/3600;
        printf("Hours spent: %d hours\n", hours_spent);
        Vehicle -> total_parking_hours = Vehicle -> total_parking_hours + hours_spent;
        Vehicle -> parking_space_ID = 0; 
        //updating membership
        if (Vehicle -> total_parking_hours > 200)
        {
            if (strcmp(Vehicle -> membership, "GOLDEN") != 0) // Upgrade to GOLDEN if not already GOLDEN
            {
                strcpy(Vehicle -> membership, "GOLDEN");
                printf("Membership upgraded to GOLDEN\n");
            }
        }
        else if (Vehicle -> total_parking_hours > 100)
        {
            if (strcmp(Vehicle -> membership, "SILVER") != 0) // Upgrade to SILVER if not already SILVER 
            {
                strcpy(Vehicle -> membership, "SILVER");
                printf("Membership upgraded to SILVER\n");
            }
        }
        // Calculate parking charges
        if (hours_spent <= 3)
        {
            charges = 100.0;
        }
        else
        {
            charges = (hours_spent - 3) * 50.0 + 100.0;
        }
        if (strcmp(Vehicle->membership, "GENERAL") != 0)
        {
            charges = charges*0.9;	
	    }
        printf("Parking charges: %.2lf Rs\n", charges);
    
	    Vehicle -> total_amount_paid = Vehicle -> total_amount_paid + charges;
        space -> status = 0;// now the space is unoccupied
        space -> revenue_generated = space -> revenue_generated + charges; //calculate current cost and add;
    
	    printf("Vehicle exited successfully!\nParking space ID %d is now free.\n", space -> space_ID);
	
        // Save updated data to files
        saveVehiclesToFile(vptr);
        saveParkingSpacesToFile(golden, silver, general);	
    }
    return sc;
}


int main()
{
	space *golden = NULL, *silver = NULL, *general = NULL;
    vehicle *vptr = NULL;
    
    loadParkingSpacesFromFile(&golden, &silver, &general);
    vptr = loadVehiclesFromFile();
    
    printf("\n Welcome to the Smart Car Parking Lot System!\n");
    char vehicle_num[NUM_LEN];
    char name[NAME_LEN];
    int choice,subchoice;
    statusCode sc;
    do
    {
        
        printf("1. Vehicle Entry\n");
        printf("2. Vehicle Exit\n");
        printf("3. Sorting and analysis\n");
        printf("4. Exit system\n");
        printf("Please Enter your choice: \n");
        scanf("%d", &choice);
        switch(choice)
        {
            case 1:  
            	// vehicle entry
            	printf("Enter your vehicle details\n");
            	printf("Enter the vehicle number:\n");
            	scanf(" %[^\n]",vehicle_num);
            	printf("Enter the name of owner:\n");
            	scanf(" %[^\n]",name);
            	
            	sc = updateInsertVehicle(vehicle_num , name , time(NULL) , golden , silver , general , &vptr);
            	if(sc)
                {
    			// Save updated data to files
    			    saveVehiclesToFile(vptr);
    			    saveParkingSpacesToFile(golden, silver, general);
                }
            	break;
                   
            case 2:
            	// Vehicle exit  
            	printf("Enter your vehicle number:\n");
            	scanf(" %[^\n]", vehicle_num); // Prompt for the vehicle number during exit        	
            	sc = ExitVehicle(vehicle_num, time(NULL) , vptr , golden , silver , general);
                break;
                     
            case 3:
            //sorting and analysis
            do
            {
                printf("1: Sort the list of vehicles based on the number of parkings done\n");
                printf("2: Sort the list of vehicles based on the total parking amount paid\n");
                printf("3: Sort the parking spaces based on how often they are occupied\n");
                printf("4: Sort the parking spaces based on the revenue generated \n");
                printf("5:Exit sorting and analysis \n");
                       
                printf("Enter your choice:\n");
                scanf("%d",&subchoice);
                switch(subchoice)
                {
                    case 1:
                        vehicleReports(vptr , BY_PARKINGS);
                        break;
                        
                    case 2:
                        vehicleReports(vptr , BY_AMT_PAID);   
                        break;
                        
                    case 3:
                        spaceReports(&golden , &silver , &general , BY_OCCUPANCY );
                        break;
                        
                    case 4:
                        spaceReports(&golden , &silver , &general , BY_MAX_REVENUE);
                        break;
                        
                    case 5:
                        printf("Exiting analysis and sorting\n");
                        break;
                        
                    default:
                        printf("Invalid choice! please enter a valid choice\n");
                        break;
                }
            }while(subchoice!=5);
            break;
                     
            case 4:
                printf("Exiting the system.Thank you!\n");
                break;
                         
            default:
                printf("Invalid choice. Please enter a valid choice \n");
                break;                          
        }
    }while(choice!=4 || sc == FAILURE);
    
return 0;

}

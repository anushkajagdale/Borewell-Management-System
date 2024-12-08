#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CROPS 50
#define MAX_HISTORY 100
#define HASH_SIZE 100

typedef struct CropInfo {
    int id;
    char cropType[20];
    char soilType[20];
    int waterRequired; // in liters
    time_t startTime;
    time_t endTime;
    struct CropInfo* next; // Linked list pointer
} CropInfo;

typedef struct BorewellConnection {
    int borewellId;
    int connectedTo; // ID of the connected borewell or farm
    float distance; // Distance between borewells
    float speed; // Speed of water transfer in liters/hour
    struct BorewellConnection* next; // Linked list pointer
} BorewellConnection;

typedef struct QueueNode {
    int id;
    time_t startTime;
    int waterAmount;
    float speed;
    struct QueueNode* next;
} QueueNode;

typedef struct Queue {
    QueueNode* front;
    QueueNode* rear;
} Queue;

typedef struct StackNode {
    char action[100];
    time_t timestamp;
    struct StackNode* next;
} StackNode;

typedef struct Stack {
    StackNode* top;
} Stack;

typedef struct HashEntry {
    int key; // Borewell ID
    BorewellConnection* connection; // Pointer to the connection
} HashEntry;

typedef struct HashTable {
    HashEntry* table[HASH_SIZE]; // Array of hash entries
} HashTable;

// Binary Search Tree for Crop Information
typedef struct TreeNode {
    CropInfo* crop; // Pointer to crop info
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

typedef struct HistoryEntry {
    char action[100];
    time_t timestamp;
} HistoryEntry;

// Global variables
BorewellConnection* connectionListHead = NULL;
CropInfo* cropListHead = NULL;
Queue motorQueue = {NULL, NULL};
Stack actionStack = {NULL};
HashTable connectionHashTable = {0};
HistoryEntry history[MAX_HISTORY];
int historyCount = 0;
TreeNode* cropTreeRoot = NULL;

// Function prototypes
void addBorewellConnection(int borewellId, int connectedTo, float distance, float speed, time_t startTime);
void scheduleMotor(int id, time_t startTime, int waterAmount, float speed);
void addCropInfo(int id, const char* cropType, const char* soilType, int waterRequired, time_t startTime, float speed);
void displayMenu();
void displayConnections();
void displayCropsInOrder(TreeNode* root);
void displayHistory();
void logHistory(const char* action);
void enqueueMotor(int id, time_t startTime, int waterAmount, float speed);
QueueNode* dequeueMotor();
void pushAction(const char* action);
char* popAction();
int hash(int key);
void insertHash(int key, BorewellConnection* connection);
BorewellConnection* searchHash(int key);
TreeNode* insertCrop(TreeNode* root, CropInfo* crop);

int main() {
    int choice;
    
    while (1) {
        displayMenu();
        printf("👉 Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1: {
                int borewellId, connectedTo;
                float distance, speed;
                int hour, minute;

                printf("🔗 Enter Borewell ID to connect: ");
                scanf("%d", &borewellId);
                printf("🌾 Enter ID of the Borewell/Farm to connect to: ");
                scanf("%d", &connectedTo);
                printf("📏 Enter Distance between Borewells (in meters): ");
                scanf("%f", &distance);
                printf("⚡ Enter Speed of Water Transfer (liters/hour): ");
                scanf("%f", &speed);
                printf("⏰ Enter Start Time (hours): ");
                scanf("%d", &hour);
                printf("⏰ Enter Start Time (minutes): ");
                scanf("%d", &minute);

                time_t startTime = time(NULL);
                struct tm *tm_info = localtime(&startTime);
                tm_info->tm_hour = hour;
                tm_info->tm_min = minute;
                tm_info->tm_sec = 0;

                startTime = mktime(tm_info);
                addBorewellConnection(borewellId, connectedTo, distance, speed, startTime);
                break;
            }
            case 2: {
                int id, waterAmount;
                float speed;
                int hour, minute;

                printf("🕒 Enter Borewell ID for scheduling: ");
                scanf("%d", &id);
                printf("💧 Enter Water Amount to transfer (liters): ");
                scanf("%d", &waterAmount);
                printf("⚡ Enter Speed of Water Transfer (liters/hour): ");
                scanf("%f", &speed);
                printf("⏰ Enter Start Time (hours): ");
                scanf("%d", &hour);
                printf("⏰ Enter Start Time (minutes): ");
                scanf("%d", &minute);

                time_t startTime = time(NULL);
                struct tm *tm_info = localtime(&startTime);
                tm_info->tm_hour = hour;
                tm_info->tm_min = minute;
                tm_info->tm_sec = 0;

                startTime = mktime(tm_info);
                scheduleMotor(id, startTime, waterAmount, speed);
                break;
            }
            case 3: {
                int id;
                char cropType[20], soilType[20];
                int waterRequired;
                
                printf("🌱 Enter Crop ID: ");
                scanf("%d", &id);
                printf("🌾 Enter Crop Type: ");
                scanf("%s", cropType);
                printf("🌍 Enter Soil Type: ");
                scanf("%s", soilType);
                printf("💧 Enter Water Required (liters): ");
                scanf("%d", &waterRequired);

                float speed; 
                
               printf("⚡ Enter Speed of Water Transfer (liters/hour): ");
               scanf("%f", &speed);

               int hour, minute; 
               printf("⏰ Enter Start Time (hours): "); 
               scanf("%d", &hour); 
               printf("⏰ Enter Start Time (minutes): "); 
               scanf("%d", &minute); 

               time_t startTime = time(NULL); 
               struct tm *tm_info=localtime(&startTime); 
               tm_info->tm_hour = hour; 
               tm_info->tm_min = minute; 
               tm_info->tm_sec = 0; 

               startTime = mktime(tm_info); 
               addCropInfo(id, cropType, soilType, waterRequired, startTime, speed); 
               break; 
           }
           case 4:
               displayConnections();
               break;

           case 5:
               displayCropsInOrder(cropTreeRoot);
               break;

           case 6: {
               int searchId;
               printf("🔍 Enter Borewell ID to search: ");
               scanf("%d", &searchId);

               BorewellConnection *foundConnection = searchHash(searchId);
               if (foundConnection) {
                   printf("🔗 Found Connection: Borewell %d connected to %d | Distance: %.2f m | Speed: %.2f liters/hour\n",
                          foundConnection->borewellId,
                          foundConnection->connectedTo,
                          foundConnection->distance,
                          foundConnection->speed);
               } else {
                   printf("❌ No connection found for Borewell ID %d.\n", searchId);
               }
               break;
           }
           case 7: {
               int searchId;
               printf("🔍 Enter Borewell ID to get detailed information: ");
               scanf("%d", &searchId);

               BorewellConnection *connectionDetails = searchHash(searchId);
               if (connectionDetails) {
                   printf("\n--- 📋 Borewell Information ---\n");
                   printf("Borewell ID: %d\n", connectionDetails->borewellId);
                   printf("Connected To: %d\n", connectionDetails->connectedTo);
                   printf("Distance: %.2f meters\n", connectionDetails->distance);
                   printf("Speed of Water Transfer: %.2f liters/hour\n", connectionDetails->speed); 
                   // You can add more information here if needed.
               } else {
                   printf("❌ No details found for Borewell ID %d.\n", searchId);
               }
               break;
           }
           case 8:
               displayHistory();
               break;

           case 0:
               printf("🚪 Exiting the system. Goodbye! 👋\n");
               exit(0);

           default:
               printf("❌ Invalid choice! Please try again.\n");
       }
   }

   return 0; 
}

void displayMenu() {
   printf("\n--- 🌟 Borewell Management System Menu 🌟 ---\n");
   printf("1. Add Borewell Connection\n");
   printf("2. Schedule Motor\n");
   printf("3. Add Crop Information\n");
   printf("4. Display Connections\n");
   printf("5. Display Crops in Sorted Order\n");
   printf("6. Search for Borewell Connection\n");
   printf("7. Get Detailed Borewell Information\n");
   printf("8. Display History\n");
   printf("0. Exit\n");
}

void addBorewellConnection(int borewellId, int connectedTo, float distance, float speed, time_t startTime) {
   BorewellConnection* newConnection = (BorewellConnection*)malloc(sizeof(BorewellConnection));
   
   newConnection->borewellId = borewellId; 
   newConnection->connectedTo = connectedTo; 
   newConnection->distance = distance; 
   newConnection->speed = speed; 

   // Insert into linked list for connections
   newConnection->next = connectionListHead; 
   connectionListHead = newConnection; 

   // Insert into hash table
   insertHash(borewellId, newConnection); 

   // Calculate end time (distance / speed gives hours)
   int durationSeconds = (distance / speed) * 3600; 
   time_t endTime = startTime + durationSeconds; 

   // Display suggested end time
   char buffer[26]; 
   struct tm* tm_info = localtime(&endTime); 
   strftime(buffer,sizeof(buffer),"%H:%M:%S",tm_info); 

   printf("⏰ Suggested End Time: %s\n", buffer); 
   logHistory("Added Borewell Connection"); 
   
   printf("✅ Borewell connection added successfully!\n"); 
}

void scheduleMotor(int id, time_t startTime, int waterAmount, float speed) {
   enqueueMotor(id,startTime ,waterAmount ,speed ); 

   // Calculate end time based on water amount and speed
   int durationSeconds =(waterAmount / speed) * 3600; 
   
   time_t endTime=startTime + durationSeconds; 

   // Display suggested end time
   char buffer[26]; 
   
   struct tm *tm_info=localtime(&endTime); 
   
   strftime(buffer,sizeof(buffer),"%H:%M:%S",tm_info); 

   printf ("⏰ Suggested End Time: %s\n",buffer); 

   logHistory ("Scheduled Motor Operation"); 

   printf ("✅ Motor scheduled successfully!\n"); 
}

void addCropInfo(int id,const char* cropType,const char* soilType,int waterRequired,time_t startTime,float speed) { 
     CropInfo* newCrop=(CropInfo*)malloc(sizeof(CropInfo)); 
    
     newCrop->id=id; 
    
     strcpy(newCrop->cropType,cropType); 
    
     strcpy(newCrop->soilType ,soilType ); 
    
     newCrop->waterRequired=waterRequired ; 
    
     newCrop->startTime=startTime ; 
    
     // Insert into linked list for crops 
    
     newCrop->next=cropListHead ; 
    
     cropListHead=newCrop ; 
    
     cropTreeRoot=insertCrop(cropTreeRoot,newCrop ); 
    
     // Calculate end time based on water required and speed 
    
     int durationSeconds =(waterRequired / speed) * 3600; 
    
     time_t endTime=startTime + durationSeconds; 
    
     // Display suggested end time 
    
     char buffer[26]; 
    
     struct tm *tm_info=localtime(&endTime); 
    
     strftime(buffer,sizeof(buffer),"%H:%M:%S",tm_info); 
    
     printf ("⏰ Suggested End Time: %s\n",buffer); 
    
     logHistory ("Added Crop Information"); 
    
     printf ("✅ Crop information added successfully!\n"); 
}

void displayConnections() { 
     printf("\n--- 📡 Borewell Connections ---\n"); 
    
     BorewellConnection *current=connectionListHead; 
    
     while(current) { 
        
         printf ("🔗 Borewell %d connected to %d | Distance: %.2f m | Speed: %.2f liters/hour\n",
                 current->borewellId,
                 current->connectedTo,
                 current->distance,
                 current->speed); 
        
         current=current->next;  
     }  
}

void displayCropsInOrder(TreeNode *root) { 
     if (!root) return;  
    
     displayCropsInOrder(root->left);  
    
     printf ("🌾 Crop ID: %d | Type: %s | Soil: %s | Water Required: %d liters\n",
             root->crop->id,
             root->crop->cropType,
             root->crop->soilType,
             root->crop->waterRequired);  
    
     displayCropsInOrder(root->right);  
}

void displayHistory() {  
      printf ("\n--- 📜 Action History ---\n");  
    
      for (int i=0;i<historyCount;i++) { 
        
          char buffer[26]; 
        
          struct tm *tm_info=localtime(&history[i].timestamp); 
        
          strftime(buffer,sizeof(buffer),"%Y-%m-%d %H:%M:%S",tm_info); 
        
          printf ("[%s] - %s\n",buffer ,history[i].action);  
      }  
}

TreeNode *insertCrop(TreeNode *root,CropInfo *crop) {  
      if (!root) { 
        
          TreeNode *newNode=(TreeNode*)malloc(sizeof(TreeNode)); 
        
          newNode->crop=crop; 
        
          newNode->left=newNode->right=NULL; 
        
          return newNode;  
      }  
    
      if(crop ->id<root ->crop ->id){ 
        
          root ->left=insertCrop(root ->left,crop);  
      }else{ 
        
          root ->right=insertCrop(root ->right,crop);  
      }  
    
      return root;  
}

void logHistory(const char *action) {  
      if(historyCount<MAX_HISTORY) { 
        
          strcpy(history[historyCount].action ,action ); 
        
          history[historyCount].timestamp=time(NULL ); 
        
          historyCount++;  
      }else { 
        
          printf ("⚠ History log is full!\n");  
      }  
}

void enqueueMotor(int id,time_t startTime,int waterAmount,float speed) {  
      QueueNode *newNode=(QueueNode*)malloc(sizeof(QueueNode));  
    
      newNode ->id=id ; 
    
      newNode ->startTime=startTime ; 
    
      newNode ->waterAmount=waterAmount ; 
    
      newNode ->speed=speed ;  
    
      newNode ->next=NULL ;  
    
      if(motorQueue.rear){ 
        
          motorQueue.rear ->next=newNode ;  
      }  
    
      motorQueue.rear=newNode ;  
    
      if(!motorQueue.front){ 
        
          motorQueue.front=newNode ;  
      }  
}

QueueNode *dequeueMotor() {  
      if(!motorQueue.front)return NULL ;  
    
      QueueNode *temp=motorQueue.front ;  
    
      motorQueue.front=motorQueue.front ->next ;  
    
      if(!motorQueue.front){ 
        
          motorQueue.rear=NULL ;  
      }  
    
      return temp ;  
}

void pushAction(const char *action) {  
       StackNode *newNode=(StackNode*)malloc(sizeof(StackNode));  
    
       strcpy(newNode ->action ,action );  
    
       newNode ->timestamp=time(NULL );  
    
       newNode ->next=actionStack.top ;  
    
       actionStack.top=newNode ;  
}

char *popAction() {  
       if(!actionStack.top)return NULL ;  
    
       StackNode *temp=actionStack.top ;  
    
       actionStack.top=actionStack.top ->next ;  
    
       char *action=strdup(temp ->action );  
    
       free(temp );  
    
       return action ;  
}

int hash(int key) {  
       return key % HASH_SIZE;  
}

void insertHash(int key,BorewellConnection *connection) {  
       int index=hash(key);  

       // Create a new hash entry
       HashEntry *newEntry=(HashEntry*)malloc(sizeof(HashEntry));  

       newEntry ->key=key;  

       newEntry ->connection=connection;  

       // Handle collision with linked list
       if(connectionHashTable.table[index]){  

           // Collision detected - add to the front of the linked list
           HashEntry *current=connectionHashTable.table[index];  

           while(current && current->key != key){ 

              current=current+1;}  

           // If found update the connection pointer else insert new entry
           if(current && current ->key==key){ 

              current ->connection=connection;}else{ 

              connectionHashTable.table[index]=newEntry;}  

       }else{  

           // No collision - insert directly
           connectionHashTable.table[index]=newEntry;}  

}

BorewellConnection *searchHash(int key) {  
       int index=hash(key);  

       HashEntry *entry=connectionHashTable.table[index];  

       // Traverse linked list if collision occurred
       while(entry){  

           if(entry ->key==key){ 

              return entry ->connection;} 
  
           entry=(HashEntry *)entry +1;}  

       return NULL;// Not found 
}
#include<stdio.h>
#include<time.h>
#include<string.h>
// to check whether the file exist or not
void file_exit(){
 FILE *todo=fopen("todo.csv","r");
 if (todo!=NULL){
     fclose(todo);
 }
 else {
   FILE *Ntodo=fopen("todo.csv","w");
   fprintf(Ntodo,"Day-Month-DATE,TIME-Year,TO_DO\n");
   fclose(Ntodo);
 }
}
// to display options
void display(){
  printf("----TO_DO----\n");
  printf("1.Check TODO\n");
  printf("2.ADD task\n");
 // printf("3.Delete task\n");
  printf("CHOSE AN OPTION(1-2):");
}

 void Check()
{
 FILE *file=fopen("todo.csv","r");
 char str[1024];
while(fgets(str, sizeof(str),file) != NULL) 
{ puts(str); }
fclose(file);
 } 

void add_task() { 
    // Get the current time  
    time_t t = time(NULL);
    struct tm* ti = localtime(&t);
    char* ts = asctime(ti);
    ts[24] = '\0'; // Remove newline from asctime


   char str[1024];
    printf("What to do: ");
    fgets(str, sizeof(str), stdin);

    // Remove the trailing newline character brought in by fgets
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }

    FILE *fi = fopen("todo.csv", "a");
    if (fi != NULL) {
// Added a '\n' at the end of the format string to cleanly separate CSV rows
        fprintf(fi, "%s,%s\n", ts, str); 
        fclose(fi); // Always close your files!
        printf("Task added successfully!\n");
    } 
    else {
        printf("Error: Could not open or create FILE\n");
    }
}
void delete()
{
}

int main()
{ int ch;
  char res;
 do { 
 file_exit();
 display();
 scanf("%d",&ch);

 // Remove the newline left by scanf before add_task() uses fgets().
 int c;
 while ((c = getchar()) != '\n' && c != EOF);

 if (ch>3 || ch<1){
   printf("OUT OF OPTION\n");
 }
// selecting options
 switch(ch){
     case 1:
       Check();
       break;
     case 2:
        add_task();
        break;
     default:
       printf("Out of Range!\n");
       break;
 }

printf("WANT TO CONTINUE?(y/n): ");
scanf(" %c", &res);

// Clear any remaining input before the next menu iteration.
while ((c = getchar()) != '\n' && c != EOF); 

 }while(res=='y' || res=='Y'); // later i will ask if user wanna continue or not
 return 0;
} 

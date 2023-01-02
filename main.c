#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gtk/gtk.h>


#define NO_MEMORY 0
#define PASSWD_LOADING_ERROR 0
#define GDK_KEY_EnterLeft 65293
#define GDK_KEY_EnterRight 65451

GtkBuilder *builder;

GtkWidget *Window;
GtkWidget *EventBoxWindow;
GtkWidget *Stack;
GtkWidget* DialogConfirmDel;

//Buttons
GtkWidget *ButtonPrv;
GtkWidget *ButtonNxt;
GtkWidget *ButtonAdd;
GtkWidget *ButtonDlt;
GtkWidget *ButtonClr;
GtkWidget *ButtonRfr;

//Labels
GtkWidget *LabelUsername;
GtkWidget *LabelPassword;
GtkWidget *LabelUserID;
GtkWidget *LabelGroups;
GtkWidget *LabelUserInfo;
GtkWidget *LabelDirectory;
GtkWidget *LabelDeleteOutcome;
GtkWidget *LabelAddOutcome;
GtkWidget *LabelLastPasswordChange;
GtkWidget *LabelPasswordExpires;
GtkWidget *LabelPasswordInactive;
GtkWidget *LabelAccountExpires;
GtkWidget *LabelConfirm;

//Entries
GtkWidget *EntryUsername;
GtkWidget *EntryUserID;
GtkWidget *EntryGroups;
GtkWidget *EntryUserInfo;
GtkWidget *EntryDirectory;
GtkWidget *EntryPassword;


typedef struct node {
  char* username; //1
  bool password; //2
  int userID;     //3
  char* userInfo;    //5
  char* directory;    //6

  char* groups;

  char* LastPasswordChange;
  char* PasswordExpires;
  char* PasswordInactive;
  char* AccountExpires;

  struct node *previous;
  struct node *next;
}user;

user* current_user;
user* root_user;

/*
char* combine_groups(char array[32][255]) {
    char* combined;
    
    if((combined = malloc(32 * 255 * sizeof(char))) == NULL ) {
      fprintf(stderr, "No memory available\n");
      exit(NO_MEMORY);
    }

    int i = 0;

    while(array[i+1][0] != '\0') {
      strcat(combined, array[i++]);
      strcat(combined, ", ");
    }
    strcat(combined, array[i++]);

    return combined;
}
*/

/*
char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos) {
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}
*/

//tokenizer
void next_field(char* buffer, char** line_ptr, char sep){
  int i = 0;
  while(**line_ptr != sep && **line_ptr != '\0'){
    buffer[i++] = **line_ptr;
    (*line_ptr)++;
  }
  buffer[i] = '\0';
  (*line_ptr)++;
  //printf("buff: %s \n", buffer);
}


//app data structer methods
void assign_user_group(user* u){
  char command[255];
  sprintf(command, "groups %s", u->username);

  FILE *output;
  
  int i = 0;
  int len = 0;
  char line[510];
  output = popen(command, "r");

  u->groups = (char*) malloc(510 * sizeof(char));

  if(output != NULL && fgets(line , 510, output) != NULL){
    line[strcspn(line, "\n")] = 0;
    strcpy(u->groups, line + strlen(u->username) + 3);
    //line[strcspn(line, "\n")] = 0;
    //strcpy(u->groups, replace_char(line + strlen(u->username) + 3, ' ', ','));
  }
  else{
    u->groups = "";
  }
  
}

void set_user_password_info(user* u){
  char command[255];
  sprintf(command, "chage -l %s", u->username);

  FILE *output;
  output = popen(command, "r");
  
  char line[1275];
  char *line_ptr = line;
  char buffer[255];

  //Last password change
  fgets(line, 1275, output);
  strcpy(buffer, strchr(line, ':') + 2);
  buffer[strcspn(buffer, "\n")] = 0;
  u->LastPasswordChange = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  strcpy(u->LastPasswordChange, buffer);
  //printf("%s",u->LastPasswordChange);

  //last password change
  fgets(line, 1275, output);
  strcpy(buffer, strchr(line, ':') + 2);
  buffer[strcspn(buffer, "\n")] = 0;
  u->PasswordExpires = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  strcpy(u->PasswordExpires, buffer);

  //last password change
  fgets(line, 1275, output);
  strcpy(buffer, strchr(line, ':') + 2);
  buffer[strcspn(buffer, "\n")] = 0;
  u->PasswordInactive = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  strcpy(u->PasswordInactive, buffer);

  //last password change
  fgets(line, 1275, output);
  strcpy(buffer, strchr(line, ':') + 2);
  buffer[strcspn(buffer, "\n")] = 0;
  u->AccountExpires = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  strcpy(u->AccountExpires, buffer);
}

user* fill_user_data(char* line_ptr){
  char buffer[255];

  //create new node
  user* new_user;
  if((new_user = (user*)malloc(sizeof(user) )) == NULL ) {
    fprintf(stderr, "No memory available\n");
    exit(NO_MEMORY);
  }
  char sep = ':';

  //username
  next_field(buffer, &line_ptr, sep);
  new_user->username = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  //new_user->username = (char*) malloc(strlen(buffer) * sizeof(char));
  strcpy(new_user->username, buffer);


  //password
  next_field(buffer, &line_ptr, sep);
  if(*buffer == 'x' && *(buffer+1) == '\0') { new_user->password = true; }
  else                                      { new_user->password = false; }

  //userID
  next_field(buffer, &line_ptr, sep);
  new_user->userID = atoi(buffer);

  //skip GroupID
  next_field(buffer, &line_ptr, sep);

  //userInfo
  next_field(buffer, &line_ptr, sep);
  new_user->userInfo = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  //new_user->userInfo = (char*) malloc(strlen(buffer) * sizeof(char));
  strcpy(new_user->userInfo, buffer);

  //directory
  next_field(buffer, &line_ptr, sep);
  new_user->directory = (char*) malloc((strlen(buffer) + 1) * sizeof(char));
  //new_user->directory = (char*) malloc(strlen(buffer) * sizeof(char));
  strcpy(new_user->directory, buffer);

  //add groups
  assign_user_group(new_user);

  //password info
  set_user_password_info(new_user);

  return new_user;

}

void get_users_from_passwd(user** users){
  
  FILE* passwd = fopen("/etc/passwd", "r");

  char line[1275];
  char *line_ptr = line;
  char buffer[255];

  user* new_user;

  //create and fill first node
  if(passwd != NULL && fgets(line, 1275, passwd) != NULL){
    new_user = fill_user_data(line_ptr);
  }
  else{ exit(PASSWD_LOADING_ERROR); }

  *users = new_user;
  user* iterator = *users;

  while(fgets(line, 1275, passwd) != 0){
    
    line_ptr = line;

    new_user = fill_user_data(line_ptr);
    new_user->previous = iterator; 

    iterator->next = new_user;
    iterator = iterator->next;
  }

  //link last node with first
  iterator->next = *users;
  (*users)->previous = iterator;

}

void assign_all_users_groups(){
  user* start = root_user;
  user* cur_user = current_user;
  do{
    assign_user_group(cur_user);
    cur_user = cur_user->next;
  }while(cur_user != start);
}

void print_users(user* u){
  user* start = u;
  user* cur_user = u;
  
  do{
    printf("%s:%d:%d:%s:%s:%s:%s:%s:%s:%s\n",
      cur_user->username,
      cur_user->password,
      cur_user->userID,
      cur_user->userInfo,
      cur_user->directory,
      cur_user->groups,
      cur_user->LastPasswordChange,
      cur_user->PasswordExpires,
      cur_user->PasswordInactive,
      cur_user->AccountExpires
    );
    cur_user = cur_user->next;
  }while(cur_user != start);

}

void add_user_app(){
  char line[1275];
  FILE *passwd = popen("tail -n 1 /etc/passwd", "r");

  if (passwd == NULL) { exit(PASSWD_LOADING_ERROR); }

  user* new_user;

  // Read the last line from the passwd
  if(fgets(line, 1275, passwd)) { new_user = fill_user_data(line); }
  else                          { printf("Failed to load passwd file"); }

  user* last_user = root_user->previous;

  //link last user and new_user
  last_user->next = new_user;
  new_user->previous = last_user;

  //link new_user and root
  new_user->next = root_user;
  root_user->previous = new_user;
}

void delete_user_app(){
  current_user->previous->next = current_user->next;
  current_user->next->previous = current_user->previous;
}


//handling commands
int execute_command_get_error(char* command, char error_msg[255]){
  //executes command
  //reutrn: 1 if command succesed
  //return: 0 if command failed 

  FILE *output;
  output = popen(command, "r");

  if(fgets(error_msg , 254, output) != NULL){
    //command returns error
    error_msg[strcspn(error_msg, "\n")] = 0;
    return 0;
  }
  else{
    return 1;
  }
}

int add_groups_system(const char* groups_const){
  char error_msg[255];
  char label_content[300];

  char groups_command[510];
  char groups[470];
  char* line_ptr = groups;
  char buffer[255];
  char sep = ',';
  strcpy(groups, groups_const);

  do{
    next_field(buffer, &line_ptr, sep);
    
    //prepar command
    sprintf(groups_command, "groupadd %s 2>&1", buffer);
    //printf("command: %s\n", groups_command);
    
    
    //execute command
    if(execute_command_get_error(groups_command, error_msg) == 0){
      //failed to add current group
      if(strcmp(error_msg + strlen(error_msg) - 14, "already exists") != 0){
        //the error is not already exists error
        //printf("e: %s\n", error_msg);
        sprintf(label_content, "<span color='red'>%s</span>", error_msg + 9);
        gtk_label_set_markup(GTK_LABEL(LabelAddOutcome), label_content);
        return 0;
      }
    }
  }while(*line_ptr != '\0' && *(line_ptr-1) != '\0');

  return 1;
}

int add_user_system(){

  const char* username  = gtk_entry_get_text(GTK_ENTRY(EntryUsername  ));
  const char* userID    = gtk_entry_get_text(GTK_ENTRY(EntryUserID    ));
  const char* groups    = gtk_entry_get_text(GTK_ENTRY(EntryGroups    ));
  const char* userInfo  = gtk_entry_get_text(GTK_ENTRY(EntryUserInfo  ));
  const char* directory = gtk_entry_get_text(GTK_ENTRY(EntryDirectory ));
  bool password = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(EntryPassword));

  
  if(*groups){
    if(add_groups_system(groups) == 0){
      return 0;
    }
  }

  
  //char* spaceless_groups;
  //strcpy(spaceless_groups, groups);
  //strcpy(spaceless_groups, replace_char(spaceless_groups, ' ', ''));
  //printf("g: %s ", spaceless_groups);
  

  int offset = 0;
  char command[510] = {0};
  offset += snprintf(command + offset, 510 - offset, "useradd '%s' ", username);
  if(*userID)   {offset += snprintf(command + offset, 510 - offset, " -u '%s' ", userID   );}
  if(*groups)   {offset += snprintf(command + offset, 510 - offset, " -G '%s' ", groups );}
  if(*userInfo) {offset += snprintf(command + offset, 510 - offset, " -c '%s' ", userInfo );}
  if(*directory){offset += snprintf(command + offset, 510 - offset, " -d '%s' ", directory);}
  offset += snprintf(command + offset, 510 - offset, " 2>&1 ");

  char error_msg[255];
  if(execute_command_get_error(command, error_msg) == 0){
    //failed to add a user
    char label_content[300];
    sprintf(label_content, "<span color='red'>%s</span>", error_msg + 9);
    gtk_label_set_markup(GTK_LABEL(LabelAddOutcome), label_content);
    return 0;
  }
  else{
    //User added succefully
    gtk_label_set_markup(GTK_LABEL(LabelAddOutcome), "<span color='green'>User added succefully</span>");

    //delete default password
    sprintf(command, "passwd %s -d > /dev/null 2>&1", username);
    system(command);

    if(password == 1){
      //require new password on next login
      sprintf(command, "passwd %s -e > /dev/null 2>&1", username);
      system(command);
    }
    return 1;
  }

}

int delete_user_system(){

  char command[255];
  sprintf(command, "userdel %s 2>&1", current_user->username);

  char error_msg[255];
  if(execute_command_get_error(command, error_msg) == 0){
    char label_content[300];
    sprintf(label_content, "<span color='red'>%s</span>", error_msg + 9);
    gtk_label_set_markup(GTK_LABEL(LabelDeleteOutcome), label_content);
    return 0;
  }
  else{
    gtk_label_set_markup(GTK_LABEL(LabelDeleteOutcome), "<span color='green'>User succefully deleted</span>");
    return 1;
  }
}


//app actions
void display_current_user(){
  gtk_label_set_text(GTK_LABEL(LabelUsername           ),                       current_user->username           );
  gtk_label_set_text(GTK_LABEL(LabelUserID             ), g_strdup_printf("%d", current_user->userID            ));
  gtk_label_set_text(GTK_LABEL(LabelGroups             ),                       current_user->groups             );
  gtk_label_set_text(GTK_LABEL(LabelUserInfo           ),                       current_user->userInfo           );
  gtk_label_set_text(GTK_LABEL(LabelDirectory          ),                       current_user->directory          );
  gtk_label_set_text(GTK_LABEL(LabelLastPasswordChange ),                       current_user->LastPasswordChange );
  gtk_label_set_text(GTK_LABEL(LabelPasswordExpires    ),                       current_user->PasswordExpires    );
  gtk_label_set_text(GTK_LABEL(LabelPasswordInactive   ),                       current_user->PasswordInactive   );
  gtk_label_set_text(GTK_LABEL(LabelAccountExpires     ),                       current_user->AccountExpires     );

  if(current_user->password){ gtk_label_set_text(GTK_LABEL(LabelPassword ), "yes" );}
  else{                       gtk_label_set_text(GTK_LABEL(LabelPassword ), "no"  );}
}

void clear_output_labels(){
  gtk_label_set_text (GTK_LABEL(LabelAddOutcome)   , NULL);
  gtk_label_set_text (GTK_LABEL(LabelDeleteOutcome), NULL);
}

void delete_user(){
  //create dialog window
  builder = gtk_builder_new_from_file("interface.glade");
  DialogConfirmDel = GTK_WIDGET(gtk_builder_get_object(builder, "DialogConfirmDel"));
  
  //set label text
  char label_content[255];
  LabelConfirm = GTK_WIDGET(gtk_builder_get_object(builder, "LabelConfirm"));
  sprintf(label_content, "Delete user '%s' ?", current_user->username);
  gtk_label_set_text(GTK_LABEL(LabelConfirm), label_content);
  gtk_widget_show_all(DialogConfirmDel);

  //display dialog and get responce form user
  int res = gtk_dialog_run(GTK_DIALOG(DialogConfirmDel));

  if(res == GTK_RESPONSE_OK){
    if(delete_user_system()){
      delete_user_app();
      current_user = current_user->next;
      display_current_user();
    }
  }

  gtk_widget_destroy(DialogConfirmDel);
}

void add_user(){
  if(add_user_system()){
    add_user_app();
    current_user = root_user->previous;
    display_current_user();
  }
}

void clear_input_labels(){
  gtk_entry_set_text (GTK_ENTRY(EntryUsername ), "");
  gtk_entry_set_text (GTK_ENTRY(EntryUserID   ), "");
  gtk_entry_set_text (GTK_ENTRY(EntryGroups    ), "");
  gtk_entry_set_text (GTK_ENTRY(EntryUserInfo ), "");
  gtk_entry_set_text (GTK_ENTRY(EntryDirectory), "");
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(EntryPassword), FALSE);
}


//event triggers
void on_ButtonPrv_clicked (GtkButton *b){
  current_user = current_user->previous;
  display_current_user();
  clear_output_labels();
}

void on_ButtonNxt_clicked (GtkButton *b){
  current_user = current_user->next;
  display_current_user();
  clear_output_labels();
}

void on_ButtonDlt_clicked (GtkButton *b){
  clear_output_labels();
  delete_user();
}

void on_ButtonClr_clicked (GtkButton *b){
  clear_input_labels();
  clear_output_labels();
}

void on_ButtonRfr_clicked (GtkButton *b){
  get_users_from_passwd(&current_user);
  display_current_user();
  clear_output_labels();
}

void on_stack_page_changed (GtkStackSwitcher *s){
  clear_output_labels();
}

void on_EventBoxWindow_button_press_event(GtkWidget *e){
  clear_output_labels();
}

void on_ButtonAdd_clicked (GtkButton *b){
  clear_output_labels();
  add_user();
}

bool on_Window_key_press_event(GtkWidget *widget, GdkEventKey *event){
  clear_output_labels();

  if(strcmp( gtk_stack_get_visible_child_name(GTK_STACK(Stack)), "Preview") == 0){
    //visible page is Preview
    switch (event->keyval)
    {
      case GDK_KEY_Left:
        current_user = current_user->previous;
        display_current_user();
        return true;
      case GDK_KEY_Right:
        current_user = current_user->next;
        display_current_user();
        return true;
      case GDK_KEY_Delete:
        delete_user();
        return true;
      case GDK_KEY_F5:
        get_users_from_passwd(&current_user);
        display_current_user();
        return true;
    }
  }
  else if(strcmp( gtk_stack_get_visible_child_name(GTK_STACK(Stack)), "Add") == 0){
    //visible page is Add
    switch (event->keyval)
    {
      case GDK_KEY_Delete:
        clear_input_labels();
        return true;
      case GDK_KEY_EnterRight:
        add_user();
        return true;
      case GDK_KEY_EnterLeft:
        add_user();
        return true;
    }

  }
  return false;
}


int main (int argc, char **argv) {

  current_user = NULL;

  get_users_from_passwd(&current_user);
  root_user = current_user;

  gtk_init(&argc, &argv);

  builder = gtk_builder_new_from_file("interface.glade");
  Window = GTK_WIDGET(gtk_builder_get_object(builder, "Window"));
  EventBoxWindow = GTK_WIDGET(gtk_builder_get_object(builder, "EventBoxWindow"));
  Stack = GTK_WIDGET(gtk_builder_get_object(builder, "Stack"));

  g_signal_connect(Window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_builder_connect_signals(builder, NULL);

  //buttons
  ButtonPrv = GTK_WIDGET(gtk_builder_get_object(builder, "ButtonPrv"));
  ButtonNxt = GTK_WIDGET(gtk_builder_get_object(builder, "ButtonNxt"));
  ButtonAdd = GTK_WIDGET(gtk_builder_get_object(builder, "ButtonAdd"));
  ButtonDlt = GTK_WIDGET(gtk_builder_get_object(builder, "ButtonDlt"));
  ButtonClr = GTK_WIDGET(gtk_builder_get_object(builder, "ButtonClr"));
  ButtonRfr = GTK_WIDGET(gtk_builder_get_object(builder, "ButtonRfr"));

  //labels
  LabelUsername           = GTK_WIDGET(gtk_builder_get_object(builder, "LabelUsername"           ));
  LabelPassword           = GTK_WIDGET(gtk_builder_get_object(builder, "LabelPassword"           ));
  LabelUserID             = GTK_WIDGET(gtk_builder_get_object(builder, "LabelUserID"             ));
  LabelGroups             = GTK_WIDGET(gtk_builder_get_object(builder, "LabelGroups"             ));
  LabelUserInfo           = GTK_WIDGET(gtk_builder_get_object(builder, "LabelUserInfo"           ));
  LabelDirectory          = GTK_WIDGET(gtk_builder_get_object(builder, "LabelDirectory"          ));
  LabelDeleteOutcome      = GTK_WIDGET(gtk_builder_get_object(builder, "LabelDeleteOutcome"      ));
  LabelAddOutcome         = GTK_WIDGET(gtk_builder_get_object(builder, "LabelAddOutcome"         ));
  LabelLastPasswordChange = GTK_WIDGET(gtk_builder_get_object(builder, "LabelLastPasswordChange" ));
  LabelPasswordExpires    = GTK_WIDGET(gtk_builder_get_object(builder, "LabelPasswordExpires"    ));
  LabelPasswordInactive   = GTK_WIDGET(gtk_builder_get_object(builder, "LabelPasswordInactive"   ));
  LabelAccountExpires     = GTK_WIDGET(gtk_builder_get_object(builder, "LabelAccountExpires"     ));
  

  //entries
  EntryUsername  = GTK_WIDGET(gtk_builder_get_object(builder, "EntryUsername"  ));
  EntryUsername  = GTK_WIDGET(gtk_builder_get_object(builder, "EntryUsername"  ));
  EntryUserID    = GTK_WIDGET(gtk_builder_get_object(builder, "EntryUserID"    ));
  EntryGroups    = GTK_WIDGET(gtk_builder_get_object(builder, "EntryGroups"     ));
  EntryUserInfo  = GTK_WIDGET(gtk_builder_get_object(builder, "EntryUserInfo"  ));
  EntryDirectory = GTK_WIDGET(gtk_builder_get_object(builder, "EntryDirectory" ));
  EntryPassword  = GTK_WIDGET(gtk_builder_get_object(builder, "EntryPassword"  ));

  gtk_widget_show(Window);
  display_current_user();
  gtk_main();

  return EXIT_SUCCESS;
}
